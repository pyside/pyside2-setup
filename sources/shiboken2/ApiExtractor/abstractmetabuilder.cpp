/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "abstractmetabuilder_p.h"
#include "messages.h"
#include "reporthandler.h"
#include "typedatabase.h"

#include <clangparser/clangbuilder.h>
#include <clangparser/clangutils.h>
#include <clangparser/compilersupport.h>

#include "parser/codemodel.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextCodec>
#include <QTextStream>
#include <QVariant>
#include <QTime>
#include <QQueue>
#include <QDir>

#include <cstdio>
#include <algorithm>
#include "graph.h"
#include <QTemporaryFile>

static inline QString colonColon() { return QStringLiteral("::"); }

static QString stripTemplateArgs(const QString &name)
{
    int pos = name.indexOf(QLatin1Char('<'));
    return pos < 0 ? name : name.left(pos);
}

static QStringList parseTemplateType(const QString& name) {
    int n = name.indexOf(QLatin1Char('<'));
    if (n <= 0) {
        // If name starts with '<' or contains an unmatched (i.e. any) '>', we
        // reject it
        if (n == 0 || name.count(QLatin1Char('>')))
            return QStringList();
        // Doesn't look like a template instantiation; just return the name
        return QStringList() << name;
    }

    // Split the type name into the template name and template arguments; the
    // part before the opening '<' is the template name
    //
    // Example:
    //   "foo<A, bar<B, C>, D>" -> ( "foo", "A", "bar<B, C>", "D" )
    QStringList result;
    result << name.left(n).trimmed();

    // Extract template arguments
    int i, depth = 1;
    const int l = name.length();
    for (i = n + 1; i < l; ++i) {
        // Consume balanced '<'/'>' within a single argument so that we won't
        // split on ',' as part of a single argument which is itself a
        // multi-argument template type
        if (name[i] == QLatin1Char('<')) {
            ++depth;
        } else if (name[i] == QLatin1Char('>')) {
            if (--depth == 0)
                break;
        } else if (name[i] == QLatin1Char(',') && depth == 1) {
            // Encountered ',' in template argument list that is not within
            // another template name; add current argument to result and start
            // working on the next argument
            result << name.mid(n + 1, i - n - 1).trimmed();
            n = i;
        }
    }
    if (i >= l) // arg list not closed
        return QStringList();
    if (i + 1 < l) // arg list closed before end of name
        return QStringList();

    // Add final argument and return result
    result << name.mid(n + 1, i - n - 1).trimmed();
    return result;
}

AbstractMetaBuilderPrivate::AbstractMetaBuilderPrivate() : m_currentClass(0),
    m_logDirectory(QLatin1String(".") + QDir::separator())
{
}

AbstractMetaBuilderPrivate::~AbstractMetaBuilderPrivate()
{
    qDeleteAll(m_globalEnums);
    qDeleteAll(m_globalFunctions);
    qDeleteAll(m_templates);
    qDeleteAll(m_smartPointers);
    qDeleteAll(m_metaClasses);
}

AbstractMetaBuilder::AbstractMetaBuilder() : d(new AbstractMetaBuilderPrivate)
{
    d->q = this;
}

AbstractMetaBuilder::~AbstractMetaBuilder()
{
    delete d;
}

AbstractMetaClassList AbstractMetaBuilder::classes() const
{
    return d->m_metaClasses;
}

AbstractMetaClassList AbstractMetaBuilder::templates() const
{
    return d->m_templates;
}

AbstractMetaClassList AbstractMetaBuilder::smartPointers() const
{
    return d->m_smartPointers;
}

AbstractMetaFunctionList AbstractMetaBuilder::globalFunctions() const
{
    return d->m_globalFunctions;
}

AbstractMetaEnumList AbstractMetaBuilder::globalEnums() const
{
    return d->m_globalEnums;
}

AbstractMetaEnum *AbstractMetaBuilder::findEnum(const TypeEntry *typeEntry) const
{
    if (typeEntry && typeEntry->isFlags())
        typeEntry = static_cast<const FlagsTypeEntry*>(typeEntry)->originator();
    return d->m_enums.value(typeEntry);
}

void AbstractMetaBuilderPrivate::checkFunctionModifications()
{
    const auto &entries = TypeDatabase::instance()->entries();

    for (auto it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
        const TypeEntry *entry = it.value();
        if (!entry)
            continue;
        if (!entry->isComplex() || entry->codeGeneration() == TypeEntry::GenerateNothing)
            continue;

        const ComplexTypeEntry* centry = static_cast<const ComplexTypeEntry*>(entry);

        if (!(centry->codeGeneration() & TypeEntry::GenerateTargetLang))
            continue;

        FunctionModificationList modifications = centry->functionModifications();

        for (const FunctionModification &modification : qAsConst(modifications)) {
            QString signature = modification.signature();

            QString name = signature.trimmed();
            name.truncate(name.indexOf(QLatin1Char('(')));

            AbstractMetaClass *clazz = AbstractMetaClass::findClass(m_metaClasses, centry->qualifiedCppName());
            if (!clazz)
                continue;

            const AbstractMetaFunctionList functions = clazz->functions();
            bool found = false;
            QStringList possibleSignatures;
            for (AbstractMetaFunction *function : functions) {
                if (function->implementingClass() == clazz
                    && modification.matches(function->minimalSignature())) {
                    found = true;
                    break;
                }

                if (function->originalName() == name) {
                    possibleSignatures.append(function->minimalSignature() + QLatin1String(" in ")
                                              + function->implementingClass()->name());
                }
            }

            if (!found) {
                qCWarning(lcShiboken).noquote().nospace()
                    << msgNoFunctionForModification(signature,
                                                    modification.originalSignature(),
                                                    clazz->qualifiedCppName(),
                                                    possibleSignatures, functions);
            }
        }
    }
}

AbstractMetaClass *AbstractMetaBuilderPrivate::argumentToClass(const ArgumentModelItem &argument)
{
    AbstractMetaClass* returned = 0;
    AbstractMetaType *type = translateType(argument->type());
    if (type && type->typeEntry() && type->typeEntry()->isComplex()) {
        const TypeEntry *entry = type->typeEntry();
        returned = AbstractMetaClass::findClass(m_metaClasses, entry->name());
    }
    delete type;
    return returned;
}

/**
 * Checks the argument of a hash function and flags the type if it is a complex type
 */
void AbstractMetaBuilderPrivate::registerHashFunction(const FunctionModelItem &function_item)
{
    ArgumentList arguments = function_item->arguments();
    if (arguments.size() == 1) {
        if (AbstractMetaClass *cls = argumentToClass(arguments.at(0)))
            cls->setHasHashFunction(true);
    }
}

/**
 * Check if a class has a debug stream operator that can be used as toString
 */

void AbstractMetaBuilderPrivate::registerToStringCapability(const FunctionModelItem &function_item)
{
    ArgumentList arguments = function_item->arguments();
    if (arguments.size() == 2) {
        if (arguments.at(0)->type().toString() == QLatin1String("QDebug")) {
            const ArgumentModelItem &arg = arguments.at(1);
            if (AbstractMetaClass *cls = argumentToClass(arg)) {
                if (arg->type().indirections() < 2)
                    cls->setToStringCapability(true);
            }
        }
    }
}

void AbstractMetaBuilderPrivate::traverseOperatorFunction(const FunctionModelItem &item)
{
    if (item->accessPolicy() != CodeModel::Public)
        return;

    ArgumentList arguments = item->arguments();
    AbstractMetaClass* baseoperandClass;
    bool firstArgumentIsSelf = true;
    bool unaryOperator = false;

    baseoperandClass = argumentToClass(arguments.at(0));

    if (arguments.size() == 1) {
        unaryOperator = true;
    } else if (!baseoperandClass
               || !(baseoperandClass->typeEntry()->codeGeneration() & TypeEntry::GenerateTargetLang)) {
        baseoperandClass = argumentToClass(arguments.at(1));
        firstArgumentIsSelf = false;
    } else {
        AbstractMetaType *type = translateType(item->type());
        const TypeEntry *retType = type ? type->typeEntry() : nullptr;
        AbstractMetaClass* otherArgClass = argumentToClass(arguments.at(1));
        if (otherArgClass && retType
            && (retType->isValue() || retType->isObject())
            && retType != baseoperandClass->typeEntry()
            && retType == otherArgClass->typeEntry()) {
            baseoperandClass = AbstractMetaClass::findClass(m_metaClasses, retType);
            firstArgumentIsSelf = false;
        }
        delete type;
    }

    if (baseoperandClass) {
        AbstractMetaClass* oldCurrentClass = m_currentClass;
        m_currentClass = baseoperandClass;
        AbstractMetaFunction *metaFunction = traverseFunction(item);
        if (metaFunction) {
            // Strip away first argument, since that is the containing object
            AbstractMetaArgumentList arguments = metaFunction->arguments();
            if (firstArgumentIsSelf || unaryOperator) {
                AbstractMetaArgument* first = arguments.takeFirst();
                if (!unaryOperator && first->type()->indirections())
                    metaFunction->setPointerOperator(true);
                delete first;
                metaFunction->setArguments(arguments);
            } else {
                // If the operator method is not unary and the first operator is
                // not of the same type of its owning class we suppose that it
                // must be an reverse operator (e.g. CLASS::operator(TYPE, CLASS)).
                // All operator overloads that operate over a class are already
                // being added as member functions of that class by the API Extractor.
                AbstractMetaArgument* last = arguments.takeLast();
                if (last->type()->indirections())
                    metaFunction->setPointerOperator(true);
                delete last;

                metaFunction->setArguments(arguments);
                metaFunction->setReverseOperator(true);
            }
            metaFunction->setFunctionType(AbstractMetaFunction::NormalFunction);
            metaFunction->setVisibility(AbstractMetaFunction::Public);
            metaFunction->setOriginalAttributes(metaFunction->attributes());
            setupFunctionDefaults(metaFunction, baseoperandClass);
            baseoperandClass->addFunction(metaFunction);
            Q_ASSERT(!metaFunction->wasPrivate());
        } else {
            delete metaFunction;
        }

        m_currentClass = oldCurrentClass;
    }
}

void AbstractMetaBuilderPrivate::traverseStreamOperator(const FunctionModelItem &item)
{
    ArgumentList arguments = item->arguments();
    if (arguments.size() == 2 && item->accessPolicy() == CodeModel::Public) {
        AbstractMetaClass* streamClass = argumentToClass(arguments.at(0));
        AbstractMetaClass* streamedClass = argumentToClass(arguments.at(1));

        if (streamClass && streamedClass && (streamClass->isStream())) {
            AbstractMetaClass *oldCurrentClass = m_currentClass;
            m_currentClass = streamedClass;
            AbstractMetaFunction *streamFunction = traverseFunction(item);

            if (streamFunction) {
                streamFunction->setFunctionType(AbstractMetaFunction::GlobalScopeFunction);
                // Strip first argument, since that is the containing object
                AbstractMetaArgumentList arguments = streamFunction->arguments();
                if (!streamClass->typeEntry()->generateCode())
                    delete arguments.takeLast();
                else
                    delete arguments.takeFirst();

                streamFunction->setArguments(arguments);

                *streamFunction += AbstractMetaAttributes::FinalInTargetLang;
                *streamFunction += AbstractMetaAttributes::Public;
                streamFunction->setOriginalAttributes(streamFunction->attributes());

//              streamFunction->setType(0);

                AbstractMetaClass *funcClass;

                if (!streamClass->typeEntry()->generateCode()) {
                    AbstractMetaArgumentList reverseArgs = reverseList(streamFunction->arguments());
                    streamFunction->setArguments(reverseArgs);
                    streamFunction->setReverseOperator(true);
                    funcClass = streamedClass;
                } else {
                    funcClass = streamClass;
                }

                setupFunctionDefaults(streamFunction, funcClass);
                funcClass->addFunction(streamFunction);
                if (funcClass == streamClass)
                    funcClass->typeEntry()->addExtraInclude(streamedClass->typeEntry()->include());
                else
                    funcClass->typeEntry()->addExtraInclude(streamClass->typeEntry()->include());

                m_currentClass = oldCurrentClass;
            } else {
                delete streamFunction;
            }

        }
    }
}

void AbstractMetaBuilderPrivate::fixQObjectForScope(const FileModelItem &dom,
                                                    const TypeDatabase *types,
                                                    const NamespaceModelItem &scope)
{
    const ClassList &scopeClasses = scope->classes();
    for (const ClassModelItem &item : scopeClasses) {
        QString qualifiedName = item->qualifiedName().join(colonColon());
        TypeEntry* entry = types->findType(qualifiedName);
        if (entry) {
            if (isQObject(dom, qualifiedName) && entry->isComplex())
                static_cast<ComplexTypeEntry *>(entry)->setQObject(true);
        }
    }

    const NamespaceList &namespaces = scope->namespaces();
    for (const NamespaceModelItem &n : namespaces) {
        if (scope != n)
            fixQObjectForScope(dom, types, n);
    }
}

void AbstractMetaBuilderPrivate::sortLists()
{
    for (AbstractMetaClass *cls : qAsConst(m_metaClasses))
        cls->sortFunctions();
}

FileModelItem AbstractMetaBuilderPrivate::buildDom(QByteArrayList arguments,
                                                   LanguageLevel level,
                                                   unsigned clangFlags)
{
    clang::Builder builder;
    if (level == LanguageLevel::Default)
        level = clang::emulatedCompilerLanguageLevel();
    arguments.prepend(QByteArrayLiteral("-std=")
                      + clang::languageLevelOption(level));
    FileModelItem result = clang::parse(arguments, clangFlags, builder)
        ? builder.dom() : FileModelItem();
    const clang::BaseVisitor::Diagnostics &diagnostics = builder.diagnostics();
    if (const int diagnosticsCount = diagnostics.size()) {
        QDebug d = qWarning();
        d.nospace();
        d.noquote();
        d << "Clang: " << diagnosticsCount << " diagnostic messages:\n";
        for (int i = 0; i < diagnosticsCount; ++i)
            d << "  " << diagnostics.at(i) << '\n';
    }
    return result;
}

