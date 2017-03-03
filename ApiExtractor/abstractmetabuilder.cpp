/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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
#include "reporthandler.h"
#include "typedatabase.h"

#include "parser/ast.h"
#include "parser/binder.h"
#include "parser/control.h"
#include "parser/default_visitor.h"
#include "parser/dumptree.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/tokens.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
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

AbstractMetaFunctionList AbstractMetaBuilder::globalFunctions() const
{
    return d->m_globalFunctions;
}

AbstractMetaEnumList AbstractMetaBuilder::globalEnums() const
{
    return d->m_globalEnums;
}

QSet<QString> AbstractMetaBuilder::qtMetaTypeDeclaredTypeNames() const
{
    return d->m_qmetatypeDeclaredTypenames;
}

void AbstractMetaBuilderPrivate::checkFunctionModifications()
{
    TypeDatabase *types = TypeDatabase::instance();
    SingleTypeEntryHash entryHash = types->entries();
    QList<TypeEntry*> entries = entryHash.values();

    foreach (TypeEntry* entry, entries) {
        if (!entry)
            continue;
        if (!entry->isComplex() || entry->codeGeneration() == TypeEntry::GenerateNothing)
            continue;

        ComplexTypeEntry* centry = static_cast<ComplexTypeEntry*>(entry);
        FunctionModificationList modifications = centry->functionModifications();

        foreach (const FunctionModification &modification, modifications) {
            QString signature = modification.signature;

            QString name = signature.trimmed();
            name.truncate(name.indexOf(QLatin1Char('(')));

            AbstractMetaClass* clazz = m_metaClasses.findClass(centry->qualifiedCppName());
            if (!clazz)
                continue;

            AbstractMetaFunctionList functions = clazz->functions();
            bool found = false;
            QStringList possibleSignatures;
            foreach (AbstractMetaFunction *function, functions) {
                if (function->minimalSignature() == signature && function->implementingClass() == clazz) {
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
                    << QStringLiteral("signature '%1' for function modification in '%2' not found. Possible candidates: %3")
                                      .arg(signature, clazz->qualifiedCppName(), possibleSignatures.join(QLatin1String(", ")));
            }
        }
    }
}

AbstractMetaClass *AbstractMetaBuilderPrivate::argumentToClass(ArgumentModelItem argument)
{
    AbstractMetaClass* returned = 0;
    bool ok = false;
    AbstractMetaType* type = translateType(argument->type(), &ok);
    if (ok && type && type->typeEntry() && type->typeEntry()->isComplex()) {
        const TypeEntry *entry = type->typeEntry();
        returned = m_metaClasses.findClass(entry->name());
    }
    delete type;
    return returned;
}

AbstractMetaClass *AbstractMetaBuilder::createMetaClass()
{
    return new AbstractMetaClass();
}

AbstractMetaEnum *AbstractMetaBuilder::createMetaEnum()
{
    return new AbstractMetaEnum();
}

AbstractMetaEnumValue *AbstractMetaBuilder::createMetaEnumValue()
{
    return new AbstractMetaEnumValue();
}

AbstractMetaField *AbstractMetaBuilder::createMetaField()
{
    return new AbstractMetaField();
}

AbstractMetaFunction *AbstractMetaBuilder::createMetaFunction()
{
    return new AbstractMetaFunction();
}

AbstractMetaArgument *AbstractMetaBuilder::createMetaArgument()
{
    return new AbstractMetaArgument();
}

AbstractMetaType *AbstractMetaBuilder::createMetaType()
{
    return new AbstractMetaType();
}

/**
 * Checks the argument of a hash function and flags the type if it is a complex type
 */
void AbstractMetaBuilderPrivate::registerHashFunction(FunctionModelItem function_item)
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

void AbstractMetaBuilderPrivate::registerToStringCapability(FunctionModelItem function_item)
{
    ArgumentList arguments = function_item->arguments();
    if (arguments.size() == 2) {
        if (arguments.at(0)->type().toString() == QLatin1String("QDebug")) {
            ArgumentModelItem arg = arguments.at(1);
            if (AbstractMetaClass *cls = argumentToClass(arg)) {
                if (arg->type().indirections() < 2)
                    cls->setToStringCapability(true);
            }
        }
    }
}

void AbstractMetaBuilderPrivate::traverseOperatorFunction(FunctionModelItem item)
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
        bool ok;
        AbstractMetaType* type = translateType(item->type(), &ok);
        const TypeEntry* retType = ok ? type->typeEntry() : 0;
        AbstractMetaClass* otherArgClass = argumentToClass(arguments.at(1));
        if (otherArgClass && retType
            && (retType->isValue() || retType->isObject())
            && retType != baseoperandClass->typeEntry()
            && retType == otherArgClass->typeEntry()) {
            baseoperandClass = m_metaClasses.findClass(retType);
            firstArgumentIsSelf = false;
        }
        delete type;
    }

    if (baseoperandClass) {
        AbstractMetaClass* oldCurrentClass = m_currentClass;
        m_currentClass = baseoperandClass;
        AbstractMetaFunction *metaFunction = traverseFunction(item);
        if (metaFunction && !metaFunction->isInvalid()) {
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
        } else if (metaFunction) {
            delete metaFunction;
        }

        m_currentClass = oldCurrentClass;
    }
}

void AbstractMetaBuilderPrivate::traverseStreamOperator(FunctionModelItem item)
{
    ArgumentList arguments = item->arguments();
    if (arguments.size() == 2 && item->accessPolicy() == CodeModel::Public) {
        AbstractMetaClass* streamClass = argumentToClass(arguments.at(0));
        AbstractMetaClass* streamedClass = argumentToClass(arguments.at(1));

        if (streamClass && streamedClass && (streamClass->isStream())) {
            AbstractMetaClass *oldCurrentClass = m_currentClass;
            m_currentClass = streamedClass;
            AbstractMetaFunction *streamFunction = traverseFunction(item);

            if (streamFunction && !streamFunction->isInvalid()) {
                QString name = item->name();
                streamFunction->setFunctionType(AbstractMetaFunction::GlobalScopeFunction);
                // Strip first argument, since that is the containing object
                AbstractMetaArgumentList arguments = streamFunction->arguments();
                if (!streamClass->typeEntry()->generateCode())
                    delete arguments.takeLast();
                else
                    delete arguments.takeFirst();

                streamFunction->setArguments(arguments);

                *streamFunction += AbstractMetaAttributes::Final;
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
            } else if (streamFunction) {
                delete streamFunction;
            }

        }
    }
}

void AbstractMetaBuilderPrivate::fixQObjectForScope(const FileModelItem &dom,
                                                    const TypeDatabase *types,
                                                    const NamespaceModelItem &scope)
{
    foreach (const ClassModelItem &item, scope->classes()) {
        QString qualifiedName = item->qualifiedName().join(colonColon());
        TypeEntry* entry = types->findType(qualifiedName);
        if (entry) {
            if (isQObject(dom, qualifiedName) && entry->isComplex())
                ((ComplexTypeEntry*) entry)->setQObject(true);
        }
    }

    const NamespaceList &namespaces = scope->namespaces();
    foreach (const NamespaceModelItem &n, namespaces) {
        if (scope != n)
            fixQObjectForScope(dom, types, n);
    }
}

void AbstractMetaBuilderPrivate::sortLists()
{
    foreach (AbstractMetaClass *cls, m_metaClasses)
        cls->sortFunctions();
}