void AbstractMetaBuilderPrivate::traverseDom(const FileModelItem &dom)
{
    const TypeDatabase *types = TypeDatabase::instance();

    pushScope(dom);

    // fix up QObject's in the type system..
    fixQObjectForScope(dom, types, dom);

    // Start the generation...
    const ClassList &typeValues = dom->classes();
    ReportHandler::setProgressReference(typeValues);
    for (const ClassModelItem &item : typeValues) {
        ReportHandler::progress(QStringLiteral("Generating class model (%1)...")
                                .arg(typeValues.size()));
        if (AbstractMetaClass *cls = traverseClass(dom, item))
            addAbstractMetaClass(cls);
    }

    // We need to know all global enums
    const EnumList &enums = dom->enums();
    ReportHandler::setProgressReference(enums);
    for (const EnumModelItem &item : enums) {
        ReportHandler::progress(QStringLiteral("Generating enum model (%1)...")
                                .arg(enums.size()));
        AbstractMetaEnum *metaEnum = traverseEnum(item, 0, QSet<QString>());
        if (metaEnum) {
            if (metaEnum->typeEntry()->generateCode())
                m_globalEnums << metaEnum;
        }
    }

    const auto &namespaceTypeValues = dom->namespaces();
    ReportHandler::setProgressReference(namespaceTypeValues);
    for (const NamespaceModelItem &item : namespaceTypeValues) {
        ReportHandler::progress(QStringLiteral("Generating namespace model (%1)...")
                                .arg(namespaceTypeValues.size()));
        AbstractMetaClass *metaClass = traverseNamespace(dom, item);
        if (metaClass)
            m_metaClasses << metaClass;
    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    const TypeDefList typeDefs = dom->typeDefs();
    ReportHandler::setProgressReference(typeDefs);
    for (const TypeDefModelItem &typeDef : typeDefs) {
        ReportHandler::progress(QStringLiteral("Resolving typedefs (%1)...")
                                .arg(typeDefs.size()));
        if (AbstractMetaClass *cls = traverseTypeDef(dom, typeDef))
            addAbstractMetaClass(cls);
    }

    traverseTypesystemTypedefs();

    for (const ClassModelItem &item : typeValues)
        traverseClassMembers(item);

    for (const NamespaceModelItem &item : namespaceTypeValues)
        traverseNamespaceMembers(item);

    // Global functions
    const FunctionList &functions = dom->functions();
    for (const FunctionModelItem &func : functions) {
        if (func->accessPolicy() != CodeModel::Public || func->name().startsWith(QLatin1String("operator")))
            continue;

        FunctionTypeEntry* funcEntry = types->findFunctionType(func->name());
        if (!funcEntry || !funcEntry->generateCode())
            continue;

        AbstractMetaFunction* metaFunc = traverseFunction(func);
        if (!metaFunc)
            continue;

        if (!funcEntry->hasSignature(metaFunc->minimalSignature())) {
            delete metaFunc;
            continue;
        }

        applyFunctionModifications(metaFunc);

        setInclude(funcEntry, func->fileName());
        if (metaFunc->typeEntry())
            delete metaFunc->typeEntry();

        metaFunc->setTypeEntry(funcEntry);
        m_globalFunctions << metaFunc;
    }

    ReportHandler::setProgressReference(m_metaClasses);
    for (AbstractMetaClass *cls : qAsConst(m_metaClasses)) {
        ReportHandler::progress(QLatin1String("Fixing class inheritance..."));
        if (!cls->isInterface() && !cls->isNamespace())
            setupInheritance(cls);
    }

    ReportHandler::setProgressReference(m_metaClasses);
    for (AbstractMetaClass *cls : qAsConst(m_metaClasses)) {
        ReportHandler::progress(QLatin1String("Detecting inconsistencies in class model..."));
        cls->fixFunctions();

        if (!cls->typeEntry()) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("class '%1' does not have an entry in the type system")
                                  .arg(cls->name());
        } else {
            const bool couldAddDefaultCtors = cls->isConstructible()
                && !cls->isInterface() && !cls->isNamespace()
                && (cls->attributes() & AbstractMetaAttributes::HasRejectedConstructor) == 0;
            if (couldAddDefaultCtors) {
                if (!cls->hasConstructors())
                    cls->addDefaultConstructor();
                if (cls->typeEntry()->isValue() && !cls->isAbstract() && !cls->hasCopyConstructor())
                    cls->addDefaultCopyConstructor(ancestorHasPrivateCopyConstructor(cls));
            }
        }

        if (cls->isAbstract() && !cls->isInterface())
            cls->typeEntry()->setLookupName(cls->typeEntry()->targetLangName() + QLatin1String("$ConcreteWrapper"));
    }
    const auto &allEntries = types->entries();
    ReportHandler::progress(QStringLiteral("Detecting inconsistencies in typesystem (%1)...")
                            .arg(allEntries.size()));
    for (auto it = allEntries.cbegin(), end = allEntries.cend(); it != end; ++it) {
        TypeEntry *entry = it.value();
        if (!entry->isPrimitive()) {
            if ((entry->isValue() || entry->isObject())
                && !entry->isString()
                && !entry->isChar()
                && !entry->isContainer()
                && !entry->isCustom()
                && !entry->isVariant()
                && (entry->generateCode() & TypeEntry::GenerateTargetLang)
                && !AbstractMetaClass::findClass(m_metaClasses, entry->qualifiedCppName())) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("type '%1' is specified in typesystem, but not defined. This could potentially lead to compilation errors.")
                                      .arg(entry->qualifiedCppName());
            } else if (entry->generateCode() && entry->type() == TypeEntry::FunctionType) {
                const FunctionTypeEntry* fte = static_cast<const FunctionTypeEntry*>(entry);
                const QStringList &signatures = fte->signatures();
                for (const QString &signature : signatures) {
                    bool ok = false;
                    for (AbstractMetaFunction* func : qAsConst(m_globalFunctions)) {
                        if (signature == func->minimalSignature()) {
                            ok = true;
                            break;
                        }
                    }
                    if (!ok) {
                        qCWarning(lcShiboken).noquote().nospace()
                            << QStringLiteral("Global function '%1' is specified in typesystem, but not defined. This could potentially lead to compilation errors.")
                                              .arg(signature);
                    }
                }
            } else if (entry->isEnum() && (entry->generateCode() & TypeEntry::GenerateTargetLang)) {
                const QString name = static_cast<const EnumTypeEntry *>(entry)->targetLangQualifier();
                AbstractMetaClass *cls = AbstractMetaClass::findClass(m_metaClasses, name);

                const bool enumFound = cls
                    ? cls->findEnum(entry->targetLangName()) != nullptr
                    : m_enums.contains(entry);

                if (!enumFound) {
                    entry->setCodeGeneration(TypeEntry::GenerateNothing);
                    qCWarning(lcShiboken).noquote().nospace()
                        << QStringLiteral("enum '%1' is specified in typesystem, but not declared")
                                          .arg(entry->qualifiedCppName());
                }

            }
        }
    }

    {
        const FunctionList &hashFunctions = dom->findFunctions(QLatin1String("qHash"));
        for (const FunctionModelItem &item : hashFunctions)
            registerHashFunction(item);
    }

    {
        const FunctionList &streamOps = dom->findFunctions(QLatin1String("operator<<"));
        for (const FunctionModelItem &item : streamOps)
            registerToStringCapability(item);
    }

    {
        FunctionList binaryOperators = dom->findFunctions(QStringLiteral("operator=="));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator!=")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator<=")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator>=")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator<")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator+")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator/")));
        // Filter binary operators, skipping for example
        // class Iterator { ... Value *operator*() ... };
        const FunctionList potentiallyBinaryOperators =
            dom->findFunctions(QStringLiteral("operator*"))
            + dom->findFunctions(QStringLiteral("operator&"));
        for (const FunctionModelItem &item : potentiallyBinaryOperators) {
            if (!item->arguments().isEmpty())
                binaryOperators.append(item);
        }
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator-")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator&")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator|")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator^")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator~")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator>")));

        for (const FunctionModelItem &item : qAsConst(binaryOperators))
            traverseOperatorFunction(item);
    }

    {
        const FunctionList streamOperators = dom->findFunctions(QLatin1String("operator<<"))
            + dom->findFunctions(QLatin1String("operator>>"));
        for (const FunctionModelItem &item : streamOperators)
            traverseStreamOperator(item);
    }

    checkFunctionModifications();

    // sort all classes topologically
    m_metaClasses = classesTopologicalSorted();

    for (AbstractMetaClass* cls : qAsConst(m_metaClasses)) {
//         setupEquals(cls);
//         setupComparable(cls);
        setupClonable(cls);
        setupExternalConversion(cls);

        // sort all inner classes topologically
        if (!cls->typeEntry()->codeGeneration() || cls->innerClasses().size() < 2)
            continue;

        cls->setInnerClasses(classesTopologicalSorted(cls));
    }

    dumpLog();

    sortLists();

    m_currentClass = 0;

    // Functions added to the module on the type system.
    const AddedFunctionList &globalUserFunctions = types->globalUserFunctions();
    for (const AddedFunction &addedFunc : globalUserFunctions) {
        AbstractMetaFunction* metaFunc = traverseFunction(addedFunc);
        if (Q_UNLIKELY(!metaFunc)) {
            qFatal("Unable to traverse added global function \"%s\".",
                   qPrintable(addedFunc.name()));
        }
        metaFunc->setFunctionType(AbstractMetaFunction::NormalFunction);
        m_globalFunctions << metaFunc;
    }

    std::puts("");
}

static bool metaEnumLessThan(const AbstractMetaEnum *e1, const AbstractMetaEnum *e2)
{ return e1->fullName() < e2->fullName(); }

static bool metaClassLessThan(const AbstractMetaClass *c1, const AbstractMetaClass *c2)
{ return c1->fullName() < c2->fullName(); }

static bool metaFunctionLessThan(const AbstractMetaFunction *f1, const AbstractMetaFunction *f2)
{ return f1->name() < f2->name(); }

bool AbstractMetaBuilder::build(const QByteArrayList &arguments,
                                LanguageLevel level,
                                unsigned clangFlags)
{
    const FileModelItem dom = d->buildDom(arguments, level, clangFlags);
    if (dom.isNull())
        return false;
    if (ReportHandler::isDebug(ReportHandler::MediumDebug))
        qCDebug(lcShiboken) << dom.data();
    d->traverseDom(dom);

    // Ensure that indexes are in alphabetical order, roughly
    std::sort(d->m_globalEnums.begin(), d->m_globalEnums.end(), metaEnumLessThan);
    std::sort(d->m_metaClasses.begin(), d->m_metaClasses.end(), metaClassLessThan);
    std::sort(d->m_templates.begin(), d->m_templates.end(), metaClassLessThan);
    std::sort(d->m_smartPointers.begin(), d->m_smartPointers.end(), metaClassLessThan);
    std::sort(d->m_globalFunctions.begin(), d->m_globalFunctions.end(), metaFunctionLessThan);

    return true;
}

void AbstractMetaBuilder::setLogDirectory(const QString& logDir)
{
    d->m_logDirectory = logDir;
    if (!d->m_logDirectory.endsWith(QDir::separator()))
       d->m_logDirectory.append(QDir::separator());
}

void AbstractMetaBuilderPrivate::addAbstractMetaClass(AbstractMetaClass *cls)
{
    cls->setOriginalAttributes(cls->attributes());
    if (cls->typeEntry()->isContainer()) {
        m_templates << cls;
    } else if (cls->typeEntry()->isSmartPointer()) {
        m_smartPointers << cls;
    } else {
        m_metaClasses << cls;
        if (cls->typeEntry()->designatedInterface()) {
            AbstractMetaClass* interface = cls->extractInterface();
            m_metaClasses << interface;
            if (ReportHandler::isDebug(ReportHandler::SparseDebug))
                qCDebug(lcShiboken) << QStringLiteral(" -> interface '%1'").arg(interface->name());
        }
    }
}

AbstractMetaClass *AbstractMetaBuilderPrivate::traverseNamespace(const FileModelItem &dom,
                                                                 const NamespaceModelItem &namespaceItem)
{
    QString namespaceName =
        (!m_namespacePrefix.isEmpty() ? m_namespacePrefix + colonColon() : QString())
        + namespaceItem->name();
    NamespaceTypeEntry *type = TypeDatabase::instance()->findNamespaceType(namespaceName);

    if (TypeDatabase::instance()->isClassRejected(namespaceName)) {
        m_rejectedClasses.insert(namespaceName, AbstractMetaBuilder::GenerationDisabled);
        return 0;
    }

    if (!type) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("namespace '%1' does not have a type entry").arg(namespaceName);
        return 0;
    }

    AbstractMetaClass* metaClass = new AbstractMetaClass;
    metaClass->setTypeEntry(type);

    *metaClass += AbstractMetaAttributes::Public;

    m_currentClass = metaClass;

    if (ReportHandler::isDebug(ReportHandler::SparseDebug)) {
        qCDebug(lcShiboken)
            << QStringLiteral("namespace '%1.%2'").arg(metaClass->package(), namespaceItem->name());
    }

    traverseEnums(namespaceItem, metaClass, namespaceItem->enumsDeclarations());

    pushScope(namespaceItem);
    m_namespacePrefix = currentScope()->qualifiedName().join(colonColon());

    const ClassList &classes = namespaceItem->classes();
    for (const ClassModelItem &cls : classes) {
        AbstractMetaClass* mjc = traverseClass(dom, cls);
        if (mjc) {
            metaClass->addInnerClass(mjc);
            mjc->setEnclosingClass(metaClass);
            addAbstractMetaClass(mjc);
        }
    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    const TypeDefList typeDefs = namespaceItem->typeDefs();
    for (const TypeDefModelItem &typeDef : typeDefs) {
        AbstractMetaClass *cls = traverseTypeDef(dom, typeDef);
        if (cls) {
            metaClass->addInnerClass(cls);
            cls->setEnclosingClass(metaClass);
            addAbstractMetaClass(cls);
        }
    }

    // Traverse namespaces recursively
    for (const NamespaceModelItem &ni : namespaceItem->namespaces()) {
        AbstractMetaClass* mjc = traverseNamespace(dom, ni);
        if (mjc) {
            metaClass->addInnerClass(mjc);
            mjc->setEnclosingClass(metaClass);
            addAbstractMetaClass(mjc);
        }
    }

    m_currentClass = 0;

    popScope();
    m_namespacePrefix = currentScope()->qualifiedName().join(colonColon());

    if (!type->include().isValid())
        setInclude(type, namespaceItem->fileName());

    return metaClass;
}

AbstractMetaEnum *AbstractMetaBuilderPrivate::traverseEnum(const EnumModelItem &enumItem,
                                                           AbstractMetaClass *enclosing,
                                                           const QSet<QString> &enumsDeclarations)
{
    QString qualifiedName = enumItem->qualifiedName().join(colonColon());

    TypeEntry* typeEntry = 0;
    if (enumItem->accessPolicy() == CodeModel::Private) {
        QStringList names = enumItem->qualifiedName();
        const QString &enumName = names.constLast();
        QString nspace;
        if (names.size() > 1)
            nspace = QStringList(names.mid(0, names.size() - 1)).join(colonColon());
        typeEntry = new EnumTypeEntry(nspace, enumName, QVersionNumber(0, 0));
        TypeDatabase::instance()->addType(typeEntry);
    } else if (enumItem->enumKind() != AnonymousEnum) {
        typeEntry = TypeDatabase::instance()->findType(qualifiedName);
    } else {
        QStringList tmpQualifiedName = enumItem->qualifiedName();
        const EnumeratorList &enums = enumItem->enumerators();
        for (const EnumeratorModelItem& enumValue : enums) {
            tmpQualifiedName.removeLast();
            tmpQualifiedName << enumValue->name();
            qualifiedName = tmpQualifiedName.join(colonColon());
            typeEntry = TypeDatabase::instance()->findType(qualifiedName);
            if (typeEntry)
                break;
        }
    }

    QString enumName = enumItem->name();

    QString className;
    if (m_currentClass)
        className = m_currentClass->typeEntry()->qualifiedCppName();

    QString rejectReason;
    if (TypeDatabase::instance()->isEnumRejected(className, enumName, &rejectReason)) {
        if (typeEntry)
            typeEntry->setCodeGeneration(TypeEntry::GenerateNothing);
        m_rejectedEnums.insert(qualifiedName + rejectReason, AbstractMetaBuilder::GenerationDisabled);
        return 0;
    }

    const bool rejectionWarning = !m_currentClass
        || (m_currentClass->typeEntry()->codeGeneration() & TypeEntry::GenerateTargetLang);

    if (!typeEntry) {
        if (rejectionWarning)
            qCWarning(lcShiboken, "%s", qPrintable(msgNoEnumTypeEntry(enumItem, className)));
        m_rejectedEnums.insert(qualifiedName, AbstractMetaBuilder::NotInTypeSystem);
        return nullptr;
    }

    if (!typeEntry->isEnum()) {
        if (rejectionWarning) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgNoEnumTypeConflict(enumItem, className, typeEntry)));
        }
        m_rejectedEnums.insert(qualifiedName, AbstractMetaBuilder::NotInTypeSystem);
        return nullptr;
    }

    AbstractMetaEnum *metaEnum = new AbstractMetaEnum;
    metaEnum->setEnumKind(enumItem->enumKind());
    metaEnum->setSigned(enumItem->isSigned());
    if (enumsDeclarations.contains(qualifiedName)
        || enumsDeclarations.contains(enumName)) {
        metaEnum->setHasQEnumsDeclaration(true);
    }

    EnumTypeEntry *enumTypeEntry = static_cast<EnumTypeEntry *>(typeEntry);
    metaEnum->setTypeEntry(enumTypeEntry);
    switch (enumItem->accessPolicy()) {
    case CodeModel::Public:
        *metaEnum += AbstractMetaAttributes::Public;
        break;
    case CodeModel::Protected:
        *metaEnum += AbstractMetaAttributes::Protected;
        break;
    case CodeModel::Private:
        *metaEnum += AbstractMetaAttributes::Private;
        typeEntry->setCodeGeneration(TypeEntry::GenerateNothing);
        break;
    default:
        break;
    }

    if (ReportHandler::isDebug(ReportHandler::MediumDebug))
        qCDebug(lcShiboken) << " - traversing enum " << metaEnum->fullName();

    const EnumeratorList &enums = enumItem->enumerators();
    for (const EnumeratorModelItem &value : enums) {

        AbstractMetaEnumValue *metaEnumValue = new AbstractMetaEnumValue;
        metaEnumValue->setName(value->name());
        // Deciding the enum value...

        metaEnumValue->setStringValue(value->stringValue());
        metaEnumValue->setValue(value->value());
        metaEnum->addEnumValue(metaEnumValue);

        if (ReportHandler::isDebug(ReportHandler::FullDebug)) {
            qCDebug(lcShiboken) << "   - " << metaEnumValue->name() << " = "
                << metaEnumValue->value() << " = " << metaEnumValue->value();
        }
    }

    m_enums.insert(typeEntry, metaEnum);

    if (!metaEnum->typeEntry()->include().isValid())
        setInclude(metaEnum->typeEntry(), enumItem->fileName());

    metaEnum->setOriginalAttributes(metaEnum->attributes());

    // Register all enum values on Type database
    QString prefix;
    if (enclosing) {
        prefix += enclosing->typeEntry()->qualifiedCppName();
        prefix += colonColon();
    }
    if (enumItem->enumKind() == EnumClass) {
        prefix += enumItem->name();
        prefix += colonColon();
    }
    const EnumeratorList &enumerators = enumItem->enumerators();
    for (const EnumeratorModelItem &e : enumerators) {
        QString name;
        if (enclosing) {
            name += enclosing->name();
            name += colonColon();
        }
        EnumValueTypeEntry *enumValue =
            new EnumValueTypeEntry(prefix + e->name(), e->stringValue(),
                                   enumTypeEntry, enumTypeEntry->version());
        TypeDatabase::instance()->addType(enumValue);
        if (e->value().isNullValue())
            enumTypeEntry->setNullValue(enumValue);
    }

    return metaEnum;
}

AbstractMetaClass* AbstractMetaBuilderPrivate::traverseTypeDef(const FileModelItem &dom,
                                                               const TypeDefModelItem &typeDef)
{
    TypeDatabase* types = TypeDatabase::instance();
    QString className = stripTemplateArgs(typeDef->name());

    QString fullClassName = className;
    // we have an inner class
    if (m_currentClass) {
        fullClassName = stripTemplateArgs(m_currentClass->typeEntry()->qualifiedCppName())
                          + colonColon() + fullClassName;
    }

    // If this is the alias for a primitive type
    // we store the aliased type on the alias
    // TypeEntry
    PrimitiveTypeEntry* ptype = types->findPrimitiveType(className);
    if (ptype) {
        QString typeDefName = typeDef->type().qualifiedName()[0];
        ptype->setReferencedTypeEntry(types->findPrimitiveType(typeDefName));
        return 0;
    }


    // If we haven't specified anything for the typedef, then we don't care
    ComplexTypeEntry* type = types->findComplexType(fullClassName);
    if (!type)
        return 0;

    if (type->isObject())
        static_cast<ObjectTypeEntry *>(type)->setQObject(isQObject(dom, stripTemplateArgs(typeDef->type().qualifiedName().join(colonColon()))));

    AbstractMetaClass *metaClass = new AbstractMetaClass;
    metaClass->setTypeDef(true);
    metaClass->setTypeEntry(type);
    metaClass->setBaseClassNames(QStringList(typeDef->type().toString()));
    *metaClass += AbstractMetaAttributes::Public;

    // Set the default include file name
    if (!type->include().isValid())
        setInclude(type, typeDef->fileName());

    fillAddedFunctions(metaClass);

    return metaClass;
}

// Add the typedef'ed classes
void AbstractMetaBuilderPrivate::traverseTypesystemTypedefs()
{
    const auto &entries = TypeDatabase::instance()->typedefEntries();
    for (auto it = entries.begin(), end = entries.end(); it != end; ++it) {
        TypedefEntry *te = it.value();
        AbstractMetaClass *metaClass = new AbstractMetaClass;
        metaClass->setTypeDef(true);
        metaClass->setTypeEntry(te->target());
        metaClass->setBaseClassNames(QStringList(te->sourceType()));
        *metaClass += AbstractMetaAttributes::Public;
        fillAddedFunctions(metaClass);
        addAbstractMetaClass(metaClass);
    }
}

AbstractMetaClass *AbstractMetaBuilderPrivate::traverseClass(const FileModelItem &dom,
                                                             const ClassModelItem &classItem)
{
    QString className = stripTemplateArgs(classItem->name());
    QString fullClassName = className;

    // we have inner an class
    if (m_currentClass) {
        fullClassName = stripTemplateArgs(m_currentClass->typeEntry()->qualifiedCppName())
                          + colonColon() + fullClassName;
    }

    ComplexTypeEntry* type = TypeDatabase::instance()->findComplexType(fullClassName);
    AbstractMetaBuilder::RejectReason reason = AbstractMetaBuilder::NoReason;

    if (TypeDatabase::instance()->isClassRejected(fullClassName)) {
        reason = AbstractMetaBuilder::GenerationDisabled;
    } else if (!type) {
        TypeEntry *te = TypeDatabase::instance()->findType(fullClassName);
        if (te && !te->isComplex())
            reason = AbstractMetaBuilder::RedefinedToNotClass;
        else
            reason = AbstractMetaBuilder::NotInTypeSystem;
    } else if (type->codeGeneration() == TypeEntry::GenerateNothing) {
        reason = AbstractMetaBuilder::GenerationDisabled;
    }
    if (reason != AbstractMetaBuilder::NoReason) {
        m_rejectedClasses.insert(fullClassName, reason);
        return 0;
    }

    if (type->isObject())
        ((ObjectTypeEntry*)type)->setQObject(isQObject(dom, fullClassName));

    AbstractMetaClass *metaClass = new AbstractMetaClass;
    metaClass->setTypeEntry(type);

    if (classItem->isFinal())
        *metaClass += AbstractMetaAttributes::FinalCppClass;

    QStringList baseClassNames;
    const QVector<_ClassModelItem::BaseClass> &baseClasses = classItem->baseClasses();
    for (const _ClassModelItem::BaseClass &baseClass : baseClasses) {
        if (baseClass.accessPolicy == CodeModel::Public)
            baseClassNames.append(baseClass.name);
    }

    metaClass->setBaseClassNames(baseClassNames);
    *metaClass += AbstractMetaAttributes::Public;
    if (type->stream())
        metaClass->setStream(true);

    AbstractMetaClass* oldCurrentClass = m_currentClass;
    m_currentClass = metaClass;

    if (ReportHandler::isDebug(ReportHandler::SparseDebug)) {
        const QString message = type->isContainer()
            ? QStringLiteral("container: '%1'").arg(fullClassName)
            : QStringLiteral("class: '%1'").arg(metaClass->fullName());
        qCDebug(lcShiboken) << message;
    }

    TemplateParameterList template_parameters = classItem->templateParameters();
    QVector<TypeEntry *> template_args;
    template_args.clear();
    for (int i = 0; i < template_parameters.size(); ++i) {
        const TemplateParameterModelItem &param = template_parameters.at(i);
        TemplateArgumentEntry *param_type = new TemplateArgumentEntry(param->name(), type->version());
        param_type->setOrdinal(i);
        template_args.append(param_type);
    }
    metaClass->setTemplateArguments(template_args);

    parseQ_Property(metaClass, classItem->propertyDeclarations());

    traverseEnums(classItem, metaClass, classItem->enumsDeclarations());

    // Inner classes
    {
        const ClassList &innerClasses = classItem->classes();
        for (const ClassModelItem &ci : innerClasses) {
            AbstractMetaClass *cl = traverseClass(dom, ci);
            if (cl) {
                cl->setEnclosingClass(metaClass);
                metaClass->addInnerClass(cl);
                m_metaClasses << cl;
            }
        }

    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    const TypeDefList typeDefs = classItem->typeDefs();
    for (const TypeDefModelItem &typeDef : typeDefs) {
        AbstractMetaClass *cls = traverseTypeDef(dom, typeDef);
        if (cls) {
            cls->setEnclosingClass(metaClass);
            addAbstractMetaClass(cls);
        }
    }


    m_currentClass = oldCurrentClass;

    // Set the default include file name
    if (!type->include().isValid())
        setInclude(type, classItem->fileName());

    return metaClass;
}

void AbstractMetaBuilderPrivate::traverseScopeMembers(ScopeModelItem item,
                                                      AbstractMetaClass *metaClass)
{
    // Classes/Namespace members
    traverseFields(item, metaClass);
    traverseFunctions(item, metaClass);

    // Inner classes
    const ClassList &innerClasses = item->classes();
    for (const ClassModelItem& ci : innerClasses)
        traverseClassMembers(ci);
}

AbstractMetaClass* AbstractMetaBuilderPrivate::currentTraversedClass(ScopeModelItem item)
{
    QString className = stripTemplateArgs(item->name());
    QString fullClassName = className;

    // This is an inner class
    if (m_currentClass)
        fullClassName = stripTemplateArgs(m_currentClass->typeEntry()->qualifiedCppName()) + colonColon() + fullClassName;

    AbstractMetaClass *metaClass = AbstractMetaClass::findClass(m_metaClasses, fullClassName);
    if (!metaClass)
        metaClass = AbstractMetaClass::findClass(m_templates, fullClassName);

    if (!metaClass)
        metaClass = AbstractMetaClass::findClass(m_smartPointers, fullClassName);
    return metaClass;
}

void AbstractMetaBuilderPrivate::traverseClassMembers(ClassModelItem item)
{
    AbstractMetaClass* metaClass = currentTraversedClass(item);
    if (!metaClass)
        return;

    AbstractMetaClass* oldCurrentClass = m_currentClass;
    m_currentClass = metaClass;

    // Class members
    traverseScopeMembers(item, metaClass);

    m_currentClass = oldCurrentClass;
}

void AbstractMetaBuilderPrivate::traverseNamespaceMembers(NamespaceModelItem item)
{
    AbstractMetaClass* metaClass = currentTraversedClass(item);
    if (!metaClass)
        return;

    AbstractMetaClass* oldCurrentClass = m_currentClass;
    m_currentClass = metaClass;

    // Namespace members
    traverseScopeMembers(item, metaClass);

    // Inner namespaces
    for (const NamespaceModelItem &ni : item->namespaces())
        traverseNamespaceMembers(ni);

    m_currentClass = oldCurrentClass;
}

static inline QString fieldSignatureWithType(const VariableModelItem &field)
{
    return field->name() + QStringLiteral(" -> ") + field->type().toString();
}

static inline QString qualifiedFieldSignatureWithType(const QString &className,
                                                      const VariableModelItem &field)
{
    return className + colonColon() + fieldSignatureWithType(field);
}

AbstractMetaField *AbstractMetaBuilderPrivate::traverseField(const VariableModelItem &field,
                                                             const AbstractMetaClass *cls)
{
    QString fieldName = field->name();
    QString className = m_currentClass->typeEntry()->qualifiedCppName();

    // Ignore friend decl.
    if (field->isFriend())
        return 0;

    if (field->accessPolicy() == CodeModel::Private)
        return 0;

    QString rejectReason;
    if (TypeDatabase::instance()->isFieldRejected(className, fieldName, &rejectReason)) {
        m_rejectedFields.insert(qualifiedFieldSignatureWithType(className, field) + rejectReason,
                                AbstractMetaBuilder::GenerationDisabled);
        return 0;
    }


    AbstractMetaField *metaField = new AbstractMetaField;
    metaField->setName(fieldName);
    metaField->setEnclosingClass(cls);

    TypeInfo fieldType = field->type();
    AbstractMetaType *metaType = translateType(fieldType);

    if (!metaType) {
        const QString type = TypeInfo::resolveType(fieldType, currentScope()).qualifiedName().join(colonColon());
        if (m_currentClass->typeEntry()->codeGeneration() & TypeEntry::GenerateTargetLang) {
            qCWarning(lcShiboken).noquote().nospace()
                 << QStringLiteral("skipping field '%1::%2' with unmatched type '%3'")
                                   .arg(m_currentClass->name(), fieldName, type);
        }
        delete metaField;
        return 0;
    }

    metaField->setType(metaType);

    AbstractMetaAttributes::Attributes attr = 0;
    if (field->isStatic())
        attr |= AbstractMetaAttributes::Static;

    CodeModel::AccessPolicy policy = field->accessPolicy();
    if (policy == CodeModel::Public)
        attr |= AbstractMetaAttributes::Public;
    else if (policy == CodeModel::Protected)
        attr |= AbstractMetaAttributes::Protected;
    else
        attr |= AbstractMetaAttributes::Private;
    metaField->setAttributes(attr);

    return metaField;
}

void AbstractMetaBuilderPrivate::traverseFields(const ScopeModelItem &scope_item,
                                                AbstractMetaClass *metaClass)
{
    const VariableList &variables = scope_item->variables();
    for (const VariableModelItem &field : variables) {
        AbstractMetaField* metaField = traverseField(field, metaClass);

        if (metaField && !metaField->isModifiedRemoved()) {
            metaField->setOriginalAttributes(metaField->attributes());
            metaClass->addField(metaField);
        }
    }
}

void AbstractMetaBuilderPrivate::setupFunctionDefaults(AbstractMetaFunction *metaFunction,
                                                       AbstractMetaClass *metaClass)
{
    // Set the default value of the declaring class. This may be changed
    // in fixFunctions later on
    metaFunction->setDeclaringClass(metaClass);

    // Some of the queries below depend on the implementing class being set
    // to function properly. Such as function modifications
    metaFunction->setImplementingClass(metaClass);

    if (metaFunction->name() == QLatin1String("operator_equal"))
        metaClass->setHasEqualsOperator(true);
}

void AbstractMetaBuilderPrivate::fixReturnTypeOfConversionOperator(AbstractMetaFunction *metaFunction)
{
    if (!metaFunction->isConversionOperator())
        return;

    TypeDatabase* types = TypeDatabase::instance();
    static const QRegularExpression operatorRegExp(QStringLiteral("^operator "));
    Q_ASSERT(operatorRegExp.isValid());
    QString castTo = metaFunction->name().remove(operatorRegExp).trimmed();

    if (castTo.endsWith(QLatin1Char('&')))
        castTo.chop(1);
    if (castTo.startsWith(QLatin1String("const ")))
        castTo.remove(0, 6);

    TypeEntry* retType = types->findType(castTo);
    if (!retType)
        return;

    AbstractMetaType* metaType = new AbstractMetaType;
    metaType->setTypeEntry(retType);
    metaFunction->replaceType(metaType);
}

static bool _compareAbstractMetaTypes(const AbstractMetaType* type, const AbstractMetaType* other)
{
    return (type != nullptr) == (other != nullptr)
        && (type == nullptr || *type == *other);
}

static bool _compareAbstractMetaFunctions(const AbstractMetaFunction* func, const AbstractMetaFunction* other)
{
    if (!func && !other)
        return true;
    if (!func || !other)
        return false;
    if (func->arguments().count() != other->arguments().count()
        || func->isConstant() != other->isConstant()
        || func->isStatic() != other->isStatic()
        || !_compareAbstractMetaTypes(func->type(), other->type())) {
        return false;
    }
    for (int i = 0; i < func->arguments().count(); ++i) {
        if (!_compareAbstractMetaTypes(func->arguments().at(i)->type(), other->arguments().at(i)->type()))
            return false;
    }
    return true;
}

AbstractMetaFunctionList AbstractMetaBuilderPrivate::classFunctionList(const ScopeModelItem &scopeItem,
                                                                       bool *constructorRejected)
{
    *constructorRejected = false;
    AbstractMetaFunctionList result;
    const FunctionList &scopeFunctionList = scopeItem->functions();
    result.reserve(scopeFunctionList.size());
    for (const FunctionModelItem &function : scopeFunctionList) {
        if (AbstractMetaFunction *metaFunction = traverseFunction(function))
            result.append(metaFunction);
        else if (function->functionType() == CodeModel::Constructor)
            *constructorRejected = true;
    }
    return result;
}

// For template classes, entries with more specific types may exist from out-of-
// line definitions. If there is a declaration which matches it after fixing
// the parameters, remove it as duplicate. For example:
// template class<T> Vector { public:
//     Vector(const Vector &rhs);
// };
// template class<T>
// Vector<T>::Vector(const Vector<T>&) {} // More specific, remove declaration.

class DuplicatingFunctionPredicate : public std::unary_function<bool, const AbstractMetaFunction *> {
public:
    explicit DuplicatingFunctionPredicate(const AbstractMetaFunction *f) : m_function(f) {}

    bool operator()(const AbstractMetaFunction *rhs) const
    {
        return rhs != m_function && rhs->name() == m_function->name()
            && _compareAbstractMetaFunctions(m_function, rhs);
    }

private:
    const AbstractMetaFunction *m_function;
};

void AbstractMetaBuilderPrivate::traverseFunctions(ScopeModelItem scopeItem,
                                                   AbstractMetaClass *metaClass)
{
    bool constructorRejected = false;
    const AbstractMetaFunctionList functions =
        classFunctionList(scopeItem, &constructorRejected);

    if (constructorRejected)
        *metaClass += AbstractMetaAttributes::HasRejectedConstructor;

    for (AbstractMetaFunction *metaFunction : functions){
        metaFunction->setOriginalAttributes(metaFunction->attributes());
        if (metaClass->isNamespace())
            *metaFunction += AbstractMetaAttributes::Static;

        QPropertySpec *read = 0;
        if (!metaFunction->isSignal() && (read = metaClass->propertySpecForRead(metaFunction->name()))) {
            // Property reader must be in the form "<type> name()"
            if (metaFunction->type() && (read->type() == metaFunction->type()->typeEntry()) && (metaFunction->arguments().size() == 0)) {
                *metaFunction += AbstractMetaAttributes::PropertyReader;
                metaFunction->setPropertySpec(read);
            }
        } else if (QPropertySpec* write = metaClass->propertySpecForWrite(metaFunction->name())) {
            // Property setter must be in the form "void name(<type>)"
            // make sure the function was created with all aguments, some argument can be missing during the pareser because of errors on typesystem
            if ((!metaFunction->type()) && (metaFunction->arguments().size() == 1) && (write->type() == metaFunction->arguments().at(0)->type()->typeEntry())) {
                *metaFunction += AbstractMetaAttributes::PropertyWriter;
                metaFunction->setPropertySpec(write);
            }
        } else if (QPropertySpec* reset = metaClass->propertySpecForReset(metaFunction->name())) {
            // Property resetter must be in the form "void name()"
            if ((!metaFunction->type()) && metaFunction->arguments().isEmpty()) {
                *metaFunction += AbstractMetaAttributes::PropertyResetter;
                metaFunction->setPropertySpec(reset);
            }
        }

        const bool isInvalidDestructor = metaFunction->isDestructor() && metaFunction->isPrivate();
        const bool isInvalidConstructor = metaFunction->functionType() == AbstractMetaFunction::ConstructorFunction
            && metaFunction->isPrivate();
        if (isInvalidConstructor)
            metaClass->setHasPrivateConstructor(true);
        if ((isInvalidDestructor || isInvalidConstructor)
            && !metaClass->hasNonPrivateConstructor()) {
            *metaClass += AbstractMetaAttributes::FinalInTargetLang;
        } else if (metaFunction->isConstructor() && !metaFunction->isPrivate()) {
            *metaClass -= AbstractMetaAttributes::FinalInTargetLang;
            metaClass->setHasNonPrivateConstructor(true);
        }

        // Classes with virtual destructors should always have a shell class
        // (since we aren't registering the destructors, we need this extra check)
        if (metaFunction->isDestructor() && metaFunction->isVirtual()
            && metaFunction->visibility() != AbstractMetaAttributes::Private) {
            metaClass->setForceShellClass(true);
        }

        if (!metaFunction->isDestructor()
            && !(metaFunction->isPrivate() && metaFunction->functionType() == AbstractMetaFunction::ConstructorFunction)) {

            setupFunctionDefaults(metaFunction, metaClass);

            if (metaFunction->isSignal() && metaClass->hasSignal(metaFunction)) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("signal '%1' in class '%2' is overloaded.")
                                      .arg(metaFunction->name(), metaClass->name());
            }

            if (metaFunction->isSignal() && !metaClass->isQObject()) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("signal '%1' in non-QObject class '%2'")
                                      .arg(metaFunction->name(), metaClass->name());
            }

            if (metaFunction->isConversionOperator())
                fixReturnTypeOfConversionOperator(metaFunction);

            metaClass->addFunction(metaFunction);
            applyFunctionModifications(metaFunction);
        } else if (metaFunction->isDestructor()) {
            metaClass->setHasPrivateDestructor(metaFunction->isPrivate());
            metaClass->setHasProtectedDestructor(metaFunction->isProtected());
            metaClass->setHasVirtualDestructor(metaFunction->isVirtual());
        }
        if (!metaFunction->ownerClass()) {
            delete metaFunction;
            metaFunction = 0;
        }
    }

    fillAddedFunctions(metaClass);
}