FileModelItem AbstractMetaBuilderPrivate::buildDom(QIODevice *input)
{
    Q_ASSERT(input);

    if (!input->isOpen() && !input->open(QIODevice::ReadOnly))
        return FileModelItem();

    QByteArray contents = input->readAll();
    input->close();

    Control control;
    Parser p(&control);
    pool __pool;

    TranslationUnitAST* ast = p.parse(contents, contents.size(), &__pool);

    CodeModel model;
    Binder binder(&model, p.location());
    return binder.run(ast);
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
    foreach (const ClassModelItem &item, typeValues) {
        ReportHandler::progress(QLatin1String("Generating class model..."));
        AbstractMetaClass *cls = traverseClass(dom, item);
        if (!cls)
            continue;

        addAbstractMetaClass(cls);
    }

    // We need to know all global enums
    ReportHandler::setProgressReference(dom->enums());
    foreach (const EnumModelItem &item, dom->enums()) {
        ReportHandler::progress(QLatin1String("Generating enum model..."));
        AbstractMetaEnum *metaEnum = traverseEnum(item, 0, QSet<QString>());
        if (metaEnum) {
            if (metaEnum->typeEntry()->generateCode())
                m_globalEnums << metaEnum;
        }
    }

    const QSet<NamespaceModelItem> &namespaceTypeValues = dom->uniqueNamespaces();
    ReportHandler::setProgressReference(namespaceTypeValues);
    foreach (NamespaceModelItem item, namespaceTypeValues) {
        ReportHandler::progress(QLatin1String("Generating namespace model..."));
        AbstractMetaClass *metaClass = traverseNamespace(dom, item);
        if (metaClass)
            m_metaClasses << metaClass;
    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    TypeDefList typeDefs = dom->typeDefs();
    ReportHandler::setProgressReference(typeDefs);
    foreach (const TypeDefModelItem &typeDef, typeDefs) {
        ReportHandler::progress(QLatin1String("Resolving typedefs..."));
        AbstractMetaClass* cls = traverseTypeDef(dom, typeDef);
        addAbstractMetaClass(cls);
    }

    figureOutEnumValues();

    foreach (const ClassModelItem &item, typeValues)
        traverseClassMembers(item);

    foreach (const NamespaceModelItem &item, namespaceTypeValues)
        traverseNamespaceMembers(item);

    // Global functions
    foreach (const FunctionModelItem &func, dom->functions()) {
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
    foreach (AbstractMetaClass* cls, m_metaClasses) {
        ReportHandler::progress(QLatin1String("Fixing class inheritance..."));
        if (!cls->isInterface() && !cls->isNamespace())
            setupInheritance(cls);
    }

    ReportHandler::setProgressReference(m_metaClasses);
    foreach (AbstractMetaClass* cls, m_metaClasses) {
        ReportHandler::progress(QLatin1String("Detecting inconsistencies in class model..."));
        cls->fixFunctions();

        if (!cls->typeEntry()) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("class '%1' does not have an entry in the type system")
                                  .arg(cls->name());
        } else {
            bool couldAddDefaultCtors = !cls->isFinalInCpp() && !cls->isInterface() && !cls->isNamespace();
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
    TypeEntryHash allEntries = types->allEntries();
    ReportHandler::progress(QLatin1String("Detecting inconsistencies in typesystem..."));
    foreach (QList<TypeEntry*> entries, allEntries) {
        foreach (TypeEntry* entry, entries) {

            if (entry->isPrimitive())
                continue;

            if ((entry->isValue() || entry->isObject())
                && !entry->isString()
                && !entry->isChar()
                && !entry->isContainer()
                && !entry->isCustom()
                && !entry->isVariant()
                && !m_metaClasses.findClass(entry->qualifiedCppName())) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("type '%1' is specified in typesystem, but not defined. This could potentially lead to compilation errors.")
                                      .arg(entry->qualifiedCppName());
            } else if (entry->generateCode() && entry->type() == TypeEntry::FunctionType) {
                const FunctionTypeEntry* fte = static_cast<const FunctionTypeEntry*>(entry);
                foreach (const QString &signature, fte->signatures()) {
                    bool ok = false;
                    foreach (AbstractMetaFunction* func, m_globalFunctions) {
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
            } else if (entry->isEnum()) {
                const QString name = ((EnumTypeEntry*) entry)->targetLangQualifier();
                AbstractMetaClass* cls = m_metaClasses.findClass(name);

                bool enumFound = false;
                if (cls) {
                    enumFound = cls->findEnum(entry->targetLangName());
                } else { // Global enum
                    foreach (AbstractMetaEnum* metaEnum, m_enums) {
                        if (metaEnum->typeEntry() == entry) {
                            enumFound = true;
                            break;
                        }
                    }
                }
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
        FunctionList hashFunctions = dom->findFunctions(QLatin1String("qHash"));
        foreach (const FunctionModelItem &item, hashFunctions)
            registerHashFunction(item);
    }

    {
        FunctionList hashFunctions = dom->findFunctions(QLatin1String("operator<<"));
        foreach (const FunctionModelItem &item, hashFunctions)
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
        foreach (const FunctionModelItem &item, potentiallyBinaryOperators) {
            if (!item->arguments().isEmpty())
                binaryOperators.append(item);
        }
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator-")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator&")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator|")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator^")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator~")));
        binaryOperators.append(dom->findFunctions(QStringLiteral("operator>")));

        foreach (const FunctionModelItem &item, binaryOperators)
            traverseOperatorFunction(item);
    }

    {
        FunctionList streamOperators = dom->findFunctions(QLatin1String("operator<<"))
                                       + dom->findFunctions(QLatin1String("operator>>"));
        foreach (const FunctionModelItem &item, streamOperators)
            traverseStreamOperator(item);
    }

    figureOutDefaultEnumArguments();
    checkFunctionModifications();

    // sort all classes topologically
    m_metaClasses = classesTopologicalSorted();

    foreach (AbstractMetaClass* cls, m_metaClasses) {
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
    foreach (const AddedFunction &addedFunc, types->globalUserFunctions()) {
        AbstractMetaFunction* metaFunc = traverseFunction(addedFunc);
        metaFunc->setFunctionType(AbstractMetaFunction::NormalFunction);
        m_globalFunctions << metaFunc;
    }

    std::puts("");
}

bool AbstractMetaBuilder::build(QIODevice *input)
{
    FileModelItem dom = d->buildDom(input);
    const bool result = dom.data() != Q_NULLPTR;
    if (result)
        d->traverseDom(dom);
    return result;
}

void AbstractMetaBuilder::setLogDirectory(const QString& logDir)
{
    d->m_logDirectory = logDir;
    if (!d->m_logDirectory.endsWith(QDir::separator()))
       d->m_logDirectory.append(QDir::separator());
}

void AbstractMetaBuilderPrivate::addAbstractMetaClass(AbstractMetaClass *cls)
{
    if (!cls)
        return;

    cls->setOriginalAttributes(cls->attributes());
    if (cls->typeEntry()->isContainer()) {
        m_templates << cls;
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

    AbstractMetaClass* metaClass = q->createMetaClass();
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

    ClassList classes = namespaceItem->classes();
    foreach (const ClassModelItem &cls, classes) {
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
    foreach (const TypeDefModelItem &typeDef, typeDefs) {
        AbstractMetaClass *cls = traverseTypeDef(dom, typeDef);
        if (cls) {
            metaClass->addInnerClass(cls);
            cls->setEnclosingClass(metaClass);
            addAbstractMetaClass(cls);
        }
    }

    // Traverse namespaces recursively
    const QSet<NamespaceModelItem> &innerNamespaces = namespaceItem->uniqueNamespaces();
    foreach (const NamespaceModelItem &ni, innerNamespaces) {
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

struct Operator
{
    enum Type { Complement, Plus, ShiftRight, ShiftLeft, None };

    Operator() : type(None) {}

    int calculate(int x)
    {
        switch (type) {
        case Complement: return ~value;
        case Plus: return x + value;
        case ShiftRight: return x >> value;
        case ShiftLeft: return x << value;
        case None: return x;
        }
        return x;
    }

    Type type;
    int value;
};



Operator findOperator(QString* s)
{
    const char *names[] = {
        "~",
        "+",
        ">>",
        "<<"
    };

    for (int i = 0; i < Operator::None; ++i) {
        QString name = QLatin1String(names[i]);
        QString str = *s;
        int splitPoint = str.indexOf(name);
        if (splitPoint > -1) {
            bool ok;
            QString right = str.mid(splitPoint + name.length());
            Operator op;

            op.value = right.toInt(&ok);
            if (!ok && right.length() > 0 && right.at(right.length() - 1).toLower() == QLatin1Char('u'))
                op.value = right.left(right.length() - 1).toUInt(&ok, 0);

            if (ok) {
                op.type = Operator::Type(i);
                if (splitPoint > 0)
                    *s = str.left(splitPoint).trimmed();
                else
                    *s = QString();
                return op;
            }
        }
    }
    return Operator();
}

int AbstractMetaBuilderPrivate::figureOutEnumValue(const QString &stringValue,
                                                   int oldValuevalue,
                                                   AbstractMetaEnum *metaEnum,
                                                   AbstractMetaFunction *metaFunction)
{
    if (stringValue.isEmpty())
        return oldValuevalue;

    QStringList stringValues = stringValue.split(QLatin1Char('|'));

    int returnValue = 0;

    bool matched = false;

    for (int i = 0; i < stringValues.size(); ++i) {
        QString s = stringValues.at(i).trimmed();

        bool ok;
        int v;

        Operator op = findOperator(&s);

        if (s.length() > 0 && s.at(0) == QLatin1Char('0'))
            v = s.toUInt(&ok, 0);
        else if (s.length() > 0 && s.at(s.length() - 1).toLower() == QLatin1Char('u'))
            v = s.left(s.length() - 1).toUInt(&ok, 0);
        else
            v = s.toInt(&ok);

        if (ok || s.isEmpty()) {
            matched = true;
        } else if (m_enumValues.contains(s)) {
            v = m_enumValues[s]->value();
            matched = true;
        } else {
            if (metaEnum) {
                v = findOutValueFromString(s, matched);
                if (!matched) {
                    QString enclosingClass = QString(metaEnum->enclosingClass() ? metaEnum->enclosingClass()->name() + colonColon() : QString());
                    qCWarning(lcShiboken).noquote().nospace()
                        << "unhandled enum value: " << s << " in "
                        << enclosingClass << metaEnum->name() << " from header '"
                        << metaEnum->typeEntry()->include().name() << '\'';
                }
            } else {
                qCWarning(lcShiboken) << "unhandled enum value: Unknown enum";
            }
        }

        if (matched)
            returnValue |= op.calculate(v);
    }

    if (!matched) {
        QString warn = QStringLiteral("unmatched enum %1").arg(stringValue);

        if (metaFunction) {
            warn += QStringLiteral(" when parsing default value of '%1' in class '%2'")
                    .arg(metaFunction->name(), metaFunction->implementingClass()->name());
        }
        warn += QLatin1String(" from header '") + metaEnum->typeEntry()->include().name()
                + QLatin1Char('\'');

        qCWarning(lcShiboken).noquote().nospace() << warn;
        returnValue = oldValuevalue;
    }

    return returnValue;
}

void AbstractMetaBuilderPrivate::figureOutEnumValuesForClass(AbstractMetaClass *metaClass,
                                                             QSet<AbstractMetaClass *> *classes)
{
    AbstractMetaClass* base = metaClass->baseClass();

    if (base && !classes->contains(base))
        figureOutEnumValuesForClass(base, classes);

    if (classes->contains(metaClass))
        return;

    AbstractMetaEnumList enums = metaClass->enums();
    foreach (AbstractMetaEnum* e, enums) {
        if (!e) {
            qCWarning(lcShiboken).noquote().nospace() << "bad enum in class " << metaClass->name();
            continue;
        }
        AbstractMetaEnumValueList lst = e->values();
        int value = 0;
        for (int i = 0; i < lst.size(); ++i) {
            value = figureOutEnumValue(lst.at(i)->stringValue(), value, e);
            lst.at(i)->setValue(value);
            value++;
        }
    }

    *classes += metaClass;
}


void AbstractMetaBuilderPrivate::figureOutEnumValues()
{
    // Keep a set of classes that we already traversed. We use this to
    // enforce that we traverse base classes prior to subclasses.
    QSet<AbstractMetaClass*> classes;
    foreach (AbstractMetaClass *c, m_metaClasses)
        figureOutEnumValuesForClass(c, &classes);

    foreach (AbstractMetaEnum* metaEnum, m_globalEnums) {
        AbstractMetaEnumValueList enumValues = metaEnum->values();
        int value = 0;
        for (int i = 0; i < enumValues.size(); ++i) {
            value = figureOutEnumValue(enumValues.at(i)->stringValue(), value, metaEnum);
            enumValues.at(i)->setValue(value);
            value++;
        }
    }
}

void AbstractMetaBuilderPrivate::figureOutDefaultEnumArguments()
{
    foreach (AbstractMetaClass* metaClass, m_metaClasses) {
        foreach (AbstractMetaFunction* metaFunction, metaClass->functions()) {
            foreach (AbstractMetaArgument *arg, metaFunction->arguments()) {
                QString expr = arg->defaultValueExpression();
                if (expr.isEmpty())
                    continue;

                if (!metaFunction->replacedDefaultExpression(metaFunction->implementingClass(),
                                                             arg->argumentIndex() + 1).isEmpty()) {
                    continue;
                }

                arg->setDefaultValueExpression(expr);
            }
        }
    }
}


AbstractMetaEnum *AbstractMetaBuilderPrivate::traverseEnum(EnumModelItem enumItem,
                                                           AbstractMetaClass *enclosing,
                                                           const QSet<QString> &enumsDeclarations)
{
    QString qualifiedName = enumItem->qualifiedName().join(colonColon());

    TypeEntry* typeEntry = 0;
    if (enumItem->accessPolicy() == CodeModel::Private) {
        QStringList names = enumItem->qualifiedName();
        QString enumName = names.last();
        QString nspace;
        if (names.size() > 1)
            nspace = QStringList(names.mid(0, names.size() - 1)).join(colonColon());
        typeEntry = new EnumTypeEntry(nspace, enumName, 0);
        TypeDatabase::instance()->addType(typeEntry);
    } else if (!enumItem->isAnonymous()) {
        typeEntry = TypeDatabase::instance()->findType(qualifiedName);
    } else {
        QStringList tmpQualifiedName = enumItem->qualifiedName();
        foreach (const EnumeratorModelItem& enumValue, enumItem->enumerators()) {
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

    if (TypeDatabase::instance()->isEnumRejected(className, enumName)) {
        if (typeEntry)
            typeEntry->setCodeGeneration(TypeEntry::GenerateNothing);
        m_rejectedEnums.insert(qualifiedName, AbstractMetaBuilder::GenerationDisabled);
        return 0;
    }

    if (!typeEntry || !typeEntry->isEnum()) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("enum '%1' does not have a type entry or is not an enum")
                              .arg(qualifiedName);
        m_rejectedEnums.insert(qualifiedName, AbstractMetaBuilder::NotInTypeSystem);
        return 0;
    }

    AbstractMetaEnum *metaEnum = q->createMetaEnum();
    if (enumsDeclarations.contains(qualifiedName)
        || enumsDeclarations.contains(enumName)) {
        metaEnum->setHasQEnumsDeclaration(true);
    }

    metaEnum->setTypeEntry((EnumTypeEntry*) typeEntry);
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

    foreach (const EnumeratorModelItem &value, enumItem->enumerators()) {

        AbstractMetaEnumValue *metaEnumValue = q->createMetaEnumValue();
        metaEnumValue->setName(value->name());
        // Deciding the enum value...

        metaEnumValue->setStringValue(value->value());
        metaEnum->addEnumValue(metaEnumValue);

        if (ReportHandler::isDebug(ReportHandler::FullDebug)) {
            qCDebug(lcShiboken) << "   - " << metaEnumValue->name() << " = "
                << metaEnumValue->value() << " = " << metaEnumValue->value();
        }

        // Add into global register...
        if (enclosing)
            m_enumValues[enclosing->name() + colonColon() + metaEnumValue->name()] = metaEnumValue;
        else
            m_enumValues[metaEnumValue->name()] = metaEnumValue;
    }

    m_enums << metaEnum;

    if (!metaEnum->typeEntry()->include().isValid())
        setInclude(metaEnum->typeEntry(), enumItem->fileName());

    metaEnum->setOriginalAttributes(metaEnum->attributes());

    // Register all enum values on Type database
    foreach(EnumeratorModelItem e, enumItem->enumerators()) {
        QString name;
        if (enclosing) {
            name += enclosing->name();
            name += colonColon();
        }
        name += e->name();
        EnumValueTypeEntry* enumValue = new EnumValueTypeEntry(name, e->value(), static_cast<EnumTypeEntry*>(typeEntry), typeEntry->version());
        TypeDatabase::instance()->addType(enumValue);
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

    AbstractMetaClass *metaClass = q->createMetaClass();
    metaClass->setTypeDef(true);
    metaClass->setTypeEntry(type);
    metaClass->setBaseClassNames(QStringList() << typeDef->type().qualifiedName().join(colonColon()));
    *metaClass += AbstractMetaAttributes::Public;

    // Set the default include file name
    if (!type->include().isValid())
        setInclude(type, typeDef->fileName());

    fillAddedFunctions(metaClass);

    return metaClass;
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

    if (fullClassName == QLatin1String("QMetaTypeId")) {
        // QtScript: record which types have been declared
        int lpos = classItem->name().indexOf(QLatin1Char('<'));
        int rpos = classItem->name().lastIndexOf(QLatin1Char('>'));
        if ((lpos != -1) && (rpos != -1)) {
            QString declaredTypename = classItem->name().mid(lpos + 1, rpos - lpos - 1);
            m_qmetatypeDeclaredTypenames.insert(declaredTypename);
        }
    }

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

    AbstractMetaClass *metaClass = q->createMetaClass();
    metaClass->setTypeEntry(type);
    metaClass->setBaseClassNames(classItem->baseClasses());
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
    QList<TypeEntry *> template_args;
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
        foreach (const ClassModelItem &ci, innerClasses) {
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
    foreach (const TypeDefModelItem &typeDef, typeDefs) {
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
    foreach (const ClassModelItem& ci, innerClasses)
        traverseClassMembers(ci);
}

AbstractMetaClass* AbstractMetaBuilderPrivate::currentTraversedClass(ScopeModelItem item)
{
    QString className = stripTemplateArgs(item->name());
    QString fullClassName = className;

    // This is an inner class
    if (m_currentClass)
        fullClassName = stripTemplateArgs(m_currentClass->typeEntry()->qualifiedCppName()) + colonColon() + fullClassName;

    AbstractMetaClass* metaClass = m_metaClasses.findClass(fullClassName);
    if (!metaClass)
        metaClass = m_templates.findClass(fullClassName);
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
    const QSet<NamespaceModelItem> &innerNamespaces = item->uniqueNamespaces();
    foreach (const NamespaceModelItem &ni, innerNamespaces)
        traverseNamespaceMembers(ni);

    m_currentClass = oldCurrentClass;
}

static inline QString fieldSignatureWithType(VariableModelItem field)
{
    return field->name() + QStringLiteral(" -> ") + field->type().toString();
}

static inline QString qualifiedFieldSignatureWithType(const QString &className,
                                                      VariableModelItem field)
{
    return className + colonColon() + fieldSignatureWithType(field);
}

AbstractMetaField *AbstractMetaBuilderPrivate::traverseField(VariableModelItem field,
                                                             const AbstractMetaClass *cls)
{
    QString fieldName = field->name();
    QString className = m_currentClass->typeEntry()->qualifiedCppName();

    // Ignore friend decl.
    if (field->isFriend())
        return 0;

    if (field->accessPolicy() == CodeModel::Private)
        return 0;

    if (TypeDatabase::instance()->isFieldRejected(className, fieldName)) {
        m_rejectedFields.insert(qualifiedFieldSignatureWithType(className, field),
                                AbstractMetaBuilder::GenerationDisabled);
        return 0;
    }


    AbstractMetaField *metaField = q->createMetaField();
    metaField->setName(fieldName);
    metaField->setEnclosingClass(cls);

    bool ok;
    TypeInfo fieldType = field->type();
    AbstractMetaType *metaType = translateType(fieldType, &ok);

    if (!metaType || !ok) {
        const QString type = TypeInfo::resolveType(fieldType, currentScope()).qualifiedName().join(colonColon());
        qCWarning(lcShiboken).noquote().nospace()
             << QStringLiteral("skipping field '%1::%2' with unmatched type '%3'")
                               .arg(m_currentClass->name(), fieldName, type);
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

void AbstractMetaBuilderPrivate::traverseFields(ScopeModelItem scope_item,
                                                AbstractMetaClass *metaClass)
{
    foreach (const VariableModelItem &field, scope_item->variables()) {
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

    if (!metaFunction->isFinalInTargetLang()
        && metaFunction->isRemovedFrom(metaClass, TypeSystem::TargetLangCode)) {
        *metaFunction += AbstractMetaAttributes::FinalInCpp;
    }
}

void AbstractMetaBuilderPrivate::fixReturnTypeOfConversionOperator(AbstractMetaFunction *metaFunction)
{
    if (!metaFunction->isConversionOperator())
        return;

    TypeDatabase* types = TypeDatabase::instance();
    QString castTo = metaFunction->name().remove(QRegExp(QLatin1String("^operator "))).trimmed();

    if (castTo.endsWith(QLatin1Char('&')))
        castTo.chop(1);
    if (castTo.startsWith(QLatin1String("const ")))
        castTo.remove(0, 6);

    TypeEntry* retType = types->findType(castTo);
    if (!retType)
        return;

    AbstractMetaType* metaType = q->createMetaType();
    metaType->setTypeEntry(retType);
    metaFunction->replaceType(metaType);
}

static bool _compareAbstractMetaTypes(const AbstractMetaType* type, const AbstractMetaType* other)
{
    if (!type && !other)
        return true;
    if (!type || !other)
        return false;
    return type->typeEntry() == other->typeEntry()
            && type->isConstant() == other->isConstant()
            && type->referenceType() == other->referenceType()
            && type->indirections() == other->indirections();
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

// Fix the arguments of template classes that take the class itself, for example:
// "QList(const QList &)" to "QList(const QList<T> &)".
static bool _fixFunctionModelItemTypes(FunctionModelItem& function, const AbstractMetaClass* metaClass)
{
    const QList<TypeEntry *> &templateTypes = metaClass->templateArguments();
    if (templateTypes.isEmpty())
        return false;

    const QStringList classType = metaClass->typeEntry()->qualifiedCppName().split(colonColon());
    QStringList fixedClassType = classType;
    fixedClassType.last().append(QLatin1Char('<'));
    for (int i = 0, count = templateTypes.size(); i < count; ++i) {
        if (i)
            fixedClassType.last().append(QLatin1String(", "));
        fixedClassType.last().append(templateTypes.at(i)->qualifiedCppName());
    }
    fixedClassType.last().append(QLatin1String(" >"));

    bool templateTypeFixed = false;
    TypeInfo functionType = function->type();
    if (functionType.qualifiedName() == classType) {
        templateTypeFixed = true;
        functionType.setQualifiedName(fixedClassType);
        function->setType(functionType);
    }

    ArgumentList arguments = function->arguments();
    for (int i = 0; i < arguments.size(); ++i) {
        ArgumentModelItem arg = arguments.at(i);
        TypeInfo type = arg->type();
        if (type.qualifiedName() == classType) {
            type.setQualifiedName(fixedClassType);
            arg->setType(type);
            templateTypeFixed = true;
        }
    }
    return templateTypeFixed;
}

void AbstractMetaBuilderPrivate::traverseFunctions(ScopeModelItem scopeItem,
                                                   AbstractMetaClass *metaClass)
{
    foreach (FunctionModelItem function, scopeItem->functions()) {

        // This fixes method's arguments and return types that are templates
        // but the template variable wasn't declared in the C++ header.
        bool templateTypeFixed = _fixFunctionModelItemTypes(function, metaClass);

        AbstractMetaFunction* metaFunction = traverseFunction(function);

        if (!metaFunction)
            continue;

        if (templateTypeFixed) {
            foreach (AbstractMetaFunction* func, metaClass->queryFunctionsByName(metaFunction->name())) {
                if (_compareAbstractMetaFunctions(metaFunction, func)) {
                    delete metaFunction;
                    metaFunction = 0;
                    break;
                }
            }
            if (!metaFunction)
                continue;
        }

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
            if ((!metaFunction->type()) && (metaFunction->arguments().size() == 0)) {
                *metaFunction += AbstractMetaAttributes::PropertyResetter;
                metaFunction->setPropertySpec(reset);
            }
        }

        const bool isInvalidDestructor = metaFunction->isDestructor() && metaFunction->isPrivate();
        const bool isInvalidConstructor = metaFunction->isConstructor()
            && ((metaFunction->isPrivate() && metaFunction->functionType() == AbstractMetaFunction::ConstructorFunction)
                || metaFunction->isInvalid());
        if ((isInvalidDestructor || isInvalidConstructor)
            && !metaClass->hasNonPrivateConstructor()) {
            *metaClass += AbstractMetaAttributes::Final;
        } else if (metaFunction->isConstructor() && !metaFunction->isPrivate()) {
            *metaClass -= AbstractMetaAttributes::Final;
            metaClass->setHasNonPrivateConstructor(true);
        }

        // Classes with virtual destructors should always have a shell class
        // (since we aren't registering the destructors, we need this extra check)
        if (metaFunction->isDestructor() && !metaFunction->isFinal())
            metaClass->setForceShellClass(true);

        if (!metaFunction->isDestructor()
            && !metaFunction->isInvalid()
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
    foreach (const AddedFunction &addedFunc, metaClass->typeEntry()->addedFunctions())
        traverseFunction(addedFunc, metaClass);
}

void AbstractMetaBuilderPrivate::applyFunctionModifications(AbstractMetaFunction *func)
{
    FunctionModificationList mods = func->modifications(func->implementingClass());
    AbstractMetaFunction& funcRef = *func;
    foreach (const FunctionModification &mod, mods) {
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
    if (baseClasses.size() == 1 && baseClasses.first().contains(QLatin1Char('<'))) {
        TypeParser::Info info;
        ComplexTypeEntry* baseContainerType;
        AbstractMetaClass* templ = findTemplateClass(baseClasses.first(), metaClass, &info, &baseContainerType);
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
                              .arg(baseClasses.first(), metaClass->name());
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
        AbstractMetaClass* baseClass = m_metaClasses.findClass(baseClasses.at(primary));
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
            AbstractMetaClass* baseClass = m_metaClasses.findClass(baseClasses.at(i));
            if (!baseClass) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("class not found for setup inheritance '%1'").arg(baseClasses.at(i));
                return false;
            }

            setupInheritance(baseClass);

            QString interfaceName = baseClass->isInterface() ? InterfaceTypeEntry::interfaceName(baseClass->name()) : baseClass->name();
            AbstractMetaClass* iface = m_metaClasses.findClass(interfaceName);
            if (!iface) {
                qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("unknown interface for '%1': '%2'").arg(metaClass->name(), interfaceName);
                return false;
            }
            metaClass->addInterface(iface);

            AbstractMetaClassList interfaces = iface->interfaces();
            foreach (AbstractMetaClass* iface, interfaces)
                metaClass->addInterface(iface);
        }
    }

    return true;
}

void AbstractMetaBuilderPrivate::traverseEnums(ScopeModelItem scopeItem,
                                               AbstractMetaClass *metaClass,
                                               const QStringList &enumsDeclarations)
{
    EnumList enums = scopeItem->enums();
    foreach (const EnumModelItem &enumItem, enums) {
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
    AbstractMetaFunction *metaFunction = q->createMetaFunction();
    metaFunction->setConstant(addedFunc.isConstant());
    metaFunction->setName(addedFunc.name());
    metaFunction->setOriginalName(addedFunc.name());
    AbstractMetaClass::Attributes visibility =
        addedFunc.access() == AddedFunction::Public
        ? AbstractMetaAttributes::Public : AbstractMetaAttributes::Protected;
    metaFunction->setVisibility(visibility);
    metaFunction->setUserAdded(true);
    AbstractMetaAttributes::Attribute isStatic = addedFunc.isStatic() ? AbstractMetaFunction::Static : AbstractMetaFunction::None;
    metaFunction->setAttributes(metaFunction->attributes() | AbstractMetaAttributes::Final | isStatic);
    metaFunction->setType(translateType(addedFunc.version(), addedFunc.returnType()));


    QList<AddedFunction::TypeInfo> args = addedFunc.arguments();
    AbstractMetaArgumentList metaArguments;

    for (int i = 0; i < args.count(); ++i) {
        AddedFunction::TypeInfo& typeInfo = args[i];
        AbstractMetaArgument *metaArg = q->createMetaArgument();
        AbstractMetaType* type = translateType(addedFunc.version(), typeInfo);
        decideUsagePattern(type);
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
            QString expr = replacedExpression;
            if (!metaFunction->removedDefaultExpression(m_currentClass, i + 1)) {
                metaArg->setDefaultValueExpression(expr);
                metaArg->setOriginalDefaultValueExpression(expr);

                if (metaArg->type()->isEnum() || metaArg->type()->isFlags())
                    m_enumDefaultArguments << QPair<AbstractMetaArgument*, AbstractMetaFunction*>(metaArg, metaFunction);
            }
        }
    }

    metaFunction->setOriginalAttributes(metaFunction->attributes());
    fixArgumentNames(metaFunction);

    if (metaClass) {
        const AbstractMetaArgumentList fargs = metaFunction->arguments();
        if (metaClass->isNamespace())
            *metaFunction += AbstractMetaFunction::Static;
        if (metaFunction->name() == metaClass->name()) {
            metaFunction->setFunctionType(AbstractMetaFunction::ConstructorFunction);
            if (fargs.size() == 1) {
                const TypeEntry *te = fargs.first()->type()->typeEntry();
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

void AbstractMetaBuilderPrivate::fixArgumentNames(AbstractMetaFunction *func)
{
    if (func->arguments().isEmpty())
        return;
    foreach (const FunctionModification &mod, func->modifications(m_currentClass)) {
        foreach (const ArgumentModification &argMod, mod.argument_mods) {
            if (!argMod.renamed_to.isEmpty()) {
                AbstractMetaArgument* arg = func->arguments().at(argMod.index - 1);
                arg->setOriginalName(arg->name());
                arg->setName(argMod.renamed_to, false);
            }
        }
    }

    int i = 1;
    foreach (AbstractMetaArgument* arg, func->arguments()) {
        if (arg->name().isEmpty())
            arg->setName(QLatin1String("arg__") + QString::number(i), false);
        ++i;
    }
}

static QString functionSignature(FunctionModelItem functionItem)
{
    QStringList args;
    foreach (const ArgumentModelItem &arg, functionItem->arguments())
        args << arg->type().toString();
    return functionItem->name() + QLatin1Char('(') + args.join(QLatin1Char(',')) + QLatin1Char(')');
}

static inline QString functionSignatureWithReturnType(FunctionModelItem functionItem)
{
    return functionSignature(functionItem)
            + QStringLiteral(" -> ") + functionItem->type().toString();
}

static inline QString qualifiedFunctionSignatureWithType(const QString &className,
                                                  FunctionModelItem functionItem)
{
    return className + colonColon() + functionSignatureWithReturnType(functionItem);
}

AbstractMetaFunction *AbstractMetaBuilderPrivate::traverseFunction(FunctionModelItem functionItem)
{
    QString functionName = functionItem->name();
    QString className;
    QString rejectedFunctionSignature;
    if (m_currentClass)
        className = m_currentClass->typeEntry()->qualifiedCppName();

    if (TypeDatabase::instance()->isFunctionRejected(className, functionName)) {
        rejectedFunctionSignature = qualifiedFunctionSignatureWithType(className, functionItem);
        m_rejectedFunctions.insert(rejectedFunctionSignature, AbstractMetaBuilder::GenerationDisabled);
        return 0;
    }
    else if (TypeDatabase::instance()->isFunctionRejected(className,
                                                          functionSignature(functionItem))) {
        rejectedFunctionSignature = qualifiedFunctionSignatureWithType(className, functionItem);
        m_rejectedFunctions.insert(rejectedFunctionSignature, AbstractMetaBuilder::GenerationDisabled);
        return 0;
    }

    Q_ASSERT(functionItem->functionType() == CodeModel::Normal
             || functionItem->functionType() == CodeModel::Signal
             || functionItem->functionType() == CodeModel::Slot);

    if (functionItem->isFriend())
        return 0;

    AbstractMetaFunction *metaFunction = q->createMetaFunction();
    metaFunction->setConstant(functionItem->isConstant());

    if (ReportHandler::isDebug(ReportHandler::MediumDebug))
        qCDebug(lcShiboken).noquote().nospace() << " - " << functionName << "()";

    metaFunction->setName(functionName);
    metaFunction->setOriginalName(functionItem->name());

    if (functionItem->isAbstract())
        *metaFunction += AbstractMetaAttributes::Abstract;

    if (!metaFunction->isAbstract())
        *metaFunction += AbstractMetaAttributes::Native;

    if (!functionItem->isVirtual())
        *metaFunction += AbstractMetaAttributes::Final;

    if (functionItem->isInvokable())
        *metaFunction += AbstractMetaAttributes::Invokable;

    if (functionItem->isStatic()) {
        *metaFunction += AbstractMetaAttributes::Static;
        *metaFunction += AbstractMetaAttributes::Final;
    }

    // Access rights
    if (functionItem->accessPolicy() == CodeModel::Public)
        *metaFunction += AbstractMetaAttributes::Public;
    else if (functionItem->accessPolicy() == CodeModel::Private)
        *metaFunction += AbstractMetaAttributes::Private;
    else
        *metaFunction += AbstractMetaAttributes::Protected;


    QString strippedClassName = className;
    int cc_pos = strippedClassName.lastIndexOf(colonColon());
    if (cc_pos > 0)
        strippedClassName = strippedClassName.mid(cc_pos + 2);

    TypeInfo functionType = functionItem->type();
    if (functionName.startsWith(QLatin1Char('~'))) {
        metaFunction->setFunctionType(AbstractMetaFunction::DestructorFunction);
        metaFunction->setInvalid(true);
    } else if (stripTemplateArgs(functionName) == strippedClassName) {
        metaFunction->setFunctionType(AbstractMetaFunction::ConstructorFunction);
        // Check for Copy/Move down below
        metaFunction->setExplicit(functionItem->isExplicit());
        metaFunction->setName(m_currentClass->name());
    } else {
        bool ok;
        AbstractMetaType* type = translateType(functionType, &ok);

        if (!ok) {
            Q_ASSERT(type == 0);
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("skipping function '%1::%2', unmatched return type '%3'")
                                  .arg(className, functionItem->name(),
                                       functionItem->type().toString());
            rejectedFunctionSignature = qualifiedFunctionSignatureWithType(className, functionItem);
            m_rejectedFunctions.insert(rejectedFunctionSignature, AbstractMetaBuilder::UnmatchedReturnType);
            metaFunction->setInvalid(true);
            return metaFunction;
        }

        metaFunction->setType(type);

        if (functionItem->functionType() == CodeModel::Signal)
            metaFunction->setFunctionType(AbstractMetaFunction::SignalFunction);
        else if (functionItem->functionType() == CodeModel::Slot)
            metaFunction->setFunctionType(AbstractMetaFunction::SlotFunction);
    }

    ArgumentList arguments = functionItem->arguments();

    if (arguments.size() == 1) {
        ArgumentModelItem arg = arguments.at(0);
        TypeInfo type = arg->type();
        if (type.qualifiedName().first() == QLatin1String("void") && type.indirections() == 0)
            arguments.pop_front();
    }

    AbstractMetaArgumentList metaArguments;

    for (int i = 0; i < arguments.size(); ++i) {
        ArgumentModelItem arg = arguments.at(i);

        bool ok;
        AbstractMetaType* metaType = translateType(arg->type(), &ok);
        if (!ok) {
            Q_ASSERT(metaType == 0);
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("skipping function '%1::%2', unmatched parameter type '%3'")
                                  .arg(className, functionItem->name(), arg->type().toString());
            rejectedFunctionSignature = qualifiedFunctionSignatureWithType(className, functionItem);
            m_rejectedFunctions.insert(rejectedFunctionSignature, AbstractMetaBuilder::UnmatchedArgumentType);
            metaFunction->setInvalid(true);
            return metaFunction;
        }

        if (metaType == Q_NULLPTR) {
            qCWarning(lcShiboken).noquote().nospace()
                << QString::fromLatin1("skipping function '%1::%2', 'void' encountered at parameter "
                                       "position %3, but it can only be the the first and only "
                                       "parameter")
                                       .arg(className, functionItem->name()).arg(i);
            rejectedFunctionSignature = qualifiedFunctionSignatureWithType(className, functionItem);
            m_rejectedFunctions.insert(rejectedFunctionSignature, AbstractMetaBuilder::UnmatchedArgumentType);
            metaFunction->setInvalid(true);
            return metaFunction;
        }

        AbstractMetaArgument *metaArgument = q->createMetaArgument();

        metaArgument->setType(metaType);
        metaArgument->setName(arg->name());
        metaArgument->setArgumentIndex(i);
        metaArguments << metaArgument;
    }

    metaFunction->setArguments(metaArguments);

    // Find the correct default values
    for (int i = 0; i < arguments.size(); ++i) {
        ArgumentModelItem arg = arguments.at(i);
        AbstractMetaArgument* metaArg = metaArguments.at(i);

        //use relace-default-expression for set default value
        QString replacedExpression;
        if (m_currentClass) {
            replacedExpression = metaFunction->replacedDefaultExpression(m_currentClass, i + 1);
        } else {
            FunctionModificationList mods = TypeDatabase::instance()->functionModifications(metaFunction->minimalSignature());
            if (!mods.isEmpty()) {
                QList<ArgumentModification> argMods = mods.first().argument_mods;
                if (!argMods.isEmpty())
                    replacedExpression = argMods.first().replacedDefaultExpression;
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

            if (metaArg->type()->isEnum() || metaArg->type()->isFlags())
                m_enumDefaultArguments << QPair<AbstractMetaArgument *, AbstractMetaFunction *>(metaArg, metaFunction);

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

    fixArgumentNames(metaFunction);

    // Determine class special functions
    if (m_currentClass && metaFunction->arguments().size() == 1) {
        const AbstractMetaType *argType = metaFunction->arguments().first()->type();
        if (argType->typeEntry() == m_currentClass->typeEntry() && argType->indirections() == 0) {
            if (metaFunction->isConstructor()) {
                switch (argType->referenceType()) {
                case NoReference:
                    metaFunction->setFunctionType(AbstractMetaFunction::CopyConstructorFunction);
                    break;
                case LValueReference:
                    if (argType->isConstant())
                        metaFunction->setFunctionType(AbstractMetaFunction::CopyConstructorFunction);
                    break;
                case RValueReference:
                    metaFunction->setFunctionType(AbstractMetaFunction::MoveConstructorFunction);
                    break;
                }
            } else if (metaFunction->name() == QLatin1String("operator=")) {
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

AbstractMetaType *AbstractMetaBuilderPrivate::translateType(double vr,
                                                            const AddedFunction::TypeInfo &typeInfo)
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
        SingleTypeEntryHash entries = typeDb->entries();
        for (SingleTypeEntryHash::const_iterator it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
            // Let's try to find the type in different scopes.
            if (it.key().endsWith(colonColon() + typeName))
                candidates.append(it.key());
        }

        QString msg = QStringLiteral("Type '%1' wasn't found in the type database.\n").arg(typeName);

        if (candidates.isEmpty())
            qFatal(qPrintable(QString(msg + QLatin1String("Declare it in the type system using the proper <*-type> tag."))), NULL);

        msg += QLatin1String("Remember to inform the full qualified name for the type you want to use.\nCandidates are:\n");
        candidates.sort();
        foreach (const QString& candidate, candidates) {
            msg += QLatin1String("    ") + candidate + QLatin1Char('\n');
        }
        qFatal(qPrintable(msg), NULL);
    }

    AbstractMetaType *metaType = q->createMetaType();
    metaType->setTypeEntry(type);
    metaType->setIndirections(typeInfo.indirections);
    if (typeInfo.isReference)
        metaType->setReferenceType(LValueReference);
    metaType->setConstant(typeInfo.isConstant);
    if (isTemplate) {
        foreach (const QString& templateArg, templateArgs) {
            AbstractMetaType* metaArgType = translateType(vr, AddedFunction::TypeInfo::fromSignature(templateArg));
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
    while(!type && (context.size() > 0) ) {
        type = TypeDatabase::instance()->findType(context.join(colonColon()) + colonColon() + qualifiedName);
        context.removeLast();
    }
    return type;
}

AbstractMetaType *AbstractMetaBuilderPrivate::translateType(const TypeInfo &_typei,
                                                            bool *ok, bool resolveType,
                                                            bool resolveScope)
{
    Q_ASSERT(ok);
    *ok = true;

    // 1. Test the type info without resolving typedefs in case this is present in the
    //    type system
    TypeInfo typei;
    if (resolveType) {
        bool ok;
        AbstractMetaType* t = translateType(_typei, &ok, false, resolveScope);
        if (t && ok)
            return t;
        Q_ASSERT(t == 0);
    }

    if (!resolveType) {
        typei = _typei;
    } else {
        // Go through all parts of the current scope (including global namespace)
        // to resolve typedefs. The parser does not properly resolve typedefs in
        // the global scope when they are referenced from inside a namespace.
        // This is a work around to fix this bug since fixing it in resolveType
        // seemed non-trivial
        int i = m_scopes.size() - 1;
        while (i >= 0) {
            typei = TypeInfo::resolveType(_typei, m_scopes.at(i--));
            if (typei.qualifiedName().join(colonColon()) != _typei.qualifiedName().join(colonColon()))
                break;
        }

    }

    if (typei.isFunctionPointer()) {
        *ok = false;
        return 0;
    }

    QString errorMessage;
    TypeParser::Info typeInfo = TypeParser::parse(typei.toString(), &errorMessage);
    if (typeInfo.is_busted) {
        qWarning().noquote().nospace() << "Unable to translate type \"" << _typei.toString()
            << "\": " << errorMessage;
        *ok = false;
        return 0;
    }

    // 2. Handle pointers specified as arrays with unspecified size
    bool arrayOfUnspecifiedSize = false;
    if (typeInfo.arrays.size() > 0) {
        arrayOfUnspecifiedSize = true;
        for (int i = 0; i < typeInfo.arrays.size(); ++i)
            arrayOfUnspecifiedSize = arrayOfUnspecifiedSize && typeInfo.arrays.at(i).isEmpty();

        if (!arrayOfUnspecifiedSize) {
            TypeInfo newInfo;
            //newInfo.setArguments(typei.arguments());
            newInfo.setIndirections(typei.indirections());
            newInfo.setConstant(typei.isConstant());
            newInfo.setFunctionPointer(typei.isFunctionPointer());
            newInfo.setQualifiedName(typei.qualifiedName());
            newInfo.setReferenceType(typei.referenceType());
            newInfo.setVolatile(typei.isVolatile());

            AbstractMetaType* elementType = translateType(newInfo, ok);
            if (!(*ok))
                return 0;

            for (int i = typeInfo.arrays.size() - 1; i >= 0; --i) {
                QString s = typeInfo.arrays.at(i);
                bool _ok;
                int elems = findOutValueFromString(s, _ok);

                AbstractMetaType *arrayType = q->createMetaType();
                arrayType->setArrayElementCount(elems);
                arrayType->setArrayElementType(elementType);
                arrayType->setTypeEntry(new ArrayTypeEntry(elementType->typeEntry() , elementType->typeEntry()->version()));
                decideUsagePattern(arrayType);

                elementType = arrayType;
            }

            return elementType;
        }  else {
            typeInfo.indirections += typeInfo.arrays.size();
        }
    }

    QStringList qualifierList = typeInfo.qualified_name;
    if (qualifierList.isEmpty()) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("horribly broken type '%1'").arg(_typei.toString());
        *ok = false;
        return 0;
    }

    QString qualifiedName = qualifierList.join(colonColon());
    QString name = qualifierList.takeLast();

    // 3. Special case 'void' type
    if (name == QLatin1String("void") && !typeInfo.indirections)
        return 0;

    // 4. Special case QFlags (include instantiation in name)
    if (qualifiedName == QLatin1String("QFlags"))
        qualifiedName = typeInfo.toString();

    const TypeEntry *type = 0;
    // 5. Try to find the type

    // 5.1 - Try first using the current scope
    if (m_currentClass) {
        type = findTypeEntryUsingContext(m_currentClass, qualifiedName);

        // 5.1.1 - Try using the class parents' scopes
        if (!type && !m_currentClass->baseClassNames().isEmpty()) {
            foreach (const AbstractMetaClass* cls, getBaseClasses(m_currentClass)) {
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
    if (!type && m_currentClass) {
        QList<TypeEntry *> template_args = m_currentClass->templateArguments();
        foreach (TypeEntry *te, template_args) {
            if (te->name() == qualifiedName)
                type = te;
        }
    }

    // 9. Try finding the type by prefixing it with the current
    //    context and all baseclasses of the current context
    if (!type && !TypeDatabase::instance()->isClassRejected(qualifiedName) && m_currentClass && resolveScope) {
        QStringList contexts;
        contexts.append(m_currentClass->qualifiedCppName());
        contexts.append(currentScope()->qualifiedName().join(colonColon()));


        TypeInfo info = typei;
        bool subclassesDone = false;
        while (!contexts.isEmpty() && !type) {
            type = TypeDatabase::instance()->findType(contexts.at(0) + colonColon() + qualifiedName);
            contexts.pop_front();

            // 10. Last resort: Special cased prefix of Qt namespace since the meta object implicitly inherits this, so
            //     enum types from there may be addressed without any scope resolution in properties.
            if (!contexts.size() && !subclassesDone) {
                contexts << QLatin1String("Qt");
                subclassesDone = true;
            }
        }

    }

    if (!type) {
        *ok = false;
        return 0;
    }

    // Used to for diagnostics later...
    m_usedTypes << type;

    // These are only implicit and should not appear in code...
    Q_ASSERT(!type->isInterface());

    AbstractMetaType *metaType = q->createMetaType();
    metaType->setTypeEntry(type);
    metaType->setIndirections(typeInfo.indirections);
    metaType->setReferenceType(typeInfo.referenceType);
    metaType->setConstant(typeInfo.is_constant);
    metaType->setOriginalTypeDescription(_typei.toString());

    foreach (const TypeParser::Info &ta, typeInfo.template_instantiations) {
        TypeInfo info;
        info.setConstant(ta.is_constant);
        info.setReferenceType(ta.referenceType);
        info.setIndirections(ta.indirections);

        info.setFunctionPointer(false);
        info.setQualifiedName(ta.instantiationName().split(colonColon()));

        AbstractMetaType* targType = translateType(info, ok);
        if (!(*ok)) {
            delete metaType;
            return 0;
        }

        metaType->addInstantiation(targType, true);
    }

    // The usage pattern *must* be decided *after* the possible template
    // instantiations have been determined, or else the absence of
    // such instantiations will break the caching scheme of
    // AbstractMetaType::cppSignature().
    decideUsagePattern(metaType);

    return metaType;
}


int AbstractMetaBuilderPrivate::findOutValueFromString(const QString &stringValue, bool &ok)
{
    int value = stringValue.toInt(&ok);
    if (ok)
        return value;

    if (stringValue == QLatin1String("true") || stringValue == QLatin1String("false")) {
        ok = true;
        return (stringValue == QLatin1String("true"));
    }

    // This is a very lame way to handle expression evaluation,
    // but it is not critical and will do for the time being.
    static QRegExp variableNameRegExp(QLatin1String("^[a-zA-Z_][a-zA-Z0-9_]*$"));
    if (!variableNameRegExp.exactMatch(stringValue)) {
        ok = true;
        return 0;
    }

    AbstractMetaEnumValue* enumValue = m_metaClasses.findEnumValue(stringValue);
    if (enumValue) {
        ok = true;
        return enumValue->value();
    }

    foreach (AbstractMetaEnum* metaEnum, m_globalEnums) {
        foreach (AbstractMetaEnumValue* ev, metaEnum->values()) {
            if (ev->name() == stringValue) {
                ok = true;
                return ev->value();
            }
        }
    }

    ok = false;
    return 0;
}

void AbstractMetaBuilderPrivate::decideUsagePattern(AbstractMetaType *metaType)
{
    metaType->decideUsagePattern();
}

QString AbstractMetaBuilderPrivate::fixDefaultValue(ArgumentModelItem item,
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
                QRegExp typeRegEx(QLatin1String("[^<]*[<]([^:]*::).*"));
                typeRegEx.indexIn(type->minimalSignature());
                expr = typeRegEx.cap(1) + expr;
            }
        } else if (type->isContainer() && expr.contains(QLatin1Char('<'))) {
            QRegExp typeRegEx(QLatin1String("[^<]*<(.*)>"));
            typeRegEx.indexIn(type->minimalSignature());
            QRegExp defaultRegEx(QLatin1String("([^<]*<).*(>[^>]*)"));
            defaultRegEx.indexIn(expr);
            expr = defaultRegEx.cap(1) + typeRegEx.cap(1) + defaultRegEx.cap(2);
        } else {
            // Here the default value is supposed to be a constructor,
            // a class field, or a constructor receiving a class field
            QRegExp defaultRegEx(QLatin1String("([^\\(]*\\(|)([^\\)]*)(\\)|)"));
            defaultRegEx.indexIn(expr);

            QString defaultValueCtorName = defaultRegEx.cap(1);
            if (defaultValueCtorName.endsWith(QLatin1Char('(')))
                defaultValueCtorName.chop(1);

            // Fix the scope for constructor using the already
            // resolved argument type as a reference.
            // The following regular expression extracts any
            // use of namespaces/scopes from the type string.
            QRegExp typeRegEx(QLatin1String("^(?:const[\\s]+|)([\\w:]*::|)([A-Za-z_]\\w*)\\s*[&\\*]?$"));
            typeRegEx.indexIn(type->minimalSignature());

            QString typeNamespace = typeRegEx.cap(1);
            QString typeCtorName = typeRegEx.cap(2);
            if (!typeNamespace.isEmpty() && defaultValueCtorName == typeCtorName)
                expr.prepend(typeNamespace);

            // Fix scope if the parameter is a field of the current class
            if (implementingClass) {
                foreach (const AbstractMetaField* field, implementingClass->fields()) {
                    if (defaultRegEx.cap(2) == field->name()) {
                        expr = defaultRegEx.cap(1) + implementingClass->name()
                               + colonColon() + defaultRegEx.cap(2) + defaultRegEx.cap(3);
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

    bool isqobject = classItem && classItem->extendsClass(QLatin1String("QObject"));

    if (classItem && !isqobject) {
        QStringList baseClasses = classItem->baseClasses();
        for (int i = 0; i < baseClasses.count(); ++i) {

            isqobject = isQObject(dom, baseClasses.at(i));
            if (isqobject)
                break;
        }
    }

    return isqobject;
}


bool AbstractMetaBuilderPrivate::isEnum(const FileModelItem &dom, const QStringList& qualified_name)
{
    CodeModelItem item = dom->model()->findItem(qualified_name, dom);
    return item && item->kind() == _EnumModelItem::__node_kind;
}

AbstractMetaClass* AbstractMetaBuilderPrivate::findTemplateClass(const QString &name,
                                                                 const AbstractMetaClass *context,
                                                                 TypeParser::Info *info,
                                                                 ComplexTypeEntry **baseContainerType) const
{
    TypeParser::Info localInfo;
    if (!info)
        info = &localInfo;

    TypeDatabase* types = TypeDatabase::instance();

    QStringList scope = context->typeEntry()->qualifiedCppName().split(colonColon());
    QString errorMessage;
    scope.removeLast();
    for (int i = scope.size(); i >= 0; --i) {
        QString prefix = i > 0 ? QStringList(scope.mid(0, i)).join(colonColon()) + colonColon() : QString();
        QString completeName = prefix + name;
        const TypeParser::Info parsed = TypeParser::parse(completeName, &errorMessage);
        if (parsed.is_busted) {
            qWarning().noquote().nospace() << "Unable to parse type \"" << completeName
                << "\" while looking for template \"" << name << "\": " << errorMessage;
            continue;
        }
        *info = parsed;
        QString qualifiedName = info->qualified_name.join(colonColon());

        AbstractMetaClass* templ = 0;
        foreach (AbstractMetaClass *c, m_templates) {
            if (c->typeEntry()->name() == qualifiedName) {
                templ = c;
                break;
            }
        }

        if (!templ)
            templ = m_metaClasses.findClass(qualifiedName);

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
    foreach (const QString& parent, metaClass->baseClassNames()) {
        AbstractMetaClass* cls = 0;
        if (parent.contains(QLatin1Char('<')))
            cls = findTemplateClass(parent, metaClass);
        else
            cls = m_metaClasses.findClass(parent);

        if (cls)
            baseClasses << cls;
    }
    return baseClasses;
}

bool AbstractMetaBuilderPrivate::ancestorHasPrivateCopyConstructor(const AbstractMetaClass *metaClass) const
{
    if (metaClass->hasPrivateCopyConstructor())
        return true;
    foreach (const AbstractMetaClass* cls, getBaseClasses(metaClass)) {
        if (ancestorHasPrivateCopyConstructor(cls))
            return true;
    }
    return false;
}

AbstractMetaType* AbstractMetaBuilderPrivate::inheritTemplateType(const QList<AbstractMetaType *> &templateTypes,
                                                                  const AbstractMetaType *metaType,
                                                                  bool *ok)
{
    if (ok)
        *ok = true;
    if (!metaType || (!metaType->typeEntry()->isTemplateArgument() && !metaType->hasInstantiations()))
        return metaType ? metaType->copy() : 0;

    AbstractMetaType *returned = metaType->copy();
    returned->setOriginalTemplateType(metaType->copy());

    if (returned->typeEntry()->isTemplateArgument()) {
        const TemplateArgumentEntry* tae = static_cast<const TemplateArgumentEntry*>(returned->typeEntry());

        // If the template is intantiated with void we special case this as rejecting the functions that use this
        // parameter from the instantiation.
        if (templateTypes.size() <= tae->ordinal() || templateTypes.at(tae->ordinal())->typeEntry()->name() == QLatin1String("void")) {
            if (ok)
                *ok = false;
            return 0;
        }

        AbstractMetaType* t = returned->copy();
        t->setTypeEntry(templateTypes.at(tae->ordinal())->typeEntry());
        t->setIndirections(templateTypes.at(tae->ordinal())->indirections() + t->indirections() ? 1 : 0);
        decideUsagePattern(t);

        delete returned;
        returned = inheritTemplateType(templateTypes, t, ok);
        if (ok && !(*ok))
            return 0;
    }

    if (returned->hasInstantiations()) {
        AbstractMetaTypeList instantiations = returned->instantiations();
        for (int i = 0; i < instantiations.count(); ++i) {
            AbstractMetaType *type = instantiations[i];
            instantiations[i] = inheritTemplateType(templateTypes, type, ok);
            if (ok && !(*ok))
                return 0;
        }
        returned->setInstantiations(instantiations, true);
    }

    return returned;
}

bool AbstractMetaBuilderPrivate::inheritTemplate(AbstractMetaClass *subclass,
                                                 const AbstractMetaClass *templateClass,
                                                 const TypeParser::Info &info)
{
    QList<TypeParser::Info> targs = info.template_instantiations;
    QList<AbstractMetaType*> templateTypes;

    if (subclass->isTypeDef()) {
        subclass->setHasCloneOperator(templateClass->hasCloneOperator());
        subclass->setHasEqualsOperator(templateClass->hasEqualsOperator());
        subclass->setHasHashFunction(templateClass->hasHashFunction());
        subclass->setHasNonPrivateConstructor(templateClass->hasNonPrivateConstructor());
        subclass->setHasPrivateDestructor(templateClass->hasPrivateDestructor());
        subclass->setHasProtectedDestructor(templateClass->hasProtectedDestructor());
        subclass->setHasVirtualDestructor(templateClass->hasVirtualDestructor());
    }

    foreach (const TypeParser::Info &i, targs) {
        QString typeName = i.qualified_name.join(colonColon());
        QStringList possibleNames;
        possibleNames << subclass->qualifiedCppName() + colonColon() + typeName;
        possibleNames << templateClass->qualifiedCppName() + colonColon() + typeName;
        if (subclass->enclosingClass())
            possibleNames << subclass->enclosingClass()->qualifiedCppName() + colonColon() + typeName;
        possibleNames << typeName;

        TypeDatabase* typeDb = TypeDatabase::instance();
        TypeEntry* t = 0;
        QString templateParamName;
        foreach (const QString &possibleName, possibleNames) {
            t = typeDb->findType(possibleName);
            if (t) {
                QString templateParamName = possibleName;
                break;
            }
        }

        if (t) {
            AbstractMetaType *temporaryType = q->createMetaType();
            temporaryType->setTypeEntry(t);
            temporaryType->setConstant(i.is_constant);
            temporaryType->setReferenceType(i.referenceType);
            temporaryType->setIndirections(i.indirections);
            temporaryType->decideUsagePattern();
            templateTypes << temporaryType;
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << "Ignoring template parameter " << templateParamName << " from "
                << info.instantiationName() << ", because I don't know what it is.";
        }
    }

    AbstractMetaFunctionList funcs = subclass->functions();
    foreach (const AbstractMetaFunction* function, templateClass->functions()) {
        if (function->isModifiedRemoved(TypeSystem::All))
            continue;

        AbstractMetaFunction *f = function->copy();
        f->setArguments(AbstractMetaArgumentList());

        bool ok = true;
        AbstractMetaType *ftype = function->type();
        f->replaceType(inheritTemplateType(templateTypes, ftype, &ok));
        if (!ok) {
            delete f;
            continue;
        }

        foreach (AbstractMetaArgument* argument, function->arguments()) {
            AbstractMetaType* atype = argument->type();

            AbstractMetaArgument *arg = argument->copy();
            arg->replaceType(inheritTemplateType(templateTypes, atype, &ok));
            if (!ok)
                break;
            f->addArgument(arg);
        }

        if (!ok) {
            delete f;
            continue;
        }

        // There is no base class in the target language to inherit from here, so
        // the template instantiation is the class that implements the function.
        f->setImplementingClass(subclass);

        // We also set it as the declaring class, since the superclass is
        // supposed to disappear. This allows us to make certain function modifications
        // on the inherited functions.
        f->setDeclaringClass(subclass);


        if (f->isConstructor() && subclass->isTypeDef()) {
            f->setName(subclass->name());
            f->setOriginalName(subclass->name());
        } else if (f->isConstructor()) {
            delete f;
            continue;
        }

        // if the instantiation has a function named the same as an existing
        // function we have shadowing so we need to skip it.
        bool found = false;
        for (int i = 0; i < funcs.size(); ++i) {
            if (funcs.at(i)->name() == f->name()) {
                found = true;
                continue;
            }
        }
        if (found) {
            delete f;
            continue;
        }

        ComplexTypeEntry* te = subclass->typeEntry();
        FunctionModificationList mods = function->modifications(templateClass);
        for (int i = 0; i < mods.size(); ++i) {
            FunctionModification mod = mods.at(i);
            mod.signature = f->minimalSignature();

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

        subclass->addFunction(f);
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
        QString p = declarations.at(i);

        QStringList l = p.split(QLatin1String(" "));


        QStringList qualifiedScopeName = currentScope()->qualifiedName();
        bool ok = false;
        AbstractMetaType* type = 0;
        QString scope;
        for (int j = qualifiedScopeName.size(); j >= 0; --j) {
            scope = j > 0 ? QStringList(qualifiedScopeName.mid(0, j)).join(colonColon()) + colonColon() : QString();
            TypeInfo info;
            info.setQualifiedName((scope + l.at(0)).split(colonColon()));

            type = translateType(info, &ok);
            if (type && ok)
                break;
        }

        if (!type || !ok) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("Unable to decide type of property: '%1' in class '%2'")
                                  .arg(l.at(0), metaClass->name());
            continue;
        }

        QString typeName = scope + l.at(0);

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
    AbstractMetaFunctionList functions = cls->queryFunctions(AbstractMetaClass::Invisible);
    functions <<  cls->queryFunctions(AbstractMetaClass::Visible);

    foreach (AbstractMetaFunction* f, functions) {
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
        baseClasses << cls->interfaces();

        while (!baseClasses.isEmpty()) {
            AbstractMetaClass* currentClass = baseClasses.dequeue();
            baseClasses << currentClass->interfaces();
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
    AbstractMetaFunctionList convOps = cls->operatorOverloads(AbstractMetaClass::ConversionOp);
    foreach (AbstractMetaFunction* func, convOps) {
        if (func->isModifiedRemoved())
            continue;
        AbstractMetaClass* metaClass = m_metaClasses.findClass(func->type()->typeEntry());
        if (!metaClass)
            continue;
        metaClass->addExternalConversionOperator(func);
    }
    foreach (AbstractMetaClass* innerClass, cls->innerClasses())
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

AbstractMetaClassList AbstractMetaBuilderPrivate::classesTopologicalSorted(const AbstractMetaClass *cppClass) const
{
    QLinkedList<int> unmappedResult;
    QHash<QString, int> map;
    QHash<int, AbstractMetaClass*> reverseMap;

    const AbstractMetaClassList& classList = cppClass ? cppClass->innerClasses() : m_metaClasses;

    int i = 0;
    foreach (AbstractMetaClass* clazz, classList) {
        if (map.contains(clazz->qualifiedCppName()))
            continue;
        map[clazz->qualifiedCppName()] = i;
        reverseMap[i] = clazz;
        i++;
    }

    Graph graph(map.count());

    // TODO choose a better name to these regexs
    QRegExp regex1(QLatin1String("\\(.*\\)"));
    QRegExp regex2(QLatin1String("::.*"));
    foreach (AbstractMetaClass* clazz, classList) {
        if (clazz->enclosingClass() && map.contains(clazz->enclosingClass()->qualifiedCppName()))
            graph.addEdge(map[clazz->enclosingClass()->qualifiedCppName()], map[clazz->qualifiedCppName()]);

        AbstractMetaClassList bases = getBaseClasses(clazz);
        foreach(AbstractMetaClass* baseClass, bases) {
            // Fix polymorphic expression
            if (clazz->baseClass() == baseClass)
                clazz->setBaseClass(baseClass);

            if (map.contains(baseClass->qualifiedCppName()))
                graph.addEdge(map[baseClass->qualifiedCppName()], map[clazz->qualifiedCppName()]);
        }

        foreach (AbstractMetaFunction* func, clazz->functions()) {
            foreach (AbstractMetaArgument* arg, func->arguments()) {
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
                        foreach(AbstractMetaClass* baseClass, bases) {
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
        foreach (int i, unmappedResult) {
            Q_ASSERT(reverseMap.contains(i));
            if (!reverseMap[i]->isInterface())
                result << reverseMap[i];
        }
    }

    return result;
}

AbstractMetaClassList AbstractMetaBuilder::classesTopologicalSorted(const AbstractMetaClass *cppClass) const
{
    return d->classesTopologicalSorted(cppClass);
}

AbstractMetaArgumentList AbstractMetaBuilderPrivate::reverseList(const AbstractMetaArgumentList &list)
{
    AbstractMetaArgumentList ret;

    int index = list.size();
    foreach (AbstractMetaArgument* arg, list) {
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
    debugFormatSequence(debug, "qtMetaTypeDeclaredTypeNames", d->m_qmetatypeDeclaredTypenames);
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