void AbstractMetaBuilderPrivate::fillAddedFunctions(AbstractMetaClass *metaClass)
{
    // Add the functions added by the typesystem
    const AddedFunctionList &addedFunctions = metaClass->typeEntry()->addedFunctions();
    for (const AddedFunction &addedFunc : addedFunctions) {
        if (!traverseFunction(addedFunc, metaClass)) {
                qFatal("Unable to traverse function \"%s\" added to \"%s\".",
                       qPrintable(addedFunc.name()), qPrintable(metaClass->name()));
        }
    }
}

void AbstractMetaBuilderPrivate::applyFunctionModifications(AbstractMetaFunction *func)
{
    const FunctionModificationList &mods = func->modifications(func->implementingClass());
    AbstractMetaFunction& funcRef = *func;
    for (const FunctionModification &mod : mods) {
        if (mod.isRenameModifier()) {
            func->setOriginalName(func->name());
            func->setName(mod.renamedTo());
        } else if (mod.isAccessModifier()) {
            funcRef -= AbstractMetaAttributes::Public;
            funcRef -= AbstractMetaAttributes::Protected;
            funcRef -= AbstractMetaAttributes::Private;
            funcRef -= AbstractMetaAttributes::Friendly;

            if (mod.isPublic())
                funcRef += AbstractMetaAttributes::Public;
            else if (mod.isProtected())
                funcRef += AbstractMetaAttributes::Protected;
            else if (mod.isPrivate())
                funcRef += AbstractMetaAttributes::Private;
            else if (mod.isFriendly())
                funcRef += AbstractMetaAttributes::Friendly;
        }

        if (mod.isFinal())
            funcRef += AbstractMetaAttributes::FinalInTargetLang;
        else if (mod.isNonFinal())
            funcRef -= AbstractMetaAttributes::FinalInTargetLang;
    }
}

bool AbstractMetaBuilderPrivate::setupInheritance(AbstractMetaClass *metaClass)
{
    Q_ASSERT(!metaClass->isInterface());

    if (m_setupInheritanceDone.contains(metaClass))
        return true;

    m_setupInheritanceDone.insert(metaClass);

    QStringList baseClasses = metaClass->baseClassNames();

    // we only support our own containers and ONLY if there is only one baseclass
    if (baseClasses.size() == 1 && baseClasses.constFirst().contains(QLatin1Char('<'))) {
        TypeInfo info;
        ComplexTypeEntry* baseContainerType;
        AbstractMetaClass* templ = findTemplateClass(baseClasses.constFirst(), metaClass, &info, &baseContainerType);
        if (templ) {
            setupInheritance(templ);
            inheritTemplate(metaClass, templ, info);
            metaClass->typeEntry()->setBaseContainerType(templ->typeEntry());
            return true;
        }

        if (baseContainerType) {
            // Container types are not necessarily wrapped as 'real' classes,
            // but there may still be classes derived from them. In such case,
            // we still need to set the base container type in order to
            // generate correct code for type conversion checking.
            //
            // Additionally, we consider this case as successfully setting up
            // inheritance.
            metaClass->typeEntry()->setBaseContainerType(baseContainerType);
            return true;
        }

        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("template baseclass '%1' of '%2' is not known")
                              .arg(baseClasses.constFirst(), metaClass->name());
        return false;
    }

    TypeDatabase* types = TypeDatabase::instance();

    int primary = -1;
    int primaries = 0;
    for (int i = 0; i < baseClasses.size(); ++i) {

        if (types->isClassRejected(baseClasses.at(i)))
            continue;

        TypeEntry* baseClassEntry = types->findType(baseClasses.at(i));
        if (!baseClassEntry) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("class '%1' inherits from unknown base class '%2'")
                                  .arg(metaClass->name(), baseClasses.at(i));
        } else if (!baseClassEntry->designatedInterface()) { // true for primary base class
            primaries++;
            primary = i;
        }
    }

    if (primary >= 0) {
        AbstractMetaClass *baseClass = AbstractMetaClass::findClass(m_metaClasses, baseClasses.at(primary));
        if (!baseClass) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("unknown baseclass for '%1': '%2'")
                                  .arg(metaClass->name(), baseClasses.at(primary));
            return false;
        }
        metaClass->setBaseClass(baseClass);
    }

    for (int i = 0; i < baseClasses.size(); ++i) {
        if (types->isClassRejected(baseClasses.at(i)))
            continue;

        if (i != primary) {
            AbstractMetaClass *baseClass = AbstractMetaClass::findClass(m_metaClasses, baseClasses.at(i));
            if (!baseClass) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("class not found for setup inheritance '%1'").arg(baseClasses.at(i));
                return false;
            }

            setupInheritance(baseClass);

            QString interfaceName = baseClass->isInterface() ? InterfaceTypeEntry::interfaceName(baseClass->name()) : baseClass->name();
            AbstractMetaClass *iface = AbstractMetaClass::findClass(m_metaClasses, interfaceName);
            if (!iface) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("unknown interface for '%1': '%2'").arg(metaClass->name(), interfaceName);
                return false;
            }
            metaClass->addInterface(iface);

            const AbstractMetaClassList &interfaces = iface->interfaces();
            for (AbstractMetaClass* iface : interfaces)
                metaClass->addInterface(iface);
        }
    }

    return true;
}

void AbstractMetaBuilderPrivate::traverseEnums(const ScopeModelItem &scopeItem,
                                               AbstractMetaClass *metaClass,
                                               const QStringList &enumsDeclarations)
{
    const EnumList &enums = scopeItem->enums();
    for (const EnumModelItem &enumItem : enums) {
        AbstractMetaEnum* metaEnum = traverseEnum(enumItem, metaClass, QSet<QString>::fromList(enumsDeclarations));
        if (metaEnum) {
            metaClass->addEnum(metaEnum);
            metaEnum->setEnclosingClass(metaClass);
        }
    }
}

AbstractMetaFunction* AbstractMetaBuilderPrivate::traverseFunction(const AddedFunction& addedFunc)
{
    return traverseFunction(addedFunc, 0);
}

AbstractMetaFunction* AbstractMetaBuilderPrivate::traverseFunction(const AddedFunction& addedFunc,
                                                                   AbstractMetaClass *metaClass)
{
    AbstractMetaFunction *metaFunction = new AbstractMetaFunction;
    metaFunction->setConstant(addedFunc.isConstant());
    metaFunction->setName(addedFunc.name());
    metaFunction->setOriginalName(addedFunc.name());
    AbstractMetaClass::Attributes visibility =
        addedFunc.access() == AddedFunction::Public
        ? AbstractMetaAttributes::Public : AbstractMetaAttributes::Protected;
    metaFunction->setVisibility(visibility);
    metaFunction->setUserAdded(true);
    AbstractMetaAttributes::Attribute isStatic = addedFunc.isStatic() ? AbstractMetaFunction::Static : AbstractMetaFunction::None;
    metaFunction->setAttributes(metaFunction->attributes() | AbstractMetaAttributes::FinalInTargetLang | isStatic);
    metaFunction->setType(translateType(addedFunc.returnType()));


    QVector<AddedFunction::TypeInfo> args = addedFunc.arguments();
    AbstractMetaArgumentList metaArguments;

    for (int i = 0; i < args.count(); ++i) {
        AddedFunction::TypeInfo& typeInfo = args[i];
        AbstractMetaArgument *metaArg = new AbstractMetaArgument;
        AbstractMetaType *type = translateType(typeInfo);
        if (Q_UNLIKELY(!type)) {
            qCWarning(lcShiboken,
                      "Unable to translate type \"%s\" of argument %d of added function \"%s\".",
                      qPrintable(typeInfo.name), i + 1, qPrintable(addedFunc.name()));
            delete metaFunction;
            return nullptr;
        }
        type->decideUsagePattern();
        metaArg->setType(type);
        metaArg->setArgumentIndex(i);
        metaArg->setDefaultValueExpression(typeInfo.defaultValue);
        metaArg->setOriginalDefaultValueExpression(typeInfo.defaultValue);
        metaArguments.append(metaArg);
    }

    metaFunction->setArguments(metaArguments);
    if (metaFunction->isOperatorOverload() && !metaFunction->isCallOperator()) {
        if (metaArguments.size() > 2) {
            qCWarning(lcShiboken) << "An operator overload need to have 0, 1 or 2 arguments if it's reverse.";
        } else if (metaArguments.size() == 2) {
            // Check if it's a reverse operator
            if (metaArguments[1]->type()->typeEntry() == metaClass->typeEntry()) {
                metaFunction->setReverseOperator(true);
                // we need to call these two function to cache the old signature (with two args)
                // we do this buggy behaviour to comply with the original apiextractor buggy behaviour.
                metaFunction->signature();
                metaFunction->minimalSignature();
                metaArguments.removeLast();
                metaFunction->setArguments(metaArguments);
            } else {
                qCWarning(lcShiboken) << "Operator overload can have two arguments only if it's a reverse operator!";
            }
        }
    }


    // Find the correct default values
    for (int i = 0; i < metaArguments.size(); ++i) {
        AbstractMetaArgument* metaArg = metaArguments.at(i);

        //use relace-default-expression for set default value
        QString replacedExpression;
        if (m_currentClass)
            replacedExpression = metaFunction->replacedDefaultExpression(m_currentClass, i + 1);

        if (!replacedExpression.isEmpty()) {
            if (!metaFunction->removedDefaultExpression(m_currentClass, i + 1)) {
                metaArg->setDefaultValueExpression(replacedExpression);
                metaArg->setOriginalDefaultValueExpression(replacedExpression);
            }
        }
    }

    metaFunction->setOriginalAttributes(metaFunction->attributes());
    if (!metaArguments.isEmpty())
        fixArgumentNames(metaFunction, metaFunction->modifications(m_currentClass));

    if (metaClass) {
        const AbstractMetaArgumentList fargs = metaFunction->arguments();
        if (metaClass->isNamespace())
            *metaFunction += AbstractMetaFunction::Static;
        if (metaFunction->name() == metaClass->name()) {
            metaFunction->setFunctionType(AbstractMetaFunction::ConstructorFunction);
            if (fargs.size() == 1) {
                const TypeEntry *te = fargs.constFirst()->type()->typeEntry();
                if (te->isCustom())
                    metaFunction->setExplicit(true);
                if (te->name() == metaFunction->name())
                    metaFunction->setFunctionType(AbstractMetaFunction::CopyConstructorFunction);
            }
        } else {
            metaFunction->setFunctionType(AbstractMetaFunction::NormalFunction);
        }

        metaFunction->setDeclaringClass(metaClass);
        metaFunction->setImplementingClass(metaClass);
        metaClass->addFunction(metaFunction);
        metaClass->setHasNonPrivateConstructor(true);
    }

    return metaFunction;
}

void AbstractMetaBuilderPrivate::fixArgumentNames(AbstractMetaFunction *func, const FunctionModificationList &mods)
{
    for (const FunctionModification &mod : mods) {
        for (const ArgumentModification &argMod : mod.argument_mods) {
            if (!argMod.renamed_to.isEmpty()) {
                AbstractMetaArgument* arg = func->arguments().at(argMod.index - 1);
                arg->setOriginalName(arg->name());
                arg->setName(argMod.renamed_to, false);
            }
        }
    }

    AbstractMetaArgumentList arguments = func->arguments();
    for (int i = 0, size = arguments.size(); i < size; ++i) {
        if (arguments.at(i)->name().isEmpty())
            arguments[i]->setName(QLatin1String("arg__") + QString::number(i + 1), false);
    }
}

static QString functionSignature(const FunctionModelItem &functionItem)
{
    QStringList args;
    const ArgumentList &arguments = functionItem->arguments();
    for (const ArgumentModelItem &arg : arguments)
        args << arg->type().toString();
    return functionItem->name() + QLatin1Char('(') + args.join(QLatin1Char(',')) + QLatin1Char(')');
}

static inline QString qualifiedFunctionSignatureWithType(const FunctionModelItem &functionItem,
                                                         const QString &className = QString())
{
    QString result = functionItem->type().toString() + QLatin1Char(' ');
    if (!className.isEmpty())
        result += className + colonColon();
    result += functionSignature(functionItem);
    return result;
}
static inline AbstractMetaFunction::FunctionType functionTypeFromCodeModel(CodeModel::FunctionType ft)
{
    AbstractMetaFunction::FunctionType result = AbstractMetaFunction::NormalFunction;
    switch (ft) {
    case CodeModel::Constructor:
        result = AbstractMetaFunction::ConstructorFunction;
        break;
    case CodeModel::CopyConstructor:
        result = AbstractMetaFunction::CopyConstructorFunction;
        break;
    case CodeModel::MoveConstructor:
        result = AbstractMetaFunction::MoveConstructorFunction;
        break;
    case CodeModel::Destructor:
        result = AbstractMetaFunction::DestructorFunction;
        break;
    case CodeModel::Normal:
        break;
    case CodeModel::Signal:
        result = AbstractMetaFunction::SignalFunction;
        break;
    case CodeModel::Slot:
        result = AbstractMetaFunction::SlotFunction;
        break;
    }
    return result;
}

bool AbstractMetaBuilderPrivate::setArrayArgumentType(AbstractMetaFunction *func,
                                                      const FunctionModelItem &functionItem,
                                                      int i)
{
    if (i < 0 || i >= func->arguments().size()) {
        qCWarning(lcShiboken).noquote()
            << msgCannotSetArrayUsage(func->minimalSignature(), i,
                                      QLatin1String("Index out of range."));
        return false;
    }
    AbstractMetaType *metaType = func->arguments().at(i)->type();
    if (metaType->indirections() == 0) {
        qCWarning(lcShiboken).noquote()
            << msgCannotSetArrayUsage(func->minimalSignature(), i,
                                      QLatin1String("Type does not have indirections."));
        return false;
    }
    TypeInfo elementType = functionItem->arguments().at(i)->type();
    elementType.setIndirections(elementType.indirections() - 1);
    AbstractMetaType *element = translateType(elementType);
    if (element == nullptr) {
        qCWarning(lcShiboken).noquote()
           << msgCannotSetArrayUsage(func->minimalSignature(), i,
                                     QLatin1String("Cannot translate element type ") + elementType.toString());
        return false;
    }
    metaType->setArrayElementType(element);
    metaType->setTypeUsagePattern(AbstractMetaType::NativePointerAsArrayPattern);
    return true;
}

static bool generateExceptionHandling(const AbstractMetaFunction *func,
                                      ExceptionSpecification spec,
                                      TypeSystem::ExceptionHandling handling)
{
    switch (func->functionType()) {
    case AbstractMetaFunction::CopyConstructorFunction:
    case AbstractMetaFunction::MoveConstructorFunction:
    case AbstractMetaFunction::AssignmentOperatorFunction:
    case AbstractMetaFunction::MoveAssignmentOperatorFunction:
    case AbstractMetaFunction::DestructorFunction:
        return false;
    default:
        break;
    }
    switch (handling) {
    case TypeSystem::ExceptionHandling::On:
        return true;
    case TypeSystem::ExceptionHandling::AutoDefaultToOn:
        return spec != ExceptionSpecification::NoExcept;
    case TypeSystem::ExceptionHandling::AutoDefaultToOff:
        return spec == ExceptionSpecification::Throws;
    default:
        break;
    }
    return false;
}

AbstractMetaFunction *AbstractMetaBuilderPrivate::traverseFunction(const FunctionModelItem &functionItem)
{
    if (functionItem->isDeleted() || !functionItem->templateParameters().isEmpty())
        return nullptr;
    QString functionName = functionItem->name();
    QString className;
    TypeSystem::ExceptionHandling exceptionHandling = TypeSystem::ExceptionHandling::Unspecified;
    if (m_currentClass) {
        // Clang: Skip qt_metacast(), qt_metacall(), expanded from Q_OBJECT
        // and overridden metaObject(), QGADGET helpers
        if (functionName == QLatin1String("qt_check_for_QGADGET_macro")
            || functionName.startsWith(QLatin1String("qt_meta"))) {
            return nullptr;
        }
        className = m_currentClass->typeEntry()->qualifiedCppName();
        exceptionHandling = m_currentClass->typeEntry()->exceptionHandling();
        if (functionName == QLatin1String("metaObject") && className != QLatin1String("QObject"))
            return nullptr;
    }

    // Store original signature with unresolved typedefs for message/log purposes
    const QString originalQualifiedSignatureWithReturn =
        qualifiedFunctionSignatureWithType(functionItem, className);

    QString rejectReason;
    if (TypeDatabase::instance()->isFunctionRejected(className, functionName, &rejectReason)) {
        m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn + rejectReason, AbstractMetaBuilder::GenerationDisabled);
        return nullptr;
    }
    const QString &signature = functionSignature(functionItem);
    const bool rejected =
        TypeDatabase::instance()->isFunctionRejected(className, signature, &rejectReason);
    qCDebug(lcShiboken).nospace().noquote() << __FUNCTION__
        << ": Checking rejection for signature \"" << signature << "\" for " << className
        << ": " << rejected;
    if (rejected)
        return nullptr;

    if (functionItem->isFriend())
        return 0;

    AbstractMetaFunction *metaFunction = new AbstractMetaFunction;
    // Additional check for assignment/move assignment down below
    metaFunction->setFunctionType(functionTypeFromCodeModel(functionItem->functionType()));
    metaFunction->setConstant(functionItem->isConstant());
    metaFunction->setExceptionSpecification(functionItem->exceptionSpecification());

    if (ReportHandler::isDebug(ReportHandler::MediumDebug))
        qCDebug(lcShiboken).noquote().nospace() << " - " << functionName << "()";

    metaFunction->setName(functionName);
    metaFunction->setOriginalName(functionItem->name());

    if (functionItem->isAbstract())
        *metaFunction += AbstractMetaAttributes::Abstract;

    if (functionItem->isVirtual()) {
        *metaFunction += AbstractMetaAttributes::VirtualCppMethod;
        if (functionItem->isOverride())
            *metaFunction += AbstractMetaAttributes::OverriddenCppMethod;
        if (functionItem->isFinal())
            *metaFunction += AbstractMetaAttributes::FinalCppMethod;
    } else {
        *metaFunction += AbstractMetaAttributes::FinalInTargetLang;
    }

    if (functionItem->isInvokable())
        *metaFunction += AbstractMetaAttributes::Invokable;

    if (functionItem->isStatic()) {
        *metaFunction += AbstractMetaAttributes::Static;
        *metaFunction += AbstractMetaAttributes::FinalInTargetLang;
    }

    // Access rights
    if (functionItem->accessPolicy() == CodeModel::Public)
        *metaFunction += AbstractMetaAttributes::Public;
    else if (functionItem->accessPolicy() == CodeModel::Private)
        *metaFunction += AbstractMetaAttributes::Private;
    else
        *metaFunction += AbstractMetaAttributes::Protected;

    QString errorMessage;
    switch (metaFunction->functionType()) {
    case AbstractMetaFunction::DestructorFunction:
        break;
    case AbstractMetaFunction::ConstructorFunction:
        metaFunction->setExplicit(functionItem->isExplicit());
        metaFunction->setName(m_currentClass->name());
        break;
    default: {
        TypeInfo returnType = functionItem->type();

        if (TypeDatabase::instance()->isReturnTypeRejected(className, returnType.toString(), &rejectReason)) {
            m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn + rejectReason, AbstractMetaBuilder::GenerationDisabled);
            delete metaFunction;
            return nullptr;
        }

        AbstractMetaType *type = nullptr;
        if (!returnType.isVoid()) {
            type = translateType(returnType, true, &errorMessage);
            if (!type) {
                const QString reason = msgUnmatchedReturnType(functionItem, errorMessage);
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgSkippingFunction(functionItem, originalQualifiedSignatureWithReturn, reason)));
                m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn, AbstractMetaBuilder::UnmatchedReturnType);
                delete metaFunction;
                return nullptr;
            }
        }

        metaFunction->setType(type);
    }
        break;
    }

    ArgumentList arguments = functionItem->arguments();

    if (arguments.size() == 1) {
        ArgumentModelItem arg = arguments.at(0);
        TypeInfo type = arg->type();
        if (type.qualifiedName().constFirst() == QLatin1String("void") && type.indirections() == 0)
            arguments.pop_front();
    }

    AbstractMetaArgumentList metaArguments;

    for (int i = 0; i < arguments.size(); ++i) {
        const ArgumentModelItem &arg = arguments.at(i);

        if (TypeDatabase::instance()->isArgumentTypeRejected(className, arg->type().toString(), &rejectReason)) {
            m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn + rejectReason, AbstractMetaBuilder::GenerationDisabled);
            delete metaFunction;
            return nullptr;
        }

        AbstractMetaType *metaType = translateType(arg->type(), true, &errorMessage);
        if (!metaType) {
            // If an invalid argument has a default value, simply remove it
            if (arg->defaultValue()) {
                if (!m_currentClass
                    || (m_currentClass->typeEntry()->codeGeneration()
                        & TypeEntry::GenerateTargetLang)) {
                    qCWarning(lcShiboken).noquote().nospace()
                    << "Stripping argument #" << (i + 1) << " of "
                    << originalQualifiedSignatureWithReturn
                    << " due to unmatched type \"" << arg->type().toString()
                    << "\" with default expression \""
                    << arg->defaultValueExpression() << "\".";
                }
                break;
            }
            Q_ASSERT(metaType == 0);
            const QString reason = msgUnmatchedParameterType(arg, i, errorMessage);
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgSkippingFunction(functionItem, originalQualifiedSignatureWithReturn, reason)));
            const QString rejectedFunctionSignature = originalQualifiedSignatureWithReturn
                + QLatin1String(": ") + reason;
            m_rejectedFunctions.insert(rejectedFunctionSignature, AbstractMetaBuilder::UnmatchedArgumentType);
            delete metaFunction;
            return nullptr;
        }

        AbstractMetaArgument *metaArgument = new AbstractMetaArgument;

        metaArgument->setType(metaType);
        metaArgument->setName(arg->name());
        metaArgument->setArgumentIndex(i);
        metaArguments << metaArgument;
    }

    metaFunction->setArguments(metaArguments);

    const FunctionModificationList functionMods = metaFunction->modifications(m_currentClass);

    for (const FunctionModification &mod : functionMods) {
        if (mod.exceptionHandling() != TypeSystem::ExceptionHandling::Unspecified) {
            exceptionHandling = mod.exceptionHandling();
            break;
        }
    }

    metaFunction->setGenerateExceptionHandling(generateExceptionHandling(metaFunction,
                                                                         functionItem->exceptionSpecification(),
                                                                         exceptionHandling));

    // Find the correct default values
    for (int i = 0, size = metaArguments.size(); i < size; ++i) {
        const ArgumentModelItem &arg = arguments.at(i);
        AbstractMetaArgument* metaArg = metaArguments.at(i);

        //use relace-default-expression for set default value
        QString replacedExpression;
        if (m_currentClass) {
            replacedExpression = metaFunction->replacedDefaultExpression(m_currentClass, i + 1);
        } else {
            if (!functionMods.isEmpty()) {
                QVector<ArgumentModification> argMods = functionMods.constFirst().argument_mods;
                if (!argMods.isEmpty())
                    replacedExpression = argMods.constFirst().replacedDefaultExpression;
            }
        }

        bool hasDefaultValue = false;
        if (arg->defaultValue() || !replacedExpression.isEmpty()) {
            QString expr = arg->defaultValueExpression();
            expr = fixDefaultValue(arg, metaArg->type(), metaFunction, m_currentClass, i);
            metaArg->setOriginalDefaultValueExpression(expr);

            if (metaFunction->removedDefaultExpression(m_currentClass, i + 1)) {
                expr.clear();
            } else if (!replacedExpression.isEmpty()) {
                expr = replacedExpression;
            }
            metaArg->setDefaultValueExpression(expr);
            hasDefaultValue = !expr.isEmpty();
        }

        //Check for missing argument name
        if (hasDefaultValue
            && !metaArg->hasName()
            && !metaFunction->isOperatorOverload()
            && !metaFunction->isSignal()
            && metaFunction->argumentName(i+1, false, m_currentClass).isEmpty()) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Argument %1 on function '%2::%3' has default expression but does not have name.")
                                  .arg(i+1).arg(className, metaFunction->minimalSignature());
        }

    }

    if (!metaArguments.isEmpty()) {
        fixArgumentNames(metaFunction, functionMods);
        for (const FunctionModification &mod : functionMods) {
            for (const ArgumentModification &argMod : mod.argument_mods) {
                if (argMod.array)
                    setArrayArgumentType(metaFunction, functionItem, argMod.index - 1);
            }
        }
    }

    // Determine class special functions
    if (m_currentClass && metaFunction->arguments().size() == 1) {
        const AbstractMetaType *argType = metaFunction->arguments().constFirst()->type();
        if (argType->typeEntry() == m_currentClass->typeEntry() && argType->indirections() == 0) {
            if (metaFunction->name() == QLatin1String("operator=")) {
                switch (argType->referenceType()) {
                case NoReference:
                    metaFunction->setFunctionType(AbstractMetaFunction::AssignmentOperatorFunction);
                    break;
                case LValueReference:
                    if (argType->isConstant())
                        metaFunction->setFunctionType(AbstractMetaFunction::AssignmentOperatorFunction);
                    break;
                case RValueReference:
                    metaFunction->setFunctionType(AbstractMetaFunction::MoveAssignmentOperatorFunction);
                    break;
                }
            }
        }
    }
    return metaFunction;
}

AbstractMetaType *AbstractMetaBuilderPrivate::translateType(const AddedFunction::TypeInfo &typeInfo)
{
    Q_ASSERT(!typeInfo.name.isEmpty());
    TypeDatabase* typeDb = TypeDatabase::instance();
    TypeEntry* type;

    QString typeName = typeInfo.name;

    if (typeName == QLatin1String("void"))
        return 0;

    type = typeDb->findType(typeName);

    // test if the type is a template, like a container
    bool isTemplate = false;
    QStringList templateArgs;
    if (!type && typeInfo.name.contains(QLatin1Char('<'))) {
        const QStringList& parsedType = parseTemplateType(typeInfo.name);
        if (parsedType.isEmpty()) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Template type parsing failed for '%1'").arg(typeInfo.name);
        } else {
            templateArgs = parsedType.mid(1);
            isTemplate = (type = typeDb->findContainerType(parsedType[0]));
        }
    }

    if (!type) {
        QStringList candidates;
        const auto &entries = typeDb->entries();
        for (auto it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
            // Let's try to find the type in different scopes.
            if (it.key().endsWith(colonColon() + typeName))
                candidates.append(it.key());
        }

        QString msg = QStringLiteral("Type '%1' wasn't found in the type database.\n").arg(typeName);

        if (candidates.isEmpty()) {
            qFatal("%sDeclare it in the type system using the proper <*-type> tag.",
                   qPrintable(msg));
        }

        msg += QLatin1String("Remember to inform the full qualified name for the type you want to use.\nCandidates are:\n");
        candidates.sort();
        for (const QString& candidate : qAsConst(candidates)) {
            msg += QLatin1String("    ") + candidate + QLatin1Char('\n');
        }
        qFatal("%s", qPrintable(msg));
    }

    AbstractMetaType *metaType = new AbstractMetaType;
    metaType->setTypeEntry(type);
    metaType->setIndirections(typeInfo.indirections);
    if (typeInfo.isReference)
        metaType->setReferenceType(LValueReference);
    metaType->setConstant(typeInfo.isConstant);
    if (isTemplate) {
        for (const QString& templateArg : qAsConst(templateArgs)) {
            AbstractMetaType *metaArgType = translateType(AddedFunction::TypeInfo::fromSignature(templateArg));
            metaType->addInstantiation(metaArgType);
        }
        metaType->setTypeUsagePattern(AbstractMetaType::ContainerPattern);
    }

    return metaType;
}

static const TypeEntry* findTypeEntryUsingContext(const AbstractMetaClass* metaClass, const QString& qualifiedName)
{
    const TypeEntry* type = 0;
    QStringList context = metaClass->qualifiedCppName().split(colonColon());
    while (!type && !context.isEmpty()) {
        type = TypeDatabase::instance()->findType(context.join(colonColon()) + colonColon() + qualifiedName);
        context.removeLast();
    }
    return type;
}

AbstractMetaType *AbstractMetaBuilderPrivate::translateType(const TypeInfo &_typei,
                                                            bool resolveType,
                                                            QString *errorMessage)
{
    return translateTypeStatic(_typei, m_currentClass, this, resolveType, errorMessage);
}

AbstractMetaType *AbstractMetaBuilderPrivate::translateTypeStatic(const TypeInfo &_typei,
                                                                  AbstractMetaClass *currentClass,
                                                                  AbstractMetaBuilderPrivate *d,
                                                                  bool resolveType,
                                                                  QString *errorMessageIn)
{
    // 1. Test the type info without resolving typedefs in case this is present in the
    //    type system
    if (resolveType) {
        if (AbstractMetaType *resolved = translateTypeStatic(_typei, currentClass, d, false, errorMessageIn))
            return resolved;
    }

    TypeInfo typeInfo = _typei;
    if (resolveType) {
        // Go through all parts of the current scope (including global namespace)
        // to resolve typedefs. The parser does not properly resolve typedefs in
        // the global scope when they are referenced from inside a namespace.
        // This is a work around to fix this bug since fixing it in resolveType
        // seemed non-trivial
        int i = d ? d->m_scopes.size() - 1 : -1;
        while (i >= 0) {
            typeInfo = TypeInfo::resolveType(_typei, d->m_scopes.at(i--));
            if (typeInfo.qualifiedName().join(colonColon()) != _typei.qualifiedName().join(colonColon()))
                break;
        }

    }

    if (typeInfo.isFunctionPointer()) {
        if (errorMessageIn)
            *errorMessageIn = msgUnableToTranslateType(_typei, QLatin1String("Unsupported function pointer."));
        return nullptr;
    }

    QString errorMessage;

    // 2. Handle arrays.
    // 2.1 Handle char arrays with unspecified size (aka "const char[]") as "const char*" with
    // NativePointerPattern usage.
    bool oneDimensionalArrayOfUnspecifiedSize =
            typeInfo.arrayElements().size() == 1
            && typeInfo.arrayElements().at(0).isEmpty();

    bool isConstCharStarCase =
            oneDimensionalArrayOfUnspecifiedSize
            && typeInfo.qualifiedName().size() == 1
            && typeInfo.qualifiedName().at(0) == QStringLiteral("char")
            && typeInfo.indirections() == 0
            && typeInfo.isConstant()
            && typeInfo.referenceType() == NoReference
            && typeInfo.arguments().isEmpty();

    if (isConstCharStarCase)
        typeInfo.setIndirections(typeInfo.indirections() + typeInfo.arrayElements().size());

    // 2.2 Handle regular arrays.
    if (!typeInfo.arrayElements().isEmpty() && !isConstCharStarCase) {
        TypeInfo newInfo;
        //newInfo.setArguments(typeInfo.arguments());
        newInfo.setIndirectionsV(typeInfo.indirectionsV());
        newInfo.setConstant(typeInfo.isConstant());
        newInfo.setVolatile(typeInfo.isVolatile());
        newInfo.setFunctionPointer(typeInfo.isFunctionPointer());
        newInfo.setQualifiedName(typeInfo.qualifiedName());
        newInfo.setReferenceType(typeInfo.referenceType());
        newInfo.setVolatile(typeInfo.isVolatile());

        AbstractMetaType *elementType = translateTypeStatic(newInfo, currentClass, d, true, &errorMessage);
        if (!elementType) {
            if (errorMessageIn) {
                errorMessage.prepend(QLatin1String("Unable to translate array element: "));
                *errorMessageIn = msgUnableToTranslateType(_typei, errorMessage);
            }
            return nullptr;
        }

        for (int i = typeInfo.arrayElements().size() - 1; i >= 0; --i) {
            AbstractMetaType *arrayType = new AbstractMetaType;
            arrayType->setArrayElementType(elementType);
            const QString &arrayElement = typeInfo.arrayElements().at(i);
            if (!arrayElement.isEmpty()) {
                bool _ok;
                const qint64 elems = d
                    ? d->findOutValueFromString(arrayElement, _ok)
                    : arrayElement.toLongLong(&_ok, 0);
                if (_ok)
                    arrayType->setArrayElementCount(int(elems));
            }
            arrayType->setTypeEntry(new ArrayTypeEntry(elementType->typeEntry() , elementType->typeEntry()->version()));
            arrayType->decideUsagePattern();

            elementType = arrayType;
        }

        return elementType;
    }

    QStringList qualifierList = typeInfo.qualifiedName();
    if (qualifierList.isEmpty()) {
        errorMessage = msgUnableToTranslateType(_typei, QLatin1String("horribly broken type"));
        if (errorMessageIn)
            *errorMessageIn = errorMessage;
        else
            qCWarning(lcShiboken,"%s", qPrintable(errorMessage));
        return nullptr;
    }

    QString qualifiedName = qualifierList.join(colonColon());
    QString name = qualifierList.takeLast();

    // 4. Special case QFlags (include instantiation in name)
    if (qualifiedName == QLatin1String("QFlags")) {
        qualifiedName = typeInfo.toString();
        typeInfo.clearInstantiations();
    }

    const TypeEntry *type = 0;
    // 5. Try to find the type

    // 5.1 - Try first using the current scope
    if (currentClass) {
        type = findTypeEntryUsingContext(currentClass, qualifiedName);

        // 5.1.1 - Try using the class parents' scopes
        if (!type && d && !currentClass->baseClassNames().isEmpty()) {
            const AbstractMetaClassList &baseClasses = d->getBaseClasses(currentClass);
            for (const AbstractMetaClass *cls : baseClasses) {
                type = findTypeEntryUsingContext(cls, qualifiedName);
                if (type)
                    break;
            }
        }
    }

    // 5.2 - Try without scope
    if (!type)
        type = TypeDatabase::instance()->findType(qualifiedName);

    // 6. No? Try looking it up as a flags type
    if (!type)
        type = TypeDatabase::instance()->findFlagsType(qualifiedName);

    // 7. No? Try looking it up as a container type
    if (!type)
        type = TypeDatabase::instance()->findContainerType(name);

    // 8. No? Check if the current class is a template and this type is one
    //    of the parameters.
    if (!type && currentClass) {
        const QVector<TypeEntry *> &template_args = currentClass->templateArguments();
        for (TypeEntry *te : template_args) {
            if (te->name() == qualifiedName)
                type = te;
        }
    }

    if (!type) {
        if (errorMessageIn) {
            *errorMessageIn =
                msgUnableToTranslateType(_typei, msgCannotFindTypeEntry(qualifiedName));
        }
        return nullptr;
    }

    // These are only implicit and should not appear in code...
    Q_ASSERT(!type->isInterface());

    AbstractMetaType *metaType = new AbstractMetaType;
    metaType->setTypeEntry(type);
    metaType->setIndirectionsV(typeInfo.indirectionsV());
    metaType->setReferenceType(typeInfo.referenceType());
    metaType->setConstant(typeInfo.isConstant());
    metaType->setVolatile(typeInfo.isVolatile());
    metaType->setOriginalTypeDescription(_typei.toString());

    const auto &templateArguments = typeInfo.instantiations();
    for (int t = 0, size = templateArguments.size(); t < size; ++t) {
        const  TypeInfo &ti = templateArguments.at(t);
        AbstractMetaType *targType = translateTypeStatic(ti, currentClass, d, true, &errorMessage);
        if (!targType) {
            if (errorMessageIn)
                *errorMessageIn = msgCannotTranslateTemplateArgument(t, ti, errorMessage);
            delete metaType;
            return nullptr;
        }

        metaType->addInstantiation(targType, true);
    }

    // The usage pattern *must* be decided *after* the possible template
    // instantiations have been determined, or else the absence of
    // such instantiations will break the caching scheme of
    // AbstractMetaType::cppSignature().
    metaType->decideUsagePattern();

    return metaType;
}

AbstractMetaType *AbstractMetaBuilder::translateType(const TypeInfo &_typei,
                                                     AbstractMetaClass *currentClass,
                                                     bool resolveType,
                                                     QString *errorMessage)
{
    return AbstractMetaBuilderPrivate::translateTypeStatic(_typei, currentClass,
                                                           nullptr, resolveType,
                                                           errorMessage);
}

AbstractMetaType *AbstractMetaBuilder::translateType(const QString &t,
                                                     AbstractMetaClass *currentClass,
                                                     bool resolveType,
                                                     QString *errorMessageIn)
{
    QString errorMessage;
    TypeInfo typeInfo = TypeParser::parse(t, &errorMessage);
    if (typeInfo.qualifiedName().isEmpty()) {
        errorMessage = msgUnableToTranslateType(t, errorMessage);
        if (errorMessageIn)
            *errorMessageIn = errorMessage;
        else
            qCWarning(lcShiboken, "%s", qPrintable(errorMessage));
        return nullptr;
    }
    return translateType(typeInfo, currentClass, resolveType, errorMessageIn);
}

qint64 AbstractMetaBuilderPrivate::findOutValueFromString(const QString &stringValue, bool &ok)
{
    qint64 value = stringValue.toLongLong(&ok);
    if (ok)
        return value;

    if (stringValue == QLatin1String("true") || stringValue == QLatin1String("false")) {
        ok = true;
        return (stringValue == QLatin1String("true"));
    }

    // This is a very lame way to handle expression evaluation,
    // but it is not critical and will do for the time being.
    static const QRegularExpression variableNameRegExp(QStringLiteral("^[a-zA-Z_][a-zA-Z0-9_]*$"));
    Q_ASSERT(variableNameRegExp.isValid());
    if (!variableNameRegExp.match(stringValue).hasMatch()) {
        ok = true;
        return 0;
    }

    AbstractMetaEnumValue *enumValue = AbstractMetaClass::findEnumValue(m_metaClasses, stringValue);
    if (enumValue) {
        ok = true;
        return enumValue->value().value();
    }

    for (AbstractMetaEnum *metaEnum : qAsConst(m_globalEnums)) {
        if (const AbstractMetaEnumValue *ev = metaEnum->findEnumValue(stringValue)) {
            ok = true;
            return ev->value().value();
        }
    }

    ok = false;
    return 0;
}

QString AbstractMetaBuilderPrivate::fixDefaultValue(const ArgumentModelItem &item,
                                                    AbstractMetaType *type,
                                                    AbstractMetaFunction *fnc,
                                                    AbstractMetaClass *implementingClass,
                                                    int /* argumentIndex */)
{
    QString functionName = fnc->name();
    QString className = implementingClass ? implementingClass->qualifiedCppName() : QString();

    QString expr = item->defaultValueExpression();
    if (type) {
        if (type->isPrimitive()) {
            if (type->name() == QLatin1String("boolean")) {
                if (expr != QLatin1String("false") && expr != QLatin1String("true")) {
                    bool ok = false;
                    int number = expr.toInt(&ok);
                    if (ok && number)
                        expr = QLatin1String("true");
                    else
                        expr = QLatin1String("false");
                }
            } else {
                // This can be an enum or flag so I need to delay the
                // translation untill all namespaces are completly
                // processed. This is done in figureOutEnumValues()
            }
        } else if (type->isFlags() || type->isEnum()) {
            bool isNumber;
            expr.toInt(&isNumber);
            if (!isNumber && expr.indexOf(colonColon()) < 0) {
                // Add the enum/flag scope to default value, making it usable
                // from other contexts beside its owner class hierarchy
                static const QRegularExpression typeRegEx(QStringLiteral("[^<]*[<]([^:]*::).*"));
                Q_ASSERT(typeRegEx.isValid());
                const QRegularExpressionMatch match = typeRegEx.match(type->minimalSignature());
                if (match.hasMatch())
                    expr.prepend(match.captured(1));
            }
        } else if (type->isContainer() && expr.contains(QLatin1Char('<'))) {
            static const QRegularExpression typeRegEx(QStringLiteral("[^<]*<(.*)>"));
            Q_ASSERT(typeRegEx.isValid());
            const QRegularExpressionMatch typeMatch = typeRegEx.match(type->minimalSignature());
            static const QRegularExpression defaultRegEx(QLatin1String("([^<]*<).*(>[^>]*)"));
            Q_ASSERT(defaultRegEx.isValid());
            const QRegularExpressionMatch defaultMatch = defaultRegEx.match(expr);
            if (typeMatch.hasMatch() && defaultMatch.hasMatch())
                expr = defaultMatch.captured(1) + typeMatch.captured(1) + defaultMatch.captured(2);
        } else {
            // Here the default value is supposed to be a constructor,
            // a class field, or a constructor receiving a class field
            static const QRegularExpression defaultRegEx(QStringLiteral("([^\\(]*\\(|)([^\\)]*)(\\)|)"));
            Q_ASSERT(defaultRegEx.isValid());
            const QRegularExpressionMatch defaultMatch = defaultRegEx.match(expr);
            QString defaultValueCtorName = defaultMatch.hasMatch() ? defaultMatch.captured(1) : QString();
            if (defaultValueCtorName.endsWith(QLatin1Char('(')))
                defaultValueCtorName.chop(1);

            // Fix the scope for constructor using the already
            // resolved argument type as a reference.
            // The following regular expression extracts any
            // use of namespaces/scopes from the type string.
            static const QRegularExpression typeRegEx(QLatin1String("^(?:const[\\s]+|)([\\w:]*::|)([A-Za-z_]\\w*)\\s*[&\\*]?$"));
            Q_ASSERT(typeRegEx.isValid());
            const QRegularExpressionMatch typeMatch = typeRegEx.match(type->minimalSignature());

            QString typeNamespace = typeMatch.hasMatch() ? typeMatch.captured(1) : QString();
            QString typeCtorName  = typeMatch.hasMatch() ? typeMatch.captured(2) : QString();
            if (!typeNamespace.isEmpty() && defaultValueCtorName == typeCtorName)
                expr.prepend(typeNamespace);

            // Fix scope if the parameter is a field of the current class
            if (implementingClass) {
                const AbstractMetaFieldList &fields = implementingClass->fields();
                for (const AbstractMetaField *field : fields) {
                    if (defaultMatch.hasMatch() && defaultMatch.captured(2) == field->name()) {
                        expr = defaultMatch.captured(1) + implementingClass->name()
                               + colonColon() + defaultMatch.captured(2) + defaultMatch.captured(3);
                        break;
                    }
                }
            }
        }
    } else {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("undefined type for default value '%3' of argument in function '%1', class '%2'")
                              .arg(functionName, className, item->defaultValueExpression());

        expr = QString();
    }

    return expr;
}

bool AbstractMetaBuilderPrivate::isQObject(const FileModelItem &dom, const QString &qualifiedName)
{
    if (qualifiedName == QLatin1String("QObject"))
        return true;

    ClassModelItem classItem = dom->findClass(qualifiedName);

    if (!classItem) {
        QStringList names = qualifiedName.split(colonColon());
        NamespaceModelItem ns = dom;
        for (int i = 0; i < names.size() - 1 && ns; ++i)
            ns = ns->findNamespace(names.at(i));
        if (ns && names.size() >= 2)
            classItem = ns->findClass(names.at(names.size() - 1));
    }

    if (!classItem)
        return false;

    if (classItem->extendsClass(QLatin1String("QObject")))
        return true;

    const QVector<_ClassModelItem::BaseClass> &baseClasses = classItem->baseClasses();
    for (const _ClassModelItem::BaseClass &baseClass : baseClasses) {
        if (isQObject(dom, baseClass.name))
            return true;
    }

    return false;
}


bool AbstractMetaBuilderPrivate::isEnum(const FileModelItem &dom, const QStringList& qualified_name)
{
    CodeModelItem item = dom->model()->findItem(qualified_name, dom);
    return item && item->kind() == _EnumModelItem::__node_kind;
}

AbstractMetaClass* AbstractMetaBuilderPrivate::findTemplateClass(const QString &name,
                                                                 const AbstractMetaClass *context,
                                                                 TypeInfo *info,
                                                                 ComplexTypeEntry **baseContainerType) const
{
    TypeDatabase* types = TypeDatabase::instance();

    QStringList scope = context->typeEntry()->qualifiedCppName().split(colonColon());
    QString errorMessage;
    scope.removeLast();
    for (int i = scope.size(); i >= 0; --i) {
        QString prefix = i > 0 ? QStringList(scope.mid(0, i)).join(colonColon()) + colonColon() : QString();
        QString completeName = prefix + name;
        const TypeInfo parsed = TypeParser::parse(completeName, &errorMessage);
        QString qualifiedName = parsed.qualifiedName().join(colonColon());
        if (qualifiedName.isEmpty()) {
            qWarning().noquote().nospace() << "Unable to parse type \"" << completeName
                << "\" while looking for template \"" << name << "\": " << errorMessage;
            continue;
        }
        if (info)
            *info = parsed;

        AbstractMetaClass* templ = 0;
        for (AbstractMetaClass *c : qAsConst(m_templates)) {
            if (c->typeEntry()->name() == qualifiedName) {
                templ = c;
                break;
            }
        }

        if (!templ)
            templ = AbstractMetaClass::findClass(m_metaClasses, qualifiedName);

        if (templ)
            return templ;

        if (baseContainerType)
            *baseContainerType = types->findContainerType(qualifiedName);
    }

    return 0;
}

AbstractMetaClassList AbstractMetaBuilderPrivate::getBaseClasses(const AbstractMetaClass *metaClass) const
{
    AbstractMetaClassList baseClasses;
    const QStringList &baseClassNames = metaClass->baseClassNames();
    for (const QString& parent : baseClassNames) {
        AbstractMetaClass* cls = 0;
        if (parent.contains(QLatin1Char('<')))
            cls = findTemplateClass(parent, metaClass);
        else
            cls = AbstractMetaClass::findClass(m_metaClasses, parent);

        if (cls)
            baseClasses << cls;
    }
    return baseClasses;
}

bool AbstractMetaBuilderPrivate::ancestorHasPrivateCopyConstructor(const AbstractMetaClass *metaClass) const
{
    if (metaClass->hasPrivateCopyConstructor())
        return true;
    const AbstractMetaClassList &baseClasses = getBaseClasses(metaClass);
    for (const AbstractMetaClass *cls : baseClasses) {
        if (ancestorHasPrivateCopyConstructor(cls))
            return true;
    }
    return false;
}

AbstractMetaType *
    AbstractMetaBuilderPrivate::inheritTemplateType(const AbstractMetaTypeList &templateTypes,
                                                    const AbstractMetaType *metaType)
{
    Q_ASSERT(metaType);

    QScopedPointer<AbstractMetaType> returned(metaType->copy());

    if (!metaType->typeEntry()->isTemplateArgument() && !metaType->hasInstantiations())
        return returned.take();

    returned->setOriginalTemplateType(metaType);

    if (returned->typeEntry()->isTemplateArgument()) {
        const TemplateArgumentEntry* tae = static_cast<const TemplateArgumentEntry*>(returned->typeEntry());

        // If the template is intantiated with void we special case this as rejecting the functions that use this
        // parameter from the instantiation.
        const AbstractMetaType *templateType = templateTypes.value(tae->ordinal());
        if (!templateType || templateType->typeEntry()->isVoid())
            return nullptr;

        AbstractMetaType* t = returned->copy();
        t->setTypeEntry(templateType->typeEntry());
        t->setIndirections(templateType->indirections() + t->indirections() ? 1 : 0);
        t->decideUsagePattern();

        return inheritTemplateType(templateTypes, t);
    }

    if (returned->hasInstantiations()) {
        AbstractMetaTypeList instantiations = returned->instantiations();
        for (int i = 0; i < instantiations.count(); ++i) {
            instantiations[i] =
                inheritTemplateType(templateTypes, instantiations.at(i));
            if (!instantiations.at(i))
                return nullptr;
        }
        returned->setInstantiations(instantiations, true);
    }

    return returned.take();
}

bool AbstractMetaBuilderPrivate::inheritTemplate(AbstractMetaClass *subclass,
                                                 const AbstractMetaClass *templateClass,
                                                 const TypeInfo &info)
{
    QVector<TypeInfo> targs = info.instantiations();
    QVector<AbstractMetaType *> templateTypes;

    if (subclass->isTypeDef()) {
        subclass->setHasCloneOperator(templateClass->hasCloneOperator());
        subclass->setHasEqualsOperator(templateClass->hasEqualsOperator());
        subclass->setHasHashFunction(templateClass->hasHashFunction());
        subclass->setHasNonPrivateConstructor(templateClass->hasNonPrivateConstructor());
        subclass->setHasPrivateDestructor(templateClass->hasPrivateDestructor());
        subclass->setHasProtectedDestructor(templateClass->hasProtectedDestructor());
        subclass->setHasVirtualDestructor(templateClass->hasVirtualDestructor());
    }

    for (const TypeInfo &i : qAsConst(targs)) {
        QString typeName = i.qualifiedName().join(colonColon());
        TypeDatabase *typeDb = TypeDatabase::instance();
        TypeEntry *t = nullptr;
        // Check for a non-type template integer parameter, that is, for a base
        // "template <int R, int C> Matrix<R, C>" and subclass
        // "typedef Matrix<2,3> Matrix2x3;". If so, create dummy entries of
        // EnumValueTypeEntry for the integer values encountered on the fly.
        const bool isNumber = std::all_of(typeName.cbegin(), typeName.cend(),
                                          [](QChar c) { return c.isDigit(); });
        if (isNumber) {
            t = typeDb->findType(typeName);
            if (!t) {
                t = new EnumValueTypeEntry(typeName, typeName, nullptr,
                                           QVersionNumber(0, 0));
                t->setCodeGeneration(0);
                typeDb->addType(t);
            }
        } else {
            QStringList possibleNames;
            possibleNames << subclass->qualifiedCppName() + colonColon() + typeName;
            possibleNames << templateClass->qualifiedCppName() + colonColon() + typeName;
            if (subclass->enclosingClass())
                possibleNames << subclass->enclosingClass()->qualifiedCppName() + colonColon() + typeName;
            possibleNames << typeName;

            for (const QString &possibleName : qAsConst(possibleNames)) {
                t = typeDb->findType(possibleName);
                if (t)
                    break;
            }
        }

        if (t) {
            AbstractMetaType *temporaryType = new AbstractMetaType;
            temporaryType->setTypeEntry(t);
            temporaryType->setConstant(i.isConstant());
            temporaryType->setReferenceType(i.referenceType());
            temporaryType->setIndirectionsV(i.indirectionsV());
            temporaryType->decideUsagePattern();
            templateTypes << temporaryType;
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << "Ignoring template parameter " << typeName << " from "
                << info.toString() << ". The corresponding type was not found in the typesystem.";
        }
    }

    const AbstractMetaFunctionList &subclassFuncs = subclass->functions();
    const AbstractMetaFunctionList &templateClassFunctions = templateClass->functions();
    for (const AbstractMetaFunction *function : templateClassFunctions) {
        // If the function is modified or the instantiation has an equally named
        // function we have shadowing, so we need to skip it.
        if (function->isModifiedRemoved(TypeSystem::All)
            || AbstractMetaFunction::find(subclassFuncs, function->name()) != nullptr) {
            continue;
        }

        QScopedPointer<AbstractMetaFunction> f(function->copy());
        f->setArguments(AbstractMetaArgumentList());

        if (function->type()) { // Non-void
            AbstractMetaType *returnType = inheritTemplateType(templateTypes, function->type());
            if (!returnType)
                continue;
            f->replaceType(returnType);
        }

        const AbstractMetaArgumentList &arguments = function->arguments();
        for (AbstractMetaArgument *argument : arguments) {
            AbstractMetaType *argType = inheritTemplateType(templateTypes, argument->type());
            if (!argType)
                break;
            AbstractMetaArgument *arg = argument->copy();
            arg->replaceType(argType);
            f->addArgument(arg);
        }

        if (f->arguments().size() < function->arguments().size())
            continue;

        // There is no base class in the target language to inherit from here, so
        // the template instantiation is the class that implements the function.
        f->setImplementingClass(subclass);

        // We also set it as the declaring class, since the superclass is
        // supposed to disappear. This allows us to make certain function modifications
        // on the inherited functions.
        f->setDeclaringClass(subclass);

        if (f->isConstructor()) {
            if (!subclass->isTypeDef())
                continue;
            f->setName(subclass->name());
            f->setOriginalName(subclass->name());
        }

        ComplexTypeEntry* te = subclass->typeEntry();
        FunctionModificationList mods = function->modifications(templateClass);
        for (int i = 0; i < mods.size(); ++i) {
            FunctionModification mod = mods.at(i);
            mod.setSignature(f->minimalSignature());

            // If we ever need it... Below is the code to do
            // substitution of the template instantation type inside
            // injected code..
#if 0
            if (mod.modifiers & Modification::CodeInjection) {
                for (int j = 0; j < template_types.size(); ++j) {
                    CodeSnip &snip = mod.snips.last();
                    QString code = snip.code();
                    code.replace(QString::fromLatin1("$$QT_TEMPLATE_%1$$").arg(j),
                                 template_types.at(j)->typeEntry()->qualifiedCppName());
                    snip.codeList.clear();
                    snip.addCode(code);
                }
            }
#endif
            te->addFunctionModification(mod);
        }

        subclass->addFunction(f.take());
    }

    const AbstractMetaFieldList &subClassFields = subclass->fields();
    const AbstractMetaFieldList &templateClassFields = templateClass->fields();
    for (const AbstractMetaField *field : templateClassFields) {
        // If the field is modified or the instantiation has a field named
        // the same as an existing field we have shadowing, so we need to skip it.
        if (field->isModifiedRemoved(TypeSystem::All)
            || field->attributes().testFlag(AbstractMetaAttributes::Static)
            || AbstractMetaField::find(subClassFields, field->name()) != nullptr) {
            continue;
        }

        QScopedPointer<AbstractMetaField> f(field->copy());
        f->setEnclosingClass(subclass);
        AbstractMetaType *fieldType = inheritTemplateType(templateTypes, field->type());
        if (!fieldType)
            continue;
        f->replaceType(fieldType);
        subclass->addField(f.take());
    }

    subclass->setTemplateBaseClass(templateClass);
    subclass->setTemplateBaseClassInstantiations(templateTypes);
    subclass->setInterfaces(templateClass->interfaces());
    subclass->setBaseClass(templateClass->baseClass());

    return true;
}

void AbstractMetaBuilderPrivate::parseQ_Property(AbstractMetaClass *metaClass,
                                                 const QStringList &declarations)
{
    for (int i = 0; i < declarations.size(); ++i) {
        const QString &p = declarations.at(i);

        QStringList l = p.split(QLatin1Char(' '));


        QStringList qualifiedScopeName = currentScope()->qualifiedName();
        AbstractMetaType* type = 0;
        QString scope;
        for (int j = qualifiedScopeName.size(); j >= 0; --j) {
            scope = j > 0 ? QStringList(qualifiedScopeName.mid(0, j)).join(colonColon()) + colonColon() : QString();
            TypeInfo info;
            info.setQualifiedName((scope + l.at(0)).split(colonColon()));

            type = translateType(info);
            if (type)
                break;
        }

        if (!type) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Unable to decide type of property: '%1' in class '%2'")
                                  .arg(l.at(0), metaClass->name());
            continue;
        }

        QPropertySpec* spec = new QPropertySpec(type->typeEntry());
        spec->setName(l.at(1));
        spec->setIndex(i);

        for (int pos = 2; pos + 1 < l.size(); pos += 2) {
            if (l.at(pos) == QLatin1String("READ"))
                spec->setRead(l.at(pos + 1));
            else if (l.at(pos) == QLatin1String("WRITE"))
                spec->setWrite(l.at(pos + 1));
            else if (l.at(pos) == QLatin1String("DESIGNABLE"))
                spec->setDesignable(l.at(pos + 1));
            else if (l.at(pos) == QLatin1String("RESET"))
                spec->setReset(l.at(pos + 1));
        }

        metaClass->addPropertySpec(spec);
        delete type;
    }
}

static AbstractMetaFunction* findCopyCtor(AbstractMetaClass* cls)
{

    const auto &functions = cls->functions();

    for (AbstractMetaFunction *f : qAsConst(functions)) {
        const AbstractMetaFunction::FunctionType t = f->functionType();
        if (t == AbstractMetaFunction::CopyConstructorFunction || t == AbstractMetaFunction::AssignmentOperatorFunction)
            return f;
    }
    return 0;
}

void AbstractMetaBuilderPrivate::setupClonable(AbstractMetaClass *cls)
{
    bool result = true;

    // find copy ctor for the current class
    AbstractMetaFunction* copyCtor = findCopyCtor(cls);
    if (copyCtor) { // if exists a copy ctor in this class
        result = copyCtor->isPublic();
    } else { // else... lets find one in the parent class
        QQueue<AbstractMetaClass*> baseClasses;
        if (cls->baseClass())
            baseClasses.enqueue(cls->baseClass());
        baseClasses << cls->interfaces().toList();

        while (!baseClasses.isEmpty()) {
            AbstractMetaClass* currentClass = baseClasses.dequeue();
            baseClasses << currentClass->interfaces().toList();
            if (currentClass->baseClass())
                baseClasses.enqueue(currentClass->baseClass());

            copyCtor = findCopyCtor(currentClass);
            if (copyCtor) {
                result = copyCtor->isPublic();
                break;
            }
        }
    }
    cls->setHasCloneOperator(result);
}

void AbstractMetaBuilderPrivate::setupExternalConversion(AbstractMetaClass *cls)
{
    const AbstractMetaFunctionList &convOps = cls->operatorOverloads(AbstractMetaClass::ConversionOp);
    for (AbstractMetaFunction *func : convOps) {
        if (func->isModifiedRemoved())
            continue;
        AbstractMetaClass *metaClass = AbstractMetaClass::findClass(m_metaClasses, func->type()->typeEntry());
        if (!metaClass)
            continue;
        metaClass->addExternalConversionOperator(func);
    }
    const AbstractMetaClassList &innerClasses = cls->innerClasses();
    for (AbstractMetaClass *innerClass : innerClasses)
        setupExternalConversion(innerClass);
}

static void writeRejectLogFile(const QString &name,
                                  const QMap<QString, AbstractMetaBuilder::RejectReason> &rejects)
{
    QFile f(name);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("failed to write log file: '%1'")
                              .arg(QDir::toNativeSeparators(f.fileName()));
        return;
    }

    QTextStream s(&f);


    for (int reason = 0; reason < AbstractMetaBuilder::NoReason; ++reason) {
        s << QString(72, QLatin1Char('*')) << endl;
        switch (reason) {
        case AbstractMetaBuilder::NotInTypeSystem:
            s << "Not in type system";
            break;
        case AbstractMetaBuilder::GenerationDisabled:
            s << "Generation disabled by type system";
            break;
        case AbstractMetaBuilder::RedefinedToNotClass:
            s << "Type redefined to not be a class";
            break;

        case AbstractMetaBuilder::UnmatchedReturnType:
            s << "Unmatched return type";
            break;

        case AbstractMetaBuilder::UnmatchedArgumentType:
            s << "Unmatched argument type";
            break;

        case AbstractMetaBuilder::ApiIncompatible:
            s << "Incompatible API";
            break;

        default:
            s << "unknown reason";
            break;
        }

        s << endl;

        for (QMap<QString, AbstractMetaBuilder::RejectReason>::const_iterator it = rejects.constBegin();
             it != rejects.constEnd(); ++it) {
            if (it.value() != reason)
                continue;
            s << " - " << it.key() << endl;
        }

        s << QString(72, QLatin1Char('*')) << endl << endl;
    }

}

void AbstractMetaBuilderPrivate::dumpLog() const
{
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_classes.log"), m_rejectedClasses);
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_enums.log"), m_rejectedEnums);
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_functions.log"), m_rejectedFunctions);
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_fields.log"), m_rejectedFields);
}

AbstractMetaClassList AbstractMetaBuilderPrivate::classesTopologicalSorted(const AbstractMetaClass *cppClass,
                                                                           const Dependencies &additionalDependencies) const
{
    QLinkedList<int> unmappedResult;
    QHash<QString, int> map;
    QHash<int, AbstractMetaClass*> reverseMap;

    const AbstractMetaClassList& classList = cppClass ? cppClass->innerClasses() : m_metaClasses;

    int i = 0;
    for (AbstractMetaClass *clazz : classList) {
        if (map.contains(clazz->qualifiedCppName()))
            continue;
        map[clazz->qualifiedCppName()] = i;
        reverseMap[i] = clazz;
        i++;
    }

    Graph graph(map.count());

    for (const Dependency &dep : additionalDependencies) {
        const int parentIndex = map.value(dep.parent, -1);
        const int childIndex = map.value(dep.child, -1);
        if (parentIndex >= 0 && childIndex >= 0) {
            graph.addEdge(parentIndex, childIndex);
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << "AbstractMetaBuilder::classesTopologicalSorted(): Invalid additional dependency: "
                << dep.child << " -> " << dep.parent << '.';
        }
    }

    // TODO choose a better name to these regexs
    static const QRegularExpression regex1(QStringLiteral("\\(.*\\)"));
    Q_ASSERT(regex1.isValid());
    static const QRegularExpression regex2(QStringLiteral("::.*"));
    Q_ASSERT(regex2.isValid());
    for (AbstractMetaClass *clazz : classList) {
        if (clazz->enclosingClass() && map.contains(clazz->enclosingClass()->qualifiedCppName()))
            graph.addEdge(map[clazz->enclosingClass()->qualifiedCppName()], map[clazz->qualifiedCppName()]);

        const AbstractMetaClassList &bases = getBaseClasses(clazz);
        for (AbstractMetaClass *baseClass : bases) {
            // Fix polymorphic expression
            if (clazz->baseClass() == baseClass)
                clazz->setBaseClass(baseClass);

            if (map.contains(baseClass->qualifiedCppName()))
                graph.addEdge(map[baseClass->qualifiedCppName()], map[clazz->qualifiedCppName()]);
        }

        const AbstractMetaFunctionList &functions = clazz->functions();
        for (AbstractMetaFunction *func : functions) {
            const AbstractMetaArgumentList &arguments = func->arguments();
            for (AbstractMetaArgument *arg : arguments) {
                // check methods with default args
                QString defaultExpression = arg->originalDefaultValueExpression();
                if (!defaultExpression.isEmpty()) {
                    if (defaultExpression == QLatin1String("0") && arg->type()->isValue())
                        defaultExpression = arg->type()->name();

                    defaultExpression.remove(regex1);
                    defaultExpression.remove(regex2);
                }
                if (!defaultExpression.isEmpty()) {
                    QString exprClassName = clazz->qualifiedCppName() + colonColon() + defaultExpression;
                    if (!map.contains(exprClassName)) {
                        bool found = false;
                        for (AbstractMetaClass *baseClass : bases) {
                            exprClassName = baseClass->qualifiedCppName() + colonColon() + defaultExpression;
                            if (map.contains(exprClassName)) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            if (map.contains(defaultExpression))
                                exprClassName = defaultExpression;
                            else
                                exprClassName.clear();
                        }
                    }
                    if (!exprClassName.isEmpty() && exprClassName != clazz->qualifiedCppName())
                        graph.addEdge(map[exprClassName], map[clazz->qualifiedCppName()]);
                }
            }
        }
    }

    AbstractMetaClassList result;
    unmappedResult = graph.topologicalSort();
    if (unmappedResult.isEmpty() && graph.nodeCount()) {
        QTemporaryFile tempFile;
        tempFile.setAutoRemove(false);
        tempFile.open();
        QHash<int, QString> hash;
        QHash<QString, int>::iterator it = map.begin();
        for (; it != map.end(); ++it)
            hash[it.value()] = it.key();
        graph.dumpDot(hash, tempFile.fileName());
        qCWarning(lcShiboken).noquote().nospace()
            << "Cyclic dependency found! Graph can be found at "
            << QDir::toNativeSeparators(tempFile.fileName());
    } else {
        for (int i : qAsConst(unmappedResult)) {
            Q_ASSERT(reverseMap.contains(i));
            if (!reverseMap[i]->isInterface())
                result << reverseMap[i];
        }
    }

    return result;
}

AbstractMetaClassList AbstractMetaBuilder::classesTopologicalSorted(const AbstractMetaClass *cppClass,
                                                                    const Dependencies &additionalDependencies) const
{
    return d->classesTopologicalSorted(cppClass, additionalDependencies);
}

AbstractMetaArgumentList AbstractMetaBuilderPrivate::reverseList(const AbstractMetaArgumentList &list)
{
    AbstractMetaArgumentList ret;

    int index = list.size();
    for (AbstractMetaArgument *arg : list) {
        arg->setArgumentIndex(index);
        ret.prepend(arg);
        index--;
    }

    return ret;
}

void AbstractMetaBuilder::setGlobalHeader(const QString& globalHeader)
{
    d->m_globalHeader = QFileInfo(globalHeader);
}

void AbstractMetaBuilderPrivate::setInclude(TypeEntry *te, const QString &fileName) const
{
    QFileInfo info(fileName);
    if (m_globalHeader.fileName() != info.fileName())
        te->setInclude(Include(Include::IncludePath, info.fileName()));
}

#ifndef QT_NO_DEBUG_STREAM
template <class Container>
static void debugFormatSequence(QDebug &d, const char *key, const Container& c,
                                const char *separator = ", ")
{
    typedef typename Container::const_iterator ConstIt;
    if (c.isEmpty())
        return;
    const ConstIt begin = c.begin();
    const ConstIt end = c.end();
    d << "\n  " << key << '[' << c.size() << "]=(";
    for (ConstIt it = begin; it != end; ++it) {
        if (it != begin)
            d << separator;
        d << *it;
    }
    d << ')';
}

void AbstractMetaBuilder::formatDebug(QDebug &debug) const
{
    debug << "m_globalHeader=" << d->m_globalHeader.absoluteFilePath();
    debugFormatSequence(debug, "globalEnums", d->m_globalEnums, "\n");
    debugFormatSequence(debug, "globalFunctions", d->m_globalFunctions, "\n");
    if (const int scopeCount = d->m_scopes.size()) {
        debug << "\n  scopes[" << scopeCount << "]=(";
        for (int i = 0; i < scopeCount; ++i) {
            if (i)
                debug << ", ";
            _CodeModelItem::formatKind(debug, d->m_scopes.at(i)->kind());
            debug << " \"" << d->m_scopes.at(i)->name() << '"';
        }
        debug << ')';
    }
    debugFormatSequence(debug, "classes", d->m_metaClasses, "\n");
    debugFormatSequence(debug, "templates", d->m_templates, "\n");
}

QDebug operator<<(QDebug d, const AbstractMetaBuilder &ab)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaBuilder(";
    ab.formatDebug(d);
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
