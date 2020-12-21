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
#include "abstractmetaenum.h"
#include "abstractmetafield.h"
#include "abstractmetafunction.h"
#include "graph.h"
#include "messages.h"
#include "propertyspec.h"
#include "reporthandler.h"
#include "sourcelocation.h"
#include "typedatabase.h"
#include "typesystem.h"

#include "parser/codemodel.h"

#include <clangparser/clangbuilder.h>
#include <clangparser/clangutils.h>
#include <clangparser/compilersupport.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QQueue>
#include <QtCore/QRegularExpression>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>

#include <cstdio>
#include <algorithm>
#include <memory>

static inline QString colonColon() { return QStringLiteral("::"); }

static QString stripTemplateArgs(const QString &name)
{
    int pos = name.indexOf(QLatin1Char('<'));
    return pos < 0 ? name : name.left(pos);
}

AbstractMetaBuilderPrivate::AbstractMetaBuilderPrivate() :
    m_logDirectory(QLatin1String(".") + QDir::separator())
{
}

AbstractMetaBuilderPrivate::~AbstractMetaBuilderPrivate()
{
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

const AbstractMetaClassList &AbstractMetaBuilder::classes() const
{
    return d->m_metaClasses;
}

const AbstractMetaClassList &AbstractMetaBuilder::templates() const
{
    return d->m_templates;
}

const AbstractMetaClassList &AbstractMetaBuilder::smartPointers() const
{
    return d->m_smartPointers;
}

const AbstractMetaFunctionCList &AbstractMetaBuilder::globalFunctions() const
{
    return d->m_globalFunctions;
}

const AbstractMetaEnumList &AbstractMetaBuilder::globalEnums() const
{
    return d->m_globalEnums;
}

const QHash<const TypeEntry *, AbstractMetaEnum> &AbstractMetaBuilder::typeEntryToEnumsHash() const
{
    return d->m_enums;
}

void AbstractMetaBuilderPrivate::checkFunctionModifications()
{
    const auto &entries = TypeDatabase::instance()->entries();

    for (auto it = entries.cbegin(), end = entries.cend(); it != end; ++it) {
        const TypeEntry *entry = it.value();
        if (!entry)
            continue;
        if (!entry->isComplex() || !entry->generateCode())
            continue;

        auto centry = static_cast<const ComplexTypeEntry *>(entry);

        if (!centry->generateCode())
            continue;

        FunctionModificationList modifications = centry->functionModifications();

        for (const FunctionModification &modification : qAsConst(modifications)) {
            QString signature = modification.signature();

            QString name = signature.trimmed();
            name.truncate(name.indexOf(QLatin1Char('(')));

            AbstractMetaClass *clazz = AbstractMetaClass::findClass(m_metaClasses, centry);
            if (!clazz)
                continue;

            bool found = false;
            QStringList possibleSignatures;
            for (const auto &function : clazz->functions()) {
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
                    << msgNoFunctionForModification(clazz, signature,
                                                    modification.originalSignature(),
                                                    possibleSignatures, clazz->functions());
            }
        }
    }
}

AbstractMetaClass *AbstractMetaBuilderPrivate::argumentToClass(const ArgumentModelItem &argument,
                                                               const AbstractMetaClass *currentClass)
{
    AbstractMetaClass *returned = nullptr;
    auto type = translateType(argument->type(), currentClass);
    if (!type.has_value())
        return returned;
    const TypeEntry *entry = type->typeEntry();
    if (entry && entry->isComplex())
        returned = AbstractMetaClass::findClass(m_metaClasses, entry);
    return returned;
}

/**
 * Checks the argument of a hash function and flags the type if it is a complex type
 */
void AbstractMetaBuilderPrivate::registerHashFunction(const FunctionModelItem &function_item,
                                                      AbstractMetaClass *currentClass)
{
    ArgumentList arguments = function_item->arguments();
    if (arguments.size() == 1) {
        if (AbstractMetaClass *cls = argumentToClass(arguments.at(0), currentClass))
            cls->setHasHashFunction(true);
    }
}

void AbstractMetaBuilderPrivate::registerToStringCapabilityIn(const NamespaceModelItem &nsItem)
{
    const FunctionList &streamOps = nsItem->findFunctions(QLatin1String("operator<<"));
    for (const FunctionModelItem &item : streamOps)
        registerToStringCapability(item, nullptr);
    for (const NamespaceModelItem &ni : nsItem->namespaces())
        registerToStringCapabilityIn(ni);
}

/**
 * Check if a class has a debug stream operator that can be used as toString
 */

void AbstractMetaBuilderPrivate::registerToStringCapability(const FunctionModelItem &function_item,
                                                            AbstractMetaClass *currentClass)
{
    ArgumentList arguments = function_item->arguments();
    if (arguments.size() == 2) {
        if (arguments.at(0)->type().toString() == QLatin1String("QDebug")) {
            const ArgumentModelItem &arg = arguments.at(1);
            if (AbstractMetaClass *cls = argumentToClass(arg, currentClass)) {
                if (arg->type().indirections() < 2)
                    cls->setToStringCapability(true, int(arg->type().indirections()));
            }
        }
    }
}

void AbstractMetaBuilderPrivate::traverseOperatorFunction(const FunctionModelItem &item,
                                                          AbstractMetaClass *currentClass)
{
    if (item->accessPolicy() != CodeModel::Public)
        return;

    ArgumentList arguments = item->arguments();
    bool firstArgumentIsSelf = true;
    bool unaryOperator = false;

    auto baseoperandClass = argumentToClass(arguments.at(0), currentClass);

    if (arguments.size() == 1) {
        unaryOperator = true;
    } else if (!baseoperandClass
               || !baseoperandClass->typeEntry()->generateCode()) {
        baseoperandClass = argumentToClass(arguments.at(1), currentClass);
        firstArgumentIsSelf = false;
    } else {
        auto type = translateType(item->type(), currentClass);
        const TypeEntry *retType = type.has_value() ? type->typeEntry() : nullptr;
        AbstractMetaClass *otherArgClass = argumentToClass(arguments.at(1), currentClass);
        if (otherArgClass && retType
            && (retType->isValue() || retType->isObject())
            && retType != baseoperandClass->typeEntry()
            && retType == otherArgClass->typeEntry()) {
            baseoperandClass = AbstractMetaClass::findClass(m_metaClasses, retType);
            firstArgumentIsSelf = false;
        }
    }

    if (baseoperandClass) {
        AbstractMetaFunction *metaFunction = traverseFunction(item, baseoperandClass);
        if (metaFunction) {
            // Strip away first argument, since that is the containing object
            AbstractMetaArgumentList arguments = metaFunction->arguments();
            if (firstArgumentIsSelf || unaryOperator) {
                AbstractMetaArgument first = arguments.takeFirst();
                if (!unaryOperator && first.type().indirections())
                    metaFunction->setPointerOperator(true);
                metaFunction->setArguments(arguments);
            } else {
                // If the operator method is not unary and the first operator is
                // not of the same type of its owning class we suppose that it
                // must be an reverse operator (e.g. CLASS::operator(TYPE, CLASS)).
                // All operator overloads that operate over a class are already
                // being added as member functions of that class by the API Extractor.
                AbstractMetaArgument last = arguments.takeLast();
                if (last.type().indirections())
                    metaFunction->setPointerOperator(true);

                metaFunction->setArguments(arguments);
                metaFunction->setReverseOperator(true);
            }
            metaFunction->setVisibility(AbstractMetaFunction::Public);
            metaFunction->setOriginalAttributes(metaFunction->attributes());
            setupFunctionDefaults(metaFunction, baseoperandClass);
            baseoperandClass->addFunction(AbstractMetaFunctionCPtr(metaFunction));
            Q_ASSERT(!metaFunction->wasPrivate());
        } else {
            delete metaFunction;
        }
    }
}

bool AbstractMetaBuilderPrivate::traverseStreamOperator(const FunctionModelItem &item,
                                                        AbstractMetaClass *currentClass)
{
    ArgumentList itemArguments = item->arguments();
    if (itemArguments.size() != 2 || item->accessPolicy() != CodeModel::Public)
        return false;
    auto streamClass = argumentToClass(itemArguments.at(0), currentClass);
    if (streamClass == nullptr || !streamClass->isStream())
        return false;
   auto streamedClass = argumentToClass(itemArguments.at(1), currentClass);
   if (streamedClass == nullptr)
       return false;

   AbstractMetaFunction *streamFunction = traverseFunction(item, streamedClass);
   if (!streamFunction)
       return false;

    // Strip first argument, since that is the containing object
    AbstractMetaArgumentList arguments = streamFunction->arguments();
    if (!streamClass->typeEntry()->generateCode())
        arguments.takeLast();
    else
        arguments.takeFirst();

    streamFunction->setArguments(arguments);

    *streamFunction += AbstractMetaAttributes::FinalInTargetLang;
    *streamFunction += AbstractMetaAttributes::Public;
    streamFunction->setOriginalAttributes(streamFunction->attributes());

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
    funcClass->addFunction(AbstractMetaFunctionCPtr(streamFunction));
    if (funcClass == streamClass)
        funcClass->typeEntry()->addExtraInclude(streamedClass->typeEntry()->include());
    else
        funcClass->typeEntry()->addExtraInclude(streamClass->typeEntry()->include());
    return true;
}

static bool metaEnumLessThan(const AbstractMetaEnum &e1, const AbstractMetaEnum &e2)
{ return e1.fullName() < e2.fullName(); }

static bool metaClassLessThan(const AbstractMetaClass *c1, const AbstractMetaClass *c2)
{ return c1->fullName() < c2->fullName(); }

static bool metaFunctionLessThan(const AbstractMetaFunctionCPtr &f1, const AbstractMetaFunctionCPtr &f2)
{ return f1->name() < f2->name(); }

void AbstractMetaBuilderPrivate::sortLists()
{
    // Ensure indepedent classes are in alphabetical order,
    std::sort(m_metaClasses.begin(), m_metaClasses.end(), metaClassLessThan);
    // this is a temporary solution before new type revision implementation
    // We need move QMetaObject register before QObject.
    Dependencies additionalDependencies;
    if (auto qObjectClass = AbstractMetaClass::findClass(m_metaClasses, QStringLiteral("QObject"))) {
        if (auto qMetaObjectClass = AbstractMetaClass::findClass(m_metaClasses, QStringLiteral("QMetaObject"))) {
            Dependency dependency;
            dependency.parent = qMetaObjectClass;
            dependency.child = qObjectClass;
            additionalDependencies.append(dependency);
        }
    }
    m_metaClasses = classesTopologicalSorted(m_metaClasses, additionalDependencies);

    for (AbstractMetaClass *cls : qAsConst(m_metaClasses))
        cls->sortFunctions();

    // Ensure that indexes are in alphabetical order, roughly, except
    // for classes, which are in topological order
    std::sort(m_globalEnums.begin(), m_globalEnums.end(), metaEnumLessThan);
    std::sort(m_templates.begin(), m_templates.end(), metaClassLessThan);
    std::sort(m_smartPointers.begin(), m_smartPointers.end(), metaClassLessThan);
    std::sort(m_globalFunctions.begin(), m_globalFunctions.end(), metaFunctionLessThan);
}

FileModelItem AbstractMetaBuilderPrivate::buildDom(QByteArrayList arguments,
                                                   LanguageLevel level,
                                                   unsigned clangFlags)
{
    clang::Builder builder;
    builder.setSystemIncludes(TypeDatabase::instance()->systemIncludes());
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

    // Start the generation...
    const ClassList &typeValues = dom->classes();

    ReportHandler::startProgress("Generating class model ("
                                 + QByteArray::number(typeValues.size()) + ")...");
    for (const ClassModelItem &item : typeValues) {
        if (AbstractMetaClass *cls = traverseClass(dom, item, nullptr))
            addAbstractMetaClass(cls, item.data());
    }

    // We need to know all global enums
    const EnumList &enums = dom->enums();

    ReportHandler::startProgress("Generating enum model ("
                                 + QByteArray::number(enums.size()) + ")...");
    for (const EnumModelItem &item : enums) {
        auto metaEnum = traverseEnum(item, nullptr, QSet<QString>());
        if (metaEnum.has_value()) {
            if (metaEnum->typeEntry()->generateCode())
                m_globalEnums << metaEnum.value();
        }
    }

    const auto &namespaceTypeValues = dom->namespaces();
    ReportHandler::startProgress("Generating namespace model ("
                                 + QByteArray::number(namespaceTypeValues.size()) + ")...");
    for (const NamespaceModelItem &item : namespaceTypeValues)
        traverseNamespace(dom, item);

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    const TypeDefList typeDefs = dom->typeDefs();
    ReportHandler::startProgress("Resolving typedefs ("
                                 + QByteArray::number(typeDefs.size()) + ")...");
    for (const TypeDefModelItem &typeDef : typeDefs) {
        if (AbstractMetaClass *cls = traverseTypeDef(dom, typeDef, nullptr))
            addAbstractMetaClass(cls, typeDef.data());
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

        FunctionTypeEntry *funcEntry = types->findFunctionType(func->name());
        if (!funcEntry || !funcEntry->generateCode())
            continue;

        AbstractMetaFunction *metaFunc = traverseFunction(func, nullptr);
        if (!metaFunc)
            continue;

        AbstractMetaFunctionCPtr metaFuncPtr(metaFunc);
        if (!funcEntry->hasSignature(metaFunc->minimalSignature()))
            continue;

        metaFunc->setTypeEntry(funcEntry);
        applyFunctionModifications(metaFunc);

        setInclude(funcEntry, func->fileName());

        m_globalFunctions << metaFuncPtr;
    }

    ReportHandler::startProgress("Fixing class inheritance...");
    for (AbstractMetaClass *cls : qAsConst(m_metaClasses)) {
        if (cls->needsInheritanceSetup()) {
            setupInheritance(cls);
            if (cls->templateBaseClass())
                inheritTemplateFunctions(cls);
            if (!cls->hasVirtualDestructor() && cls->baseClass()
                && cls->baseClass()->hasVirtualDestructor())
                cls->setHasVirtualDestructor(true);
        }
    }

    ReportHandler::startProgress("Detecting inconsistencies in class model...");
    for (AbstractMetaClass *cls : qAsConst(m_metaClasses)) {
        cls->fixFunctions();

        const bool couldAddDefaultCtors = cls->isConstructible()
            && !cls->isNamespace()
            && (cls->attributes() & AbstractMetaAttributes::HasRejectedConstructor) == 0;
        if (couldAddDefaultCtors) {
            if (!cls->hasConstructors())
                cls->addDefaultConstructor();
            if (cls->typeEntry()->isValue() && !cls->isAbstract() && !cls->hasCopyConstructor())
                cls->addDefaultCopyConstructor(ancestorHasPrivateCopyConstructor(cls));
        }
    }
    const auto &allEntries = types->entries();

    ReportHandler::startProgress("Detecting inconsistencies in typesystem ("
                                 + QByteArray::number(allEntries.size()) + ")...");
    for (auto it = allEntries.cbegin(), end = allEntries.cend(); it != end; ++it) {
        TypeEntry *entry = it.value();
        if (!entry->isPrimitive()) {
            if ((entry->isValue() || entry->isObject())
                && !types->shouldDropTypeEntry(entry->qualifiedCppName())
                && !entry->isContainer()
                && !entry->isCustom()
                && entry->generateCode()
                && !AbstractMetaClass::findClass(m_metaClasses, entry)) {
                qCWarning(lcShiboken, "%s", qPrintable(msgTypeNotDefined(entry)));
            } else if (entry->generateCode() && entry->type() == TypeEntry::FunctionType) {
                auto fte = static_cast<const FunctionTypeEntry *>(entry);
                const QStringList &signatures = fte->signatures();
                for (const QString &signature : signatures) {
                    bool ok = false;
                    for (const auto &func : qAsConst(m_globalFunctions)) {
                        if (signature == func->minimalSignature()) {
                            ok = true;
                            break;
                        }
                    }
                    if (!ok) {
                        qCWarning(lcShiboken, "%s",
                                  qPrintable(msgGlobalFunctionNotDefined(fte, signature)));
                    }
                }
            } else if (entry->isEnum() && entry->generateCode()) {
                auto enumEntry = static_cast<const EnumTypeEntry *>(entry);
                const QString name = enumEntry->targetLangQualifier();
                AbstractMetaClass *cls = AbstractMetaClass::findClass(m_metaClasses,
                                                                      enumEntry->parent());

                const bool enumFound = cls
                    ? cls->findEnum(entry->targetLangEntryName()).has_value()
                    : m_enums.contains(entry);

                if (!enumFound) {
                    entry->setCodeGeneration(TypeEntry::GenerateNothing);
                    qCWarning(lcShiboken, "%s",
                              qPrintable(msgEnumNotDefined(enumEntry)));
                }

            }
        }
    }

    {
        const FunctionList &hashFunctions = dom->findFunctions(QLatin1String("qHash"));
        for (const FunctionModelItem &item : hashFunctions)
            registerHashFunction(item, nullptr);
    }

    registerToStringCapabilityIn(dom);

    for (const auto &func : dom->functions()) {
        switch (func->functionType()) {
        case CodeModel::ComparisonOperator:
        case CodeModel::ArithmeticOperator:
        case CodeModel::BitwiseOperator:
        case CodeModel::LogicalOperator:
            traverseOperatorFunction(func, nullptr);
            break;
        case CodeModel::ShiftOperator:
            if (!traverseStreamOperator(func, nullptr))
                traverseOperatorFunction(func, nullptr);
        default:
            break;
        }
    }

    ReportHandler::startProgress("Checking inconsistencies in function modifications...");

    checkFunctionModifications();

    ReportHandler::startProgress("Writing log files...");

    for (AbstractMetaClass *cls : qAsConst(m_metaClasses)) {
//         setupEquals(cls);
//         setupComparable(cls);
        setupClonable(cls);
        setupExternalConversion(cls);

        // sort all inner classes topologically
        if (!cls->typeEntry()->codeGeneration() || cls->innerClasses().size() < 2)
            continue;

        cls->setInnerClasses(classesTopologicalSorted(cls->innerClasses()));
    }

    dumpLog();

    sortLists();

    // Functions added to the module on the type system.
    const AddedFunctionList &globalUserFunctions = types->globalUserFunctions();
    for (const AddedFunctionPtr &addedFunc : globalUserFunctions) {
        if (!traverseAddedGlobalFunction(addedFunc)) {
            qFatal("Unable to traverse added global function \"%s\".",
                   qPrintable(addedFunc->name()));
        }
    }

    m_itemToClass.clear();
    m_typeSystemTypeDefs.clear();

    ReportHandler::endProgress();
}

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

    return true;
}

void AbstractMetaBuilder::setLogDirectory(const QString &logDir)
{
    d->m_logDirectory = logDir;
    if (!d->m_logDirectory.endsWith(QDir::separator()))
       d->m_logDirectory.append(QDir::separator());
}

void AbstractMetaBuilderPrivate::addAbstractMetaClass(AbstractMetaClass *cls,
                                                      const _CodeModelItem *item)
{
    cls->setOriginalAttributes(cls->attributes());
    m_itemToClass.insert(item, cls);
    if (cls->typeEntry()->isContainer()) {
        m_templates << cls;
    } else if (cls->typeEntry()->isSmartPointer()) {
        m_smartPointers << cls;
    } else {
        m_metaClasses << cls;
    }
}

AbstractMetaClass *AbstractMetaBuilderPrivate::traverseNamespace(const FileModelItem &dom,
                                                                 const NamespaceModelItem &namespaceItem)
{
    QString namespaceName = currentScope()->qualifiedName().join(colonColon());
    if (!namespaceName.isEmpty())
        namespaceName.append(colonColon());
    namespaceName.append(namespaceItem->name());

    if (TypeDatabase::instance()->isClassRejected(namespaceName)) {
        m_rejectedClasses.insert(namespaceName, AbstractMetaBuilder::GenerationDisabled);
        return nullptr;
    }

    auto type = TypeDatabase::instance()->findNamespaceType(namespaceName, namespaceItem->fileName());
    if (!type) {
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgNamespaceNoTypeEntry(namespaceItem, namespaceName)));
        return nullptr;
    }

    if (namespaceItem->type() == NamespaceType::Inline) {
        type->setInlineNamespace(true);
        TypeDatabase::instance()->addInlineNamespaceLookups(type);
    }

    // Continue populating namespace?
    AbstractMetaClass *metaClass = AbstractMetaClass::findClass(m_metaClasses, type);
    if (!metaClass) {
        metaClass = new AbstractMetaClass;
        metaClass->setTypeEntry(type);
        *metaClass += AbstractMetaAttributes::Public;
        addAbstractMetaClass(metaClass, namespaceItem.data());
        if (auto extendsType = type->extends()) {
            AbstractMetaClass *extended = AbstractMetaClass::findClass(m_metaClasses, extendsType);
            if (!extended) {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgNamespaceToBeExtendedNotFound(extendsType->name(), extendsType->targetLangPackage())));
                return nullptr;
            }
            metaClass->setExtendedNamespace(extended);
        }
    } else {
        m_itemToClass.insert(namespaceItem.data(), metaClass);
    }

    traverseEnums(namespaceItem, metaClass, namespaceItem->enumsDeclarations());

    pushScope(namespaceItem);

    const ClassList &classes = namespaceItem->classes();
    for (const ClassModelItem &cls : classes) {
        AbstractMetaClass *mjc = traverseClass(dom, cls, metaClass);
        if (mjc) {
            metaClass->addInnerClass(mjc);
            mjc->setEnclosingClass(metaClass);
            addAbstractMetaClass(mjc, cls.data());
        }
    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    const TypeDefList typeDefs = namespaceItem->typeDefs();
    for (const TypeDefModelItem &typeDef : typeDefs) {
        AbstractMetaClass *cls = traverseTypeDef(dom, typeDef, metaClass);
        if (cls) {
            metaClass->addInnerClass(cls);
            cls->setEnclosingClass(metaClass);
            addAbstractMetaClass(cls, typeDef.data());
        }
    }

    // Traverse namespaces recursively
    for (const NamespaceModelItem &ni : namespaceItem->namespaces()) {
        AbstractMetaClass *mjc = traverseNamespace(dom, ni);
        if (mjc) {
            metaClass->addInnerClass(mjc);
            mjc->setEnclosingClass(metaClass);
        }
    }

    popScope();

    if (!type->include().isValid())
        setInclude(type, namespaceItem->fileName());

    return metaClass;
}

std::optional<AbstractMetaEnum>
    AbstractMetaBuilderPrivate::traverseEnum(const EnumModelItem &enumItem,
                                             AbstractMetaClass *enclosing,
                                             const QSet<QString> &enumsDeclarations)
{
    QString qualifiedName = enumItem->qualifiedName().join(colonColon());

    TypeEntry *typeEntry = nullptr;
    const TypeEntry *enclosingTypeEntry = enclosing ? enclosing->typeEntry() : nullptr;
    if (enumItem->accessPolicy() == CodeModel::Private) {
        typeEntry = new EnumTypeEntry(enumItem->qualifiedName().constLast(),
                                      QVersionNumber(0, 0), enclosingTypeEntry);
        TypeDatabase::instance()->addType(typeEntry);
    } else if (enumItem->enumKind() != AnonymousEnum) {
        typeEntry = TypeDatabase::instance()->findType(qualifiedName);
    } else {
        QStringList tmpQualifiedName = enumItem->qualifiedName();
        const EnumeratorList &enums = enumItem->enumerators();
        for (const EnumeratorModelItem &enumValue : enums) {
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
    if (enclosingTypeEntry)
        className = enclosingTypeEntry->qualifiedCppName();

    QString rejectReason;
    if (TypeDatabase::instance()->isEnumRejected(className, enumName, &rejectReason)) {
        if (typeEntry)
            typeEntry->setCodeGeneration(TypeEntry::GenerateNothing);
        m_rejectedEnums.insert(qualifiedName + rejectReason, AbstractMetaBuilder::GenerationDisabled);
        return {};
    }

    const bool rejectionWarning = !enclosing || enclosing->typeEntry()->generateCode();

    if (!typeEntry) {
        if (rejectionWarning)
            qCWarning(lcShiboken, "%s", qPrintable(msgNoEnumTypeEntry(enumItem, className)));
        m_rejectedEnums.insert(qualifiedName, AbstractMetaBuilder::NotInTypeSystem);
        return {};
    }

    if (!typeEntry->isEnum()) {
        if (rejectionWarning) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgNoEnumTypeConflict(enumItem, className, typeEntry)));
        }
        m_rejectedEnums.insert(qualifiedName, AbstractMetaBuilder::NotInTypeSystem);
        return {};
    }

    AbstractMetaEnum metaEnum;
    metaEnum.setEnumKind(enumItem->enumKind());
    metaEnum.setSigned(enumItem->isSigned());
    if (enumsDeclarations.contains(qualifiedName)
        || enumsDeclarations.contains(enumName)) {
        metaEnum.setHasQEnumsDeclaration(true);
    }

    auto *enumTypeEntry = static_cast<EnumTypeEntry *>(typeEntry);
    metaEnum.setTypeEntry(enumTypeEntry);
    switch (enumItem->accessPolicy()) {
    case CodeModel::Public:
        metaEnum += AbstractMetaAttributes::Public;
        break;
    case CodeModel::Protected:
        metaEnum += AbstractMetaAttributes::Protected;
        break;
    case CodeModel::Private:
        metaEnum += AbstractMetaAttributes::Private;
        typeEntry->setCodeGeneration(TypeEntry::GenerateNothing);
        break;
    default:
        break;
    }

    const EnumeratorList &enums = enumItem->enumerators();
    for (const EnumeratorModelItem &value : enums) {

        AbstractMetaEnumValue metaEnumValue;
        metaEnumValue.setName(value->name());
        // Deciding the enum value...

        metaEnumValue.setStringValue(value->stringValue());
        metaEnumValue.setValue(value->value());
        metaEnum.addEnumValue(metaEnumValue);
    }

    if (!metaEnum.typeEntry()->include().isValid())
        setInclude(metaEnum.typeEntry(), enumItem->fileName());

    metaEnum.setOriginalAttributes(metaEnum.attributes());

    // Register all enum values on Type database
    const bool isScopedEnum = enumItem->enumKind() == EnumClass;
    const EnumeratorList &enumerators = enumItem->enumerators();
    for (const EnumeratorModelItem &e : enumerators) {
        auto enumValue =
            new EnumValueTypeEntry(e->name(), e->stringValue(),
                                   enumTypeEntry, isScopedEnum,
                                   enumTypeEntry->version());
        TypeDatabase::instance()->addType(enumValue);
        if (e->value().isNullValue())
            enumTypeEntry->setNullValue(enumValue);
    }

    metaEnum.setEnclosingClass(enclosing);
    m_enums.insert(typeEntry, metaEnum);

    return metaEnum;
}

AbstractMetaClass *AbstractMetaBuilderPrivate::traverseTypeDef(const FileModelItem &,
                                                               const TypeDefModelItem &typeDef,
                                                               AbstractMetaClass *currentClass)
{
    TypeDatabase *types = TypeDatabase::instance();
    QString className = stripTemplateArgs(typeDef->name());

    QString fullClassName = className;
    // we have an inner class
    if (currentClass) {
        fullClassName = stripTemplateArgs(currentClass->typeEntry()->qualifiedCppName())
                          + colonColon() + fullClassName;
    }

    // If this is the alias for a primitive type
    // we store the aliased type on the alias
    // TypeEntry
    PrimitiveTypeEntry *ptype = types->findPrimitiveType(className);
    if (ptype) {
        QString typeDefName = typeDef->type().qualifiedName()[0];
        ptype->setReferencedTypeEntry(types->findPrimitiveType(typeDefName));
        return nullptr;
    }


    // If we haven't specified anything for the typedef, then we don't care
    ComplexTypeEntry *type = types->findComplexType(fullClassName);
    if (!type)
        return nullptr;

    auto *metaClass = new AbstractMetaClass;
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
        auto *metaClass = new AbstractMetaClass;
        metaClass->setTypeDef(true);
        metaClass->setTypeEntry(te->target());
        metaClass->setBaseClassNames(QStringList(te->sourceType()));
        *metaClass += AbstractMetaAttributes::Public;
        fillAddedFunctions(metaClass);
        addAbstractMetaClass(metaClass, nullptr);
        // Ensure base classes are set up when traversing functions for the
        // type to be resolved.
        if (setupInheritance(metaClass)) {
            // Create an entry to look up up types obtained from parsing
            // functions in reverse. As opposed to container specializations,
            // which are generated into every instantiating module (indicated
            // by ContainerTypeEntry::targetLangPackage() being empty), the
            // correct index array of the module needs to be found by reverse
            // mapping the instantiations to the typedef entry.
            // Synthesize a AbstractMetaType which would be found by an
            // instantiation.
            AbstractMetaType sourceType;
            sourceType.setTypeEntry(metaClass->templateBaseClass()->typeEntry());
            sourceType.setInstantiations(metaClass->templateBaseClassInstantiations());
            sourceType.decideUsagePattern();
            m_typeSystemTypeDefs.append({sourceType, metaClass});
        }
    }
}

AbstractMetaClass *AbstractMetaBuilderPrivate::traverseClass(const FileModelItem &dom,
                                                             const ClassModelItem &classItem,
                                                             AbstractMetaClass *currentClass)
{
    QString className = stripTemplateArgs(classItem->name());
    QString fullClassName = className;

    // we have inner an class
    if (currentClass) {
        fullClassName = stripTemplateArgs(currentClass->typeEntry()->qualifiedCppName())
                          + colonColon() + fullClassName;
    }

    ComplexTypeEntry *type = TypeDatabase::instance()->findComplexType(fullClassName);
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
        if (fullClassName.isEmpty()) {
            QTextStream(&fullClassName) << "anonymous struct at " << classItem->fileName()
                << ':' << classItem->startLine();
        }
        m_rejectedClasses.insert(fullClassName, reason);
        return nullptr;
    }

    auto *metaClass = new AbstractMetaClass;
    metaClass->setSourceLocation(classItem->sourceLocation());
    metaClass->setTypeEntry(type);

    if (classItem->isFinal())
        *metaClass += AbstractMetaAttributes::FinalCppClass;

    QStringList baseClassNames;
    const QList<_ClassModelItem::BaseClass> &baseClasses = classItem->baseClasses();
    for (const _ClassModelItem::BaseClass &baseClass : baseClasses) {
        if (baseClass.accessPolicy == CodeModel::Public)
            baseClassNames.append(baseClass.name);
    }

    metaClass->setBaseClassNames(baseClassNames);
    *metaClass += AbstractMetaAttributes::Public;
    if (type->stream())
        metaClass->setStream(true);

    if (ReportHandler::isDebug(ReportHandler::MediumDebug)) {
        const QString message = type->isContainer()
            ? QStringLiteral("container: '%1'").arg(fullClassName)
            : QStringLiteral("class: '%1'").arg(metaClass->fullName());
        qCInfo(lcShiboken, "%s", qPrintable(message));
    }

    TemplateParameterList template_parameters = classItem->templateParameters();
    TypeEntries template_args;
    template_args.clear();
    auto argumentParent = metaClass->typeEntry()->typeSystemTypeEntry();
    for (int i = 0; i < template_parameters.size(); ++i) {
        const TemplateParameterModelItem &param = template_parameters.at(i);
        auto param_type = new TemplateArgumentEntry(param->name(), type->version(),
                                                    argumentParent);
        param_type->setOrdinal(i);
        template_args.append(param_type);
    }
    metaClass->setTemplateArguments(template_args);

    parseQ_Properties(metaClass, classItem->propertyDeclarations());

    traverseEnums(classItem, metaClass, classItem->enumsDeclarations());

    // Inner classes
    {
        const ClassList &innerClasses = classItem->classes();
        for (const ClassModelItem &ci : innerClasses) {
            AbstractMetaClass *cl = traverseClass(dom, ci, metaClass);
            if (cl) {
                cl->setEnclosingClass(metaClass);
                metaClass->addInnerClass(cl);
                addAbstractMetaClass(cl, ci.data());
            }
        }

    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    const TypeDefList typeDefs = classItem->typeDefs();
    for (const TypeDefModelItem &typeDef : typeDefs) {
        AbstractMetaClass *cls = traverseTypeDef(dom, typeDef, metaClass);
        if (cls) {
            cls->setEnclosingClass(metaClass);
            addAbstractMetaClass(cls, typeDef.data());
        }
    }

    // Set the default include file name
    if (!type->include().isValid())
        setInclude(type, classItem->fileName());

    return metaClass;
}

void AbstractMetaBuilderPrivate::traverseScopeMembers(const ScopeModelItem &item,
                                                      AbstractMetaClass *metaClass)
{
    // Classes/Namespace members
    traverseFields(item, metaClass);
    traverseFunctions(item, metaClass);

    // Inner classes
    const ClassList &innerClasses = item->classes();
    for (const ClassModelItem &ci : innerClasses)
        traverseClassMembers(ci);
}

void AbstractMetaBuilderPrivate::traverseClassMembers(const ClassModelItem &item)
{
    AbstractMetaClass *metaClass = m_itemToClass.value(item.data());
    if (!metaClass)
        return;

    // Class members
    traverseScopeMembers(item, metaClass);
}

void AbstractMetaBuilderPrivate::traverseNamespaceMembers(const NamespaceModelItem &item)
{
    AbstractMetaClass *metaClass = m_itemToClass.value(item.data());
    if (!metaClass)
        return;

    // Namespace members
    traverseScopeMembers(item, metaClass);

    // Inner namespaces
    for (const NamespaceModelItem &ni : item->namespaces())
        traverseNamespaceMembers(ni);

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

std::optional<AbstractMetaField>
    AbstractMetaBuilderPrivate::traverseField(const VariableModelItem &field,
                                              const AbstractMetaClass *cls)
{
    QString fieldName = field->name();
    QString className = cls->typeEntry()->qualifiedCppName();

    // Ignore friend decl.
    if (field->isFriend())
        return {};

    if (field->accessPolicy() == CodeModel::Private)
        return {};

    QString rejectReason;
    if (TypeDatabase::instance()->isFieldRejected(className, fieldName, &rejectReason)) {
        m_rejectedFields.insert(qualifiedFieldSignatureWithType(className, field) + rejectReason,
                                AbstractMetaBuilder::GenerationDisabled);
        return {};
    }


    AbstractMetaField metaField;
    metaField.setName(fieldName);
    metaField.setEnclosingClass(cls);

    TypeInfo fieldType = field->type();
    auto metaType = translateType(fieldType, cls);

    if (!metaType.has_value()) {
        const QString type = TypeInfo::resolveType(fieldType, currentScope()).qualifiedName().join(colonColon());
        if (cls->typeEntry()->generateCode()) {
             qCWarning(lcShiboken, "%s",
                       qPrintable(msgSkippingField(field, cls->name(), type)));
        }
        return {};
    }

    metaField.setType(metaType.value());

    AbstractMetaAttributes::Attributes attr;
    if (field->isStatic())
        attr |= AbstractMetaAttributes::Static;

    CodeModel::AccessPolicy policy = field->accessPolicy();
    if (policy == CodeModel::Public)
        attr |= AbstractMetaAttributes::Public;
    else if (policy == CodeModel::Protected)
        attr |= AbstractMetaAttributes::Protected;
    else
        attr |= AbstractMetaAttributes::Private;
    metaField.setAttributes(attr);

    return metaField;
}

static bool applyFieldModifications(AbstractMetaField *f)
{
    const auto &modifications = f->modifications();
    for (const auto &mod : modifications) {
        if (mod.isRemoved())
            return false;
        if (mod.isRenameModifier()) {
            f->setOriginalName(f->name());
            f->setName(mod.renamedToName());
        } else if (!mod.isReadable()) {
            f->setGetterEnabled(false);
        } else if (!mod.isWritable()) {
            f->setSetterEnabled(false);
        }
    }
    f->setOriginalAttributes(f->attributes());
    return true;
}

void AbstractMetaBuilderPrivate::traverseFields(const ScopeModelItem &scope_item,
                                                AbstractMetaClass *metaClass)
{
    const VariableList &variables = scope_item->variables();
    for (const VariableModelItem &field : variables) {
        auto metaFieldO = traverseField(field, metaClass);
        if (metaFieldO.has_value()) {
            AbstractMetaField metaField = metaFieldO.value();
            if (applyFieldModifications(&metaField))
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

    TypeDatabase *types = TypeDatabase::instance();
    static const QRegularExpression operatorRegExp(QStringLiteral("^operator "));
    Q_ASSERT(operatorRegExp.isValid());
    QString castTo = metaFunction->name().remove(operatorRegExp).trimmed();

    if (castTo.endsWith(QLatin1Char('&')))
        castTo.chop(1);
    if (castTo.startsWith(QLatin1String("const ")))
        castTo.remove(0, 6);

    TypeEntry *retType = types->findType(castTo);
    if (!retType)
        return;

    AbstractMetaType metaType(retType);
    metaType.decideUsagePattern();
    metaFunction->setType(metaType);
}

AbstractMetaFunctionRawPtrList
    AbstractMetaBuilderPrivate::classFunctionList(const ScopeModelItem &scopeItem,
                                                  AbstractMetaClass::Attributes *constructorAttributes,
                                                  AbstractMetaClass *currentClass)
{
    *constructorAttributes = {};
    AbstractMetaFunctionRawPtrList result;
    const FunctionList &scopeFunctionList = scopeItem->functions();
    result.reserve(scopeFunctionList.size());
    for (const FunctionModelItem &function : scopeFunctionList) {
        if (AbstractMetaFunction *metaFunction = traverseFunction(function, currentClass)) {
            result.append(metaFunction);
        } else if (function->functionType() == CodeModel::Constructor) {
            auto arguments = function->arguments();
            *constructorAttributes |= AbstractMetaAttributes::HasRejectedConstructor;
            if (arguments.isEmpty() || arguments.constFirst()->defaultValue())
                *constructorAttributes |= AbstractMetaAttributes::HasRejectedDefaultConstructor;
        }
    }
    return result;
}

void AbstractMetaBuilderPrivate::traverseFunctions(ScopeModelItem scopeItem,
                                                   AbstractMetaClass *metaClass)
{
    AbstractMetaAttributes::Attributes constructorAttributes;
    const AbstractMetaFunctionRawPtrList functions =
        classFunctionList(scopeItem, &constructorAttributes, metaClass);
    metaClass->setAttributes(metaClass->attributes() | constructorAttributes);

    for (AbstractMetaFunction *metaFunction : functions) {
        metaFunction->setOriginalAttributes(metaFunction->attributes());
        if (metaClass->isNamespace())
            *metaFunction += AbstractMetaAttributes::Static;

        const auto propertyFunction = metaClass->searchPropertyFunction(metaFunction->name());
        if (propertyFunction.index >= 0) {
           QPropertySpec prop = metaClass->propertySpecs().at(propertyFunction.index);
            switch (propertyFunction.function) {
            case AbstractMetaClass::PropertyFunction::Read:
                // Property reader must be in the form "<type> name()"
                if (!metaFunction->isSignal()
                    && prop.typeEntry() == metaFunction->type().typeEntry()
                    && metaFunction->arguments().isEmpty()) {
                    *metaFunction += AbstractMetaAttributes::PropertyReader;
                    metaFunction->setPropertySpecIndex(propertyFunction.index);
                }
                break;
            case AbstractMetaClass::PropertyFunction::Write:
                // Property setter must be in the form "void name(<type>)"
                // Make sure the function was created with all arguments; some
                // argument can be missing during the parsing because of errors
                // in the typesystem.
                if (metaFunction->isVoid() && metaFunction->arguments().size() == 1
                    && (prop.typeEntry() == metaFunction->arguments().at(0).type().typeEntry())) {
                    *metaFunction += AbstractMetaAttributes::PropertyWriter;
                    metaFunction->setPropertySpecIndex(propertyFunction.index);
                }
                break;
            case AbstractMetaClass::PropertyFunction::Reset:
                // Property resetter must be in the form "void name()"
                if (metaFunction->isVoid() && metaFunction->arguments().isEmpty()) {
                    *metaFunction += AbstractMetaAttributes::PropertyResetter;
                    metaFunction->setPropertySpecIndex(propertyFunction.index);
                }
                break;
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

        if (!metaFunction->isDestructor()
            && !(metaFunction->isPrivate() && metaFunction->functionType() == AbstractMetaFunction::ConstructorFunction)) {

            setupFunctionDefaults(metaFunction, metaClass);

            if (metaFunction->isSignal() && metaClass->hasSignal(metaFunction))
                qCWarning(lcShiboken, "%s", qPrintable(msgSignalOverloaded(metaClass, metaFunction)));

            if (metaFunction->isConversionOperator())
                fixReturnTypeOfConversionOperator(metaFunction);

            metaClass->addFunction(AbstractMetaFunctionCPtr(metaFunction));
            applyFunctionModifications(metaFunction);
        } else if (metaFunction->isDestructor()) {
            metaClass->setHasPrivateDestructor(metaFunction->isPrivate());
            metaClass->setHasProtectedDestructor(metaFunction->isProtected());
            metaClass->setHasVirtualDestructor(metaFunction->isVirtual());
        }
        if (!metaFunction->ownerClass()) {
            delete metaFunction;
            metaFunction = nullptr;
        }
    }

    fillAddedFunctions(metaClass);
}

void AbstractMetaBuilderPrivate::fillAddedFunctions(AbstractMetaClass *metaClass)
{
    // Add the functions added by the typesystem
    const AddedFunctionList &addedFunctions = metaClass->typeEntry()->addedFunctions();
    for (const AddedFunctionPtr &addedFunc : addedFunctions) {
        if (!traverseAddedMemberFunction(addedFunc, metaClass)) {
                qFatal("Unable to traverse function \"%s\" added to \"%s\".",
                       qPrintable(addedFunc->name()), qPrintable(metaClass->name()));
        }
    }
}

QString AbstractMetaBuilder::getSnakeCaseName(const QString &name)
{
    const int size = name.size();
    if (size < 3)
        return name;
    QString result;
    result.reserve(size + 4);
    for (int i = 0; i < size; ++i) {
        const QChar c = name.at(i);
        if (c.isUpper()) {
            if (i > 0) {
                if (name.at(i - 1).isUpper())
                    return name; // Give up at consecutive upper chars
                 result.append(u'_');
            }
            result.append(c.toLower());
        } else {
            result.append(c);
        }
    }
    return result;
}

// Names under which an item will be registered to Python depending on snakeCase
QStringList AbstractMetaBuilder::definitionNames(const QString &name,
                                                 TypeSystem::SnakeCase snakeCase)
{
    QStringList result;
    switch (snakeCase) {
    case TypeSystem::SnakeCase::Unspecified:
    case TypeSystem::SnakeCase::Disabled:
        result.append(name);
        break;
    case TypeSystem::SnakeCase::Enabled:
        result.append(AbstractMetaBuilder::getSnakeCaseName(name));
        break;
    case TypeSystem::SnakeCase::Both:
        result.append(AbstractMetaBuilder::getSnakeCaseName(name));
        if (name != result.constFirst())
            result.append(name);
        break;
    }
    return result;
}

void AbstractMetaBuilderPrivate::applyFunctionModifications(AbstractMetaFunction *func)
{
    AbstractMetaFunction& funcRef = *func;
    for (const FunctionModification &mod : func->modifications(func->implementingClass())) {
        if (mod.isRenameModifier()) {
            func->setOriginalName(func->name());
            func->setName(mod.renamedToName());
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
    if (metaClass->inheritanceDone())
        return true;

    metaClass->setInheritanceDone(true);

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

    for (const auto  &baseClassName : baseClasses) {
        if (!types->isClassRejected(baseClassName)) {
            auto typeEntry = types->findType(baseClassName);
            if (typeEntry == nullptr || !typeEntry->isComplex()) {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgBaseNotInTypeSystem(metaClass, baseClassName)));
                return false;
            }
            auto baseClass = AbstractMetaClass::findClass(m_metaClasses, typeEntry);
            if (!baseClass) {
                qCWarning(lcShiboken, "%s",
                          qPrintable(msgUnknownBase(metaClass, baseClassName)));
                return false;
            }
            metaClass->addBaseClass(baseClass);

            setupInheritance(baseClass);
        }
    }

    // Super class set by attribute "default-superclass".
    const QString defaultSuperclassName = metaClass->typeEntry()->defaultSuperclass();
    if (!defaultSuperclassName.isEmpty()) {
        auto defaultSuper = AbstractMetaClass::findClass(m_metaClasses, defaultSuperclassName);
        if (defaultSuper != nullptr) {
            metaClass->setDefaultSuperclass(defaultSuper);
        } else {
            QString message;
            QTextStream(&message) << "Class \"" << defaultSuperclassName
                << "\" specified as \"default-superclass\" of \"" << metaClass->name()
                << "\" could not be found in the code model.";
            qCWarning(lcShiboken, "%s", qPrintable(message));
        }
    }

    return true;
}

void AbstractMetaBuilderPrivate::traverseEnums(const ScopeModelItem &scopeItem,
                                               AbstractMetaClass *metaClass,
                                               const QStringList &enumsDeclarations)
{
    const EnumList &enums = scopeItem->enums();
    const QSet<QString> enumsDeclarationSet(enumsDeclarations.cbegin(), enumsDeclarations.cend());
    for (const EnumModelItem &enumItem : enums) {
        auto metaEnum = traverseEnum(enumItem, metaClass, enumsDeclarationSet);
        if (metaEnum.has_value()) {
            metaClass->addEnum(metaEnum.value());
        }
    }
}

static void applyDefaultExpressionModifications(const FunctionModificationList &functionMods,
                                                int i, AbstractMetaArgument *metaArg)
{
    // use replace/remove-default-expression for set default value
    for (const auto &modification : functionMods) {
        for (const auto &argumentModification : modification.argument_mods()) {
            if (argumentModification.index() == i + 1) {
                if (argumentModification.removedDefaultExpression()) {
                    metaArg->setDefaultValueExpression(QString());
                    break;
                }
                if (!argumentModification.replacedDefaultExpression().isEmpty()) {
                    metaArg->setDefaultValueExpression(argumentModification.replacedDefaultExpression());
                    break;
                }
            }
        }
    }
}

static AbstractMetaFunction::FunctionType functionTypeFromName(const QString &);

bool AbstractMetaBuilderPrivate::traverseAddedGlobalFunction(const AddedFunctionPtr &addedFunc)
{
    AbstractMetaFunction *metaFunction = traverseAddedFunctionHelper(addedFunc);
    if (metaFunction == nullptr)
        return false;
    m_globalFunctions << AbstractMetaFunctionCPtr(metaFunction);
    return true;
}

AbstractMetaFunction *
    AbstractMetaBuilderPrivate::traverseAddedFunctionHelper(const AddedFunctionPtr &addedFunc,
                                                            AbstractMetaClass *metaClass)
{
    QString errorMessage;

    auto returnType = translateType(addedFunc->returnType(), metaClass, {}, &errorMessage);
    if (!returnType.has_value()) {
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgAddedFunctionInvalidReturnType(addedFunc->name(),
                                                               addedFunc->returnType().qualifiedName(),
                                                               errorMessage,
                                                               metaClass)));
        return nullptr;
    }

    auto metaFunction = new AbstractMetaFunction(addedFunc);
    metaFunction->setType(returnType.value());
    metaFunction->setFunctionType(functionTypeFromName(addedFunc->name()));

    const auto &args = addedFunc->arguments();

    for (int i = 0; i < args.count(); ++i) {
        const AddedFunction::Argument &arg = args.at(i);
        auto type = translateType(arg.typeInfo, metaClass, {}, &errorMessage);
        if (Q_UNLIKELY(!type.has_value())) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgAddedFunctionInvalidArgType(addedFunc->name(),
                                                                arg.typeInfo.qualifiedName(), i + 1,
                                                                errorMessage,
                                                                metaClass)));
            delete metaFunction;
            return nullptr;
        }
        type->decideUsagePattern();

        AbstractMetaArgument metaArg;
        if (!args.at(i).name.isEmpty())
            metaArg.setName(args.at(i).name);
        metaArg.setType(type.value());
        metaArg.setArgumentIndex(i);
        metaArg.setDefaultValueExpression(arg.defaultValue);
        metaArg.setOriginalDefaultValueExpression(arg.defaultValue);
        metaFunction->addArgument(metaArg);
    }

    AbstractMetaArgumentList &metaArguments = metaFunction->arguments();

    if (metaFunction->isOperatorOverload() && !metaFunction->isCallOperator()) {
        if (metaArguments.size() > 2) {
            qCWarning(lcShiboken) << "An operator overload need to have 0, 1 or 2 arguments if it's reverse.";
        } else if (metaArguments.size() == 2) {
            // Check if it's a reverse operator
            if (metaArguments[1].type().typeEntry() == metaClass->typeEntry()) {
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
    const FunctionModificationList functionMods = metaFunction->modifications(metaClass);
    for (int i = 0; i < metaArguments.size(); ++i) {
        AbstractMetaArgument &metaArg = metaArguments[i];

        // use replace-default-expression for set default value
        applyDefaultExpressionModifications(functionMods, i, &metaArg);
        metaArg.setOriginalDefaultValueExpression(metaArg.defaultValueExpression()); // appear unmodified
    }

    metaFunction->setOriginalAttributes(metaFunction->attributes());
    if (!metaArguments.isEmpty())
        fixArgumentNames(metaFunction, metaFunction->modifications(metaClass));

    return metaFunction;
}

bool AbstractMetaBuilderPrivate::traverseAddedMemberFunction(const AddedFunctionPtr &addedFunc,
                                                             AbstractMetaClass *metaClass)
{
    AbstractMetaFunction *metaFunction = traverseAddedFunctionHelper(addedFunc, metaClass);
    if (metaFunction == nullptr)
        return false;

    const AbstractMetaArgumentList fargs = metaFunction->arguments();
    if (metaClass->isNamespace())
        *metaFunction += AbstractMetaFunction::Static;
    if (metaFunction->name() == metaClass->name()) {
        metaFunction->setFunctionType(AbstractMetaFunction::ConstructorFunction);
        if (fargs.size() == 1) {
            const TypeEntry *te = fargs.constFirst().type().typeEntry();
            if (te->isCustom())
                metaFunction->setExplicit(true);
            if (te->name() == metaFunction->name())
                metaFunction->setFunctionType(AbstractMetaFunction::CopyConstructorFunction);
        }
    }

    metaFunction->setDeclaringClass(metaClass);
    metaFunction->setImplementingClass(metaClass);
    metaClass->addFunction(AbstractMetaFunctionCPtr(metaFunction));
    metaClass->setHasNonPrivateConstructor(true);
    return true;
}

void AbstractMetaBuilderPrivate::fixArgumentNames(AbstractMetaFunction *func, const FunctionModificationList &mods)
{
    AbstractMetaArgumentList &arguments = func->arguments();

    for (const FunctionModification &mod : mods) {
        for (const ArgumentModification &argMod : mod.argument_mods()) {
            if (!argMod.renamedToName().isEmpty())
                arguments[argMod.index() - 1].setName(argMod.renamedToName(), false);
        }
    }

    for (int i = 0, size = arguments.size(); i < size; ++i) {
        if (arguments.at(i).name().isEmpty())
            arguments[i].setName(QLatin1String("arg__") + QString::number(i + 1), false);
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
    case CodeModel::AssignmentOperator:
        result = AbstractMetaFunction::AssignmentOperatorFunction;
        break;
    case CodeModel::CallOperator:
        result = AbstractMetaFunction::CallOperator;
        break;
    case CodeModel::ConversionOperator:
        result = AbstractMetaFunction::ConversionOperator;
        break;
    case CodeModel::DereferenceOperator:
        result = AbstractMetaFunction::DereferenceOperator;
        break;
    case CodeModel::ReferenceOperator:
        result = AbstractMetaFunction::ReferenceOperator;
        break;
    case CodeModel::ArrowOperator:
        result = AbstractMetaFunction::ArrowOperator;
        break;
    case CodeModel::ArithmeticOperator:
        result = AbstractMetaFunction::ArithmeticOperator;
        break;
    case CodeModel::BitwiseOperator:
        result = AbstractMetaFunction::BitwiseOperator;
        break;
    case CodeModel::LogicalOperator:
        result = AbstractMetaFunction::LogicalOperator;
        break;
    case CodeModel::ShiftOperator:
        result = AbstractMetaFunction::ShiftOperator;
        break;
    case CodeModel::SubscriptOperator:
        result = AbstractMetaFunction::SubscriptOperator;
        break;
    case CodeModel::ComparisonOperator:
        result = AbstractMetaFunction::ComparisonOperator;
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

static AbstractMetaFunction::FunctionType functionTypeFromName(const QString &name)
{
    if (name == u"__getattro__")
        return AbstractMetaFunction::GetAttroFunction;
    if (name == u"__setattro__")
        return AbstractMetaFunction::SetAttroFunction;
    const auto typeOpt = _FunctionModelItem::functionTypeFromName(name);
    if (typeOpt.has_value())
        return functionTypeFromCodeModel(typeOpt.value());
    return AbstractMetaFunction::NormalFunction;
}

// Apply the <array> modifications of the arguments
static bool applyArrayArgumentModifications(const FunctionModificationList &functionMods,
                                            AbstractMetaFunction *func,
                                            QString *errorMessage)
{
    for (const FunctionModification &mod : functionMods) {
        for (const ArgumentModification &argMod : mod.argument_mods()) {
            if (argMod.isArray()) {
                const int i = argMod.index() - 1;
                if (i < 0 || i >= func->arguments().size()) {
                    *errorMessage = msgCannotSetArrayUsage(func->minimalSignature(), i,
                                                           QLatin1String("Index out of range."));
                    return false;
                }
                auto t = func->arguments().at(i).type();
                if (!t.applyArrayModification(errorMessage)) {
                    *errorMessage = msgCannotSetArrayUsage(func->minimalSignature(), i, *errorMessage);
                    return false;
                }
                func->arguments()[i].setType(t);
            }
        }
    }
    return true;
}

AbstractMetaFunction *AbstractMetaBuilderPrivate::traverseFunction(const FunctionModelItem &functionItem,
                                                                   const AbstractMetaClass *currentClass)
{
    if (functionItem->isDeleted() || !functionItem->templateParameters().isEmpty())
        return nullptr;
    QString functionName = functionItem->name();
    QString className;
    if (currentClass) {
        // Clang: Skip qt_metacast(), qt_metacall(), expanded from Q_OBJECT
        // and overridden metaObject(), QGADGET helpers
        if (functionName == QLatin1String("qt_check_for_QGADGET_macro")
            || functionName.startsWith(QLatin1String("qt_meta"))) {
            return nullptr;
        }
        className = currentClass->typeEntry()->qualifiedCppName();
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

    if (rejected) {
        if (ReportHandler::isDebug(ReportHandler::MediumDebug)) {
            qCInfo(lcShiboken, "%s::%s was rejected by the type database (%s).",
                   qPrintable(className), qPrintable(signature), qPrintable(rejectReason));
        }
        return nullptr;
    }

    if (functionItem->isFriend())
        return nullptr;

    const bool deprecated = functionItem->isDeprecated();
    if (deprecated && m_skipDeprecated) {
        m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn + QLatin1String(" is deprecated."),
                                   AbstractMetaBuilder::GenerationDisabled);
        return nullptr;
    }

    auto *metaFunction = new AbstractMetaFunction;
    metaFunction->setSourceLocation(functionItem->sourceLocation());
    if (deprecated)
        *metaFunction += AbstractMetaAttributes::Deprecated;

    // Additional check for assignment/move assignment down below
    metaFunction->setFunctionType(functionTypeFromCodeModel(functionItem->functionType()));
    metaFunction->setConstant(functionItem->isConstant());
    metaFunction->setExceptionSpecification(functionItem->exceptionSpecification());

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
         metaFunction->setType(AbstractMetaType::createVoid());
        break;
    case AbstractMetaFunction::ConstructorFunction:
        metaFunction->setExplicit(functionItem->isExplicit());
        metaFunction->setName(currentClass->name());
        metaFunction->setType(AbstractMetaType::createVoid());
        break;
    default: {
        TypeInfo returnType = functionItem->type();

        if (TypeDatabase::instance()->isReturnTypeRejected(className, returnType.toString(), &rejectReason)) {
            m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn + rejectReason, AbstractMetaBuilder::GenerationDisabled);
            delete metaFunction;
            return nullptr;
        }

        auto type = translateType(returnType, currentClass, {}, &errorMessage);
        if (!type.has_value()) {
            const QString reason = msgUnmatchedReturnType(functionItem, errorMessage);
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgSkippingFunction(functionItem, originalQualifiedSignatureWithReturn, reason)));
            m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn, AbstractMetaBuilder::UnmatchedReturnType);
            delete metaFunction;
            return nullptr;
        }

        metaFunction->setType(type.value());
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

    for (int i = 0; i < arguments.size(); ++i) {
        const ArgumentModelItem &arg = arguments.at(i);

        if (TypeDatabase::instance()->isArgumentTypeRejected(className, arg->type().toString(), &rejectReason)) {
            m_rejectedFunctions.insert(originalQualifiedSignatureWithReturn + rejectReason, AbstractMetaBuilder::GenerationDisabled);
            delete metaFunction;
            return nullptr;
        }

        auto metaTypeO = translateType(arg->type(), currentClass, {}, &errorMessage);
        if (!metaTypeO.has_value()) {
            // If an invalid argument has a default value, simply remove it
            // unless the function is virtual (since the override in the
            // wrapper can then not correctly be generated).
            if (arg->defaultValue() && !functionItem->isVirtual()) {
                if (!currentClass || currentClass->typeEntry()->generateCode()) {
                    qCWarning(lcShiboken, "%s",
                              qPrintable(msgStrippingArgument(functionItem, i, originalQualifiedSignatureWithReturn, arg)));
                }
                break;
            }
            const QString reason = msgUnmatchedParameterType(arg, i, errorMessage);
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgSkippingFunction(functionItem, originalQualifiedSignatureWithReturn, reason)));
            const QString rejectedFunctionSignature = originalQualifiedSignatureWithReturn
                + QLatin1String(": ") + reason;
            m_rejectedFunctions.insert(rejectedFunctionSignature, AbstractMetaBuilder::UnmatchedArgumentType);
            delete metaFunction;
            return nullptr;
        }

        auto metaType = metaTypeO.value();
        // Add view substitution for simple view types of function arguments
        // std::string_view -> std::string for foo(std::string_view)
        auto viewOnTypeEntry = metaType.typeEntry()->viewOn();
        if (viewOnTypeEntry != nullptr && metaType.indirections() == 0
            && metaType.arrayElementType() == nullptr
            && !metaType.hasInstantiations()) {
            auto viewOn = metaType;
            viewOn.setTypeEntry(viewOnTypeEntry);
            metaType.setViewOn(viewOn);
        }

        AbstractMetaArgument metaArgument;
        metaArgument.setType(metaType);
        metaArgument.setName(arg->name());
        metaArgument.setArgumentIndex(i);
        metaFunction->addArgument(metaArgument);
    }

    AbstractMetaArgumentList &metaArguments = metaFunction->arguments();

    const FunctionModificationList functionMods = currentClass
        ? AbstractMetaFunction::findClassModifications(metaFunction, currentClass)
        : AbstractMetaFunction::findGlobalModifications(metaFunction);

    for (const FunctionModification &mod : functionMods) {
        if (mod.exceptionHandling() != TypeSystem::ExceptionHandling::Unspecified)
            metaFunction->setExceptionHandlingModification(mod.exceptionHandling());
        else if (mod.allowThread() != TypeSystem::AllowThread::Unspecified)
            metaFunction->setAllowThreadModification(mod.allowThread());
    }

    // Find the correct default values
    for (int i = 0, size = metaArguments.size(); i < size; ++i) {
        const ArgumentModelItem &arg = arguments.at(i);
        AbstractMetaArgument &metaArg = metaArguments[i];

        const QString originalDefaultExpression =
            fixDefaultValue(arg, metaArg.type(), currentClass, i);

        metaArg.setOriginalDefaultValueExpression(originalDefaultExpression);
        metaArg.setDefaultValueExpression(originalDefaultExpression);

        applyDefaultExpressionModifications(functionMods, i, &metaArg);

        //Check for missing argument name
        if (!metaArg.defaultValueExpression().isEmpty()
            && !metaArg.hasName()
            && !metaFunction->isOperatorOverload()
            && !metaFunction->isSignal()
            && metaFunction->argumentName(i + 1, false, currentClass).isEmpty()) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgUnnamedArgumentDefaultExpression(currentClass, i + 1,
                                                                     className, metaFunction)));
        }

    }

    if (!metaArguments.isEmpty()) {
        fixArgumentNames(metaFunction, functionMods);
        QString errorMessage;
        if (!applyArrayArgumentModifications(functionMods, metaFunction, &errorMessage)) {
            qCWarning(lcShiboken, "%s",
                      qPrintable(msgArrayModificationFailed(functionItem, className, errorMessage)));
        }
    }

    // Determine class special functions
    if (currentClass && metaFunction->arguments().size() == 1) {
        const AbstractMetaType &argType = metaFunction->arguments().constFirst().type();
        if (argType.typeEntry() == currentClass->typeEntry() && argType.indirections() == 0) {
            if (metaFunction->name() == QLatin1String("operator=")) {
                switch (argType.referenceType()) {
                case NoReference:
                    metaFunction->setFunctionType(AbstractMetaFunction::AssignmentOperatorFunction);
                    break;
                case LValueReference:
                    if (argType.isConstant())
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

static const TypeEntry* findTypeEntryUsingContext(const AbstractMetaClass* metaClass, const QString& qualifiedName)
{
    const TypeEntry* type = nullptr;
    QStringList context = metaClass->qualifiedCppName().split(colonColon());
    while (!type && !context.isEmpty()) {
        type = TypeDatabase::instance()->findType(context.join(colonColon()) + colonColon() + qualifiedName);
        context.removeLast();
    }
    return type;
}

// Helper for findTypeEntries/translateTypeStatic()
TypeEntries AbstractMetaBuilderPrivate::findTypeEntriesHelper(const QString &qualifiedName,
                                                              const QString &name,
                                                              const AbstractMetaClass *currentClass,
                                                              AbstractMetaBuilderPrivate *d)
{
    // 5.1 - Try first using the current scope
    if (currentClass) {
        if (auto type = findTypeEntryUsingContext(currentClass, qualifiedName))
            return {type};

        // 5.1.1 - Try using the class parents' scopes
        if (d && !currentClass->baseClassNames().isEmpty()) {
            const AbstractMetaClassList &baseClasses = d->getBaseClasses(currentClass);
            for (const AbstractMetaClass *cls : baseClasses) {
                if (auto type = findTypeEntryUsingContext(cls, qualifiedName))
                    return {type};
            }
        }
    }

    // 5.2 - Try without scope
    auto types = TypeDatabase::instance()->findCppTypes(qualifiedName);
    if (!types.isEmpty())
        return types;

    // 6. No? Try looking it up as a flags type
    if (auto type = TypeDatabase::instance()->findFlagsType(qualifiedName))
        return {type};

    // 7. No? Try looking it up as a container type
    if (auto type = TypeDatabase::instance()->findContainerType(name))
        return {type};

    // 8. No? Check if the current class is a template and this type is one
    //    of the parameters.
    if (currentClass) {
        const auto &template_args = currentClass->templateArguments();
        for (const TypeEntry *te : template_args) {
            if (te->name() == qualifiedName)
                return {te};
        }
    }
    return {};
}

// Helper for translateTypeStatic() that calls findTypeEntriesHelper()
// and does some error checking.
TypeEntries AbstractMetaBuilderPrivate::findTypeEntries(const QString &qualifiedName,
                                                        const QString &name,
                                                        const AbstractMetaClass *currentClass,
                                                        AbstractMetaBuilderPrivate *d,
                                                        QString *errorMessage)
{
    const TypeEntries types = findTypeEntriesHelper(qualifiedName, name, currentClass, d);
    if (types.isEmpty()) {
        if (errorMessage != nullptr)
            *errorMessage = msgCannotFindTypeEntry(qualifiedName);
        return {};
    }

    if (types.size() == 1)
        return types;

    const auto typeEntryType = types.constFirst()->type();
    const bool sameType = std::all_of(types.cbegin() + 1, types.cend(),
                                      [typeEntryType](const TypeEntry *e) {
                                          return e->type() == typeEntryType;
                                      });

    if (!sameType) {
        if (errorMessage != nullptr)
            *errorMessage = msgAmbiguousVaryingTypesFound(qualifiedName, types);
        return {};
    }
    // Ambiguous primitive/smart pointer types are possible (when
    // including type systems).
    if (typeEntryType != TypeEntry::PrimitiveType
        && typeEntryType != TypeEntry::SmartPointerType) {
        if (errorMessage != nullptr)
            *errorMessage = msgAmbiguousTypesFound(qualifiedName, types);
        return {};
    }
    return types;
}

// Reverse lookup of AbstractMetaType representing a template specialization
// found during traversing function arguments to its type system typedef'ed
// class.
const AbstractMetaClass *AbstractMetaBuilderPrivate::resolveTypeSystemTypeDef(const AbstractMetaType &t) const
{
    if (t.hasInstantiations()) {
        auto pred = [t](const TypeClassEntry &e) { return e.type.equals(t); };
        auto it = std::find_if(m_typeSystemTypeDefs.cbegin(), m_typeSystemTypeDefs.cend(), pred);
        if (it != m_typeSystemTypeDefs.cend())
            return it->klass;
    }
    return nullptr;
}

std::optional<AbstractMetaType>
    AbstractMetaBuilderPrivate::translateType(const TypeInfo &_typei,
                                              const AbstractMetaClass *currentClass,
                                              TranslateTypeFlags flags,
                                              QString *errorMessage)
{
    return translateTypeStatic(_typei, currentClass, this, flags, errorMessage);
}

static bool isNumber(const QString &s)
{
    return std::all_of(s.cbegin(), s.cend(),
                       [](QChar c) { return c.isDigit(); });
}

std::optional<AbstractMetaType>
    AbstractMetaBuilderPrivate::translateTypeStatic(const TypeInfo &_typei,
                                                    const AbstractMetaClass *currentClass,
                                                    AbstractMetaBuilderPrivate *d,
                                                    TranslateTypeFlags flags,
                                                    QString *errorMessageIn)
{
    if (_typei.isVoid())
        return AbstractMetaType::createVoid();

    // 1. Test the type info without resolving typedefs in case this is present in the
    //    type system
    const bool resolveType = !flags.testFlag(AbstractMetaBuilder::DontResolveType);
    if (resolveType) {
        auto resolved =
            translateTypeStatic(_typei, currentClass, d,
                                flags | AbstractMetaBuilder::DontResolveType,
                                errorMessageIn);
        if (resolved.has_value())
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
        return {};
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

        auto elementType = translateTypeStatic(newInfo, currentClass, d, flags, &errorMessage);
        if (!elementType.has_value()) {
            if (errorMessageIn) {
                errorMessage.prepend(QLatin1String("Unable to translate array element: "));
                *errorMessageIn = msgUnableToTranslateType(_typei, errorMessage);
            }
            return {};
        }

        for (int i = typeInfo.arrayElements().size() - 1; i >= 0; --i) {
            AbstractMetaType arrayType;
            arrayType.setArrayElementType(elementType.value());
            const QString &arrayElement = typeInfo.arrayElements().at(i);
            if (!arrayElement.isEmpty()) {
                bool _ok;
                const qint64 elems = d
                    ? d->findOutValueFromString(arrayElement, _ok)
                    : arrayElement.toLongLong(&_ok, 0);
                if (_ok)
                    arrayType.setArrayElementCount(int(elems));
            }
            auto elementTypeEntry = elementType->typeEntry();
            arrayType.setTypeEntry(new ArrayTypeEntry(elementTypeEntry, elementTypeEntry->version(),
                                                      elementTypeEntry->parent()));
            arrayType.decideUsagePattern();

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
        return {};
    }

    QString qualifiedName = qualifierList.join(colonColon());
    QString name = qualifierList.takeLast();

    // 4. Special case QFlags (include instantiation in name)
    if (qualifiedName == QLatin1String("QFlags")) {
        qualifiedName = typeInfo.toString();
        typeInfo.clearInstantiations();
    }

    const TypeEntries types = findTypeEntries(qualifiedName, name, currentClass, d, errorMessageIn);
    if (types.isEmpty()) {
        if (errorMessageIn != nullptr)
            *errorMessageIn = msgUnableToTranslateType(_typei, *errorMessageIn);
        return {};
    }

    const TypeEntry *type = types.constFirst();
    const TypeEntry::Type typeEntryType = type->type();

    AbstractMetaType metaType;
    metaType.setIndirectionsV(typeInfo.indirectionsV());
    metaType.setReferenceType(typeInfo.referenceType());
    metaType.setConstant(typeInfo.isConstant());
    metaType.setVolatile(typeInfo.isVolatile());
    metaType.setOriginalTypeDescription(_typei.toString());

    const auto &templateArguments = typeInfo.instantiations();
    for (int t = 0, size = templateArguments.size(); t < size; ++t) {
        const  TypeInfo &ti = templateArguments.at(t);
        auto targType = translateTypeStatic(ti, currentClass, d, flags, &errorMessage);
        // For non-type template parameters, create a dummy type entry on the fly
        // as is done for classes.
        if (!targType.has_value()) {
            const QString value = ti.qualifiedName().join(colonColon());
            if (isNumber(value)) {
                TypeDatabase::instance()->addConstantValueTypeEntry(value, type->typeSystemTypeEntry());
                targType = translateTypeStatic(ti, currentClass, d, flags, &errorMessage);
            }
        }
        if (!targType) {
            if (errorMessageIn)
                *errorMessageIn = msgCannotTranslateTemplateArgument(t, ti, errorMessage);
            return {};
        }

        metaType.addInstantiation(targType.value());
    }

    if (typeEntryType == TypeEntry::SmartPointerType) {
        // Find a matching instantiation
        if (metaType.instantiations().size() != 1) {
            if (errorMessageIn)
                *errorMessageIn = msgInvalidSmartPointerType(_typei);
            return {};
        }
        auto instantiationType = metaType.instantiations().constFirst().typeEntry();
        if (instantiationType->type() == TypeEntry::TemplateArgumentType) {
            // Member functions of the template itself, SharedPtr(const SharedPtr &)
            type = instantiationType;
        } else {
            auto it = std::find_if(types.cbegin(), types.cend(),
                                   [instantiationType](const TypeEntry *e) {
                auto smartPtr = static_cast<const SmartPointerTypeEntry *>(e);
                return smartPtr->matchesInstantiation(instantiationType);
            });
            if (it == types.cend()) {
                if (errorMessageIn)
                    *errorMessageIn = msgCannotFindSmartPointerInstantion(_typei);
                return {};
            }
            type =*it;
        }
    }

    metaType.setTypeEntry(type);

    // The usage pattern *must* be decided *after* the possible template
    // instantiations have been determined, or else the absence of
    // such instantiations will break the caching scheme of
    // AbstractMetaType::cppSignature().
    metaType.decideUsagePattern();

    if (d) {
        // Reverse lookup of type system typedefs. Replace by class.
        if (auto klass = d->resolveTypeSystemTypeDef(metaType)) {
            metaType = AbstractMetaType{};
            metaType.setTypeEntry(klass->typeEntry());
            metaType.decideUsagePattern();
        }
    }

    return metaType;
}

std::optional<AbstractMetaType>
    AbstractMetaBuilder::translateType(const TypeInfo &_typei,
                                       AbstractMetaClass *currentClass,
                                       TranslateTypeFlags flags,
                                       QString *errorMessage)
{
    return AbstractMetaBuilderPrivate::translateTypeStatic(_typei, currentClass,
                                                           nullptr, flags,
                                                           errorMessage);
}

std::optional<AbstractMetaType>
    AbstractMetaBuilder::translateType(const QString &t,
                                      AbstractMetaClass *currentClass,
                                      TranslateTypeFlags flags,
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
        return {};
    }
    return translateType(typeInfo, currentClass, flags, errorMessageIn);
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

    auto enumValue = AbstractMetaClass::findEnumValue(m_metaClasses, stringValue);
    if (enumValue.has_value()) {
        ok = true;
        return enumValue->value().value();
    }

    for (const AbstractMetaEnum &metaEnum : qAsConst(m_globalEnums)) {
        auto ev = metaEnum.findEnumValue(stringValue);
        if (ev.has_value()) {
            ok = true;
            return ev->value().value();
        }
    }

    ok = false;
    return 0;
}

QString AbstractMetaBuilderPrivate::fixDefaultValue(const ArgumentModelItem &item,
                                                    const AbstractMetaType &type,
                                                    const AbstractMetaClass *implementingClass,
                                                    int /* argumentIndex */)
{
    QString expr = item->defaultValueExpression();
    if (expr.isEmpty())
        return expr;

    if (type.isPrimitive()) {
        if (type.name() == QLatin1String("boolean")) {
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
            // translation until all namespaces are completely
            // processed. This is done in figureOutEnumValues()
        }
    } else if (type.isFlags() || type.isEnum()) {
        bool isNumber;
        expr.toInt(&isNumber);
        if (!isNumber && expr.indexOf(colonColon()) < 0) {
            // Add the enum/flag scope to default value, making it usable
            // from other contexts beside its owner class hierarchy
            static const QRegularExpression typeRegEx(QStringLiteral("[^<]*[<]([^:]*::).*"));
            Q_ASSERT(typeRegEx.isValid());
            const QRegularExpressionMatch match = typeRegEx.match(type.minimalSignature());
            if (match.hasMatch())
                expr.prepend(match.captured(1));
        }
    } else if (type.isContainer() && expr.contains(QLatin1Char('<'))) {
        static const QRegularExpression typeRegEx(QStringLiteral("[^<]*<(.*)>"));
        Q_ASSERT(typeRegEx.isValid());
        const QRegularExpressionMatch typeMatch = typeRegEx.match(type.minimalSignature());
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

        // Fix the scope for constructor using the already resolved argument
        // type as a reference. The following regular expression extracts any
        // use of namespaces/scopes from the type string.
        static const QRegularExpression
            typeRegEx(QLatin1String(R"(^(?:const[\s]+|)([\w:]*::|)([A-Za-z_]\w*)\s*[&\*]?$)"));
        Q_ASSERT(typeRegEx.isValid());
        const QRegularExpressionMatch typeMatch = typeRegEx.match(type.minimalSignature());

        QString typeNamespace = typeMatch.hasMatch() ? typeMatch.captured(1) : QString();
        QString typeCtorName  = typeMatch.hasMatch() ? typeMatch.captured(2) : QString();
        if (!typeNamespace.isEmpty() && defaultValueCtorName == typeCtorName)
            expr.prepend(typeNamespace);

        // Fix scope if the parameter is a field of the current class
        if (implementingClass) {
            const AbstractMetaFieldList &fields = implementingClass->fields();
            for (const AbstractMetaField &field : fields) {
                if (defaultMatch.hasMatch() && defaultMatch.captured(2) == field.name()) {
                    expr = defaultMatch.captured(1) + implementingClass->name()
                           + colonColon() + defaultMatch.captured(2) + defaultMatch.captured(3);
                    break;
                }
            }
        }
    }

    return expr;
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

        AbstractMetaClass *templ = nullptr;
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

    return nullptr;
}

AbstractMetaClassList AbstractMetaBuilderPrivate::getBaseClasses(const AbstractMetaClass *metaClass) const
{
    // Shortcut if inheritance has already been set up
    if (metaClass->inheritanceDone() || !metaClass->needsInheritanceSetup())
        return metaClass->baseClasses();
    AbstractMetaClassList baseClasses;
    const QStringList &baseClassNames = metaClass->baseClassNames();
    for (const QString& parent : baseClassNames) {
        AbstractMetaClass *cls = nullptr;
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

std::optional<AbstractMetaType>
    AbstractMetaBuilderPrivate::inheritTemplateType(const AbstractMetaTypeList &templateTypes,
                                                    const AbstractMetaType &metaType)
{
    auto returned = metaType;

    if (!metaType.typeEntry()->isTemplateArgument() && !metaType.hasInstantiations())
        return returned;

    returned.setOriginalTemplateType(metaType);

    if (returned.typeEntry()->isTemplateArgument()) {
        const auto *tae = static_cast<const TemplateArgumentEntry*>(returned.typeEntry());

        // If the template is intantiated with void we special case this as rejecting the functions that use this
        // parameter from the instantiation.
        const AbstractMetaType &templateType = templateTypes.value(tae->ordinal());
        if (templateType.typeEntry()->isVoid())
            return {};

        AbstractMetaType t = returned;
        t.setTypeEntry(templateType.typeEntry());
        t.setIndirections(templateType.indirections() + t.indirections() ? 1 : 0);
        t.decideUsagePattern();

        return inheritTemplateType(templateTypes, t);
    }

    if (returned.hasInstantiations()) {
        AbstractMetaTypeList instantiations = returned.instantiations();
        for (int i = 0; i < instantiations.count(); ++i) {
            auto ins = inheritTemplateType(templateTypes, instantiations.at(i));
            if (!ins.has_value())
                return {};
            instantiations[i] = ins.value();
        }
        returned.setInstantiations(instantiations);
    }

    return returned;
}

bool AbstractMetaBuilderPrivate::inheritTemplate(AbstractMetaClass *subclass,
                                                 const AbstractMetaClass *templateClass,
                                                 const TypeInfo &info)
{
    AbstractMetaTypeList  templateTypes;

    for (const TypeInfo &i : info.instantiations()) {
        QString typeName = i.qualifiedName().join(colonColon());
        TypeDatabase *typeDb = TypeDatabase::instance();
        TypeEntry *t = nullptr;
        // Check for a non-type template integer parameter, that is, for a base
        // "template <int R, int C> Matrix<R, C>" and subclass
        // "typedef Matrix<2,3> Matrix2x3;". If so, create dummy entries of
        // EnumValueTypeEntry for the integer values encountered on the fly.
        if (isNumber(typeName)) {
            t = typeDb->findType(typeName);
            if (!t) {
                auto parent = subclass->typeEntry()->typeSystemTypeEntry();
                t = TypeDatabase::instance()->addConstantValueTypeEntry(typeName, parent);
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
            AbstractMetaType temporaryType(t);
            temporaryType.setConstant(i.isConstant());
            temporaryType.setReferenceType(i.referenceType());
            temporaryType.setIndirectionsV(i.indirectionsV());
            temporaryType.decideUsagePattern();
            templateTypes << temporaryType;
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << "Ignoring template parameter " << typeName << " from "
                << info.toString() << ". The corresponding type was not found in the typesystem.";
        }
    }

    subclass->setTemplateBaseClass(templateClass);
    subclass->setTemplateBaseClassInstantiations(templateTypes);
    subclass->setBaseClass(templateClass->baseClass());
    return true;
}

void AbstractMetaBuilderPrivate::inheritTemplateFunctions(AbstractMetaClass *subclass)
{
    QString errorMessage;
    auto templateClass = subclass->templateBaseClass();

    if (subclass->isTypeDef()) {
        subclass->setHasCloneOperator(templateClass->hasCloneOperator());
        subclass->setHasEqualsOperator(templateClass->hasEqualsOperator());
        subclass->setHasHashFunction(templateClass->hasHashFunction());
        subclass->setHasNonPrivateConstructor(templateClass->hasNonPrivateConstructor());
        subclass->setHasPrivateDestructor(templateClass->hasPrivateDestructor());
        subclass->setHasProtectedDestructor(templateClass->hasProtectedDestructor());
        subclass->setHasVirtualDestructor(templateClass->hasVirtualDestructor());
    }

    const auto &templateTypes = subclass->templateBaseClassInstantiations();
    const AbstractMetaFunctionCList existingSubclassFuncs =
        subclass->functions(); // Take copy
    const auto &templateClassFunctions = templateClass->functions();
    for (const auto &function : templateClassFunctions) {
        // If the function is modified or the instantiation has an equally named
        // function we have shadowing, so we need to skip it.
        if (function->isModifiedRemoved()
            || AbstractMetaFunction::find(existingSubclassFuncs, function->name()) != nullptr) {
            continue;
        }

        std::unique_ptr<AbstractMetaFunction> f(function->copy());
        f->setArguments(AbstractMetaArgumentList());

        if (!function->isVoid()) {
            auto returnType = inheritTemplateType(templateTypes, function->type());
            if (!returnType.has_value())
                continue;
            f->setType(returnType.value());
        }

        const AbstractMetaArgumentList &arguments = function->arguments();
        for (const AbstractMetaArgument &argument : arguments) {
            auto argType = inheritTemplateType(templateTypes, argument.type());
            if (!argType.has_value())
                break;
            AbstractMetaArgument arg = argument;
            arg.setType(argType.value());
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


        if (!applyArrayArgumentModifications(f->modifications(subclass), f.get(),
                                             &errorMessage)) {
            qCWarning(lcShiboken, "While specializing %s (%s): %s",
                      qPrintable(subclass->name()), qPrintable(templateClass->name()),
                      qPrintable(errorMessage));
        }
        subclass->addFunction(AbstractMetaFunctionCPtr(f.release()));
    }

     // Take copy
    const AbstractMetaFieldList existingSubclassFields = subclass->fields();
    const AbstractMetaFieldList &templateClassFields = templateClass->fields();
    for (const AbstractMetaField &field : templateClassFields) {
        // If the field is modified or the instantiation has a field named
        // the same as an existing field we have shadowing, so we need to skip it.
        if (field.isModifiedRemoved(TypeSystem::All)
            || field.attributes().testFlag(AbstractMetaAttributes::Static)
            || AbstractMetaField::find(existingSubclassFields, field.name()).has_value()) {
            continue;
        }

        AbstractMetaField f = field;
        f.setEnclosingClass(subclass);
        auto fieldType = inheritTemplateType(templateTypes, field.type());
        if (!fieldType.has_value())
            continue;
        f.setType(fieldType.value());
        subclass->addField(f);
    }
}

void AbstractMetaBuilderPrivate::parseQ_Properties(AbstractMetaClass *metaClass,
                                                   const QStringList &declarations)
{
    const QStringList scopes = currentScope()->qualifiedName();
    QString errorMessage;
    int i = 0;
    for (; i < declarations.size(); ++i) {
        auto spec = QPropertySpec::parseQ_Property(this, metaClass, declarations.at(i), scopes, &errorMessage);
        if (spec.has_value()) {
            spec->setIndex(i);
            metaClass->addPropertySpec(spec.value());
        } else {
            QString message;
            QTextStream str(&message);
            str << metaClass->sourceLocation() << errorMessage;
            qCWarning(lcShiboken, "%s", qPrintable(message));
        }
    }

    // User-added properties
    auto typeEntry = metaClass->typeEntry();
    for (const TypeSystemProperty &tp : typeEntry->properties()) {
        std::optional<QPropertySpec> spec;
        if (metaClass->propertySpecByName(tp.name))
            errorMessage = msgPropertyExists(metaClass->name(), tp.name);
        else
            spec = QPropertySpec::fromTypeSystemProperty(this, metaClass, tp, scopes, &errorMessage);

        if (spec.has_value()) {
            spec->setIndex(i++);
            metaClass->addPropertySpec(spec.value());
        } else {
            QString message;
            QTextStream str(&message);
            str << typeEntry->sourceLocation() << errorMessage;
            qCWarning(lcShiboken, "%s", qPrintable(message));
        }
    }
}

static AbstractMetaFunctionCPtr findCopyCtor(AbstractMetaClass* cls)
{
    for (const auto &f : cls->functions()) {
        const AbstractMetaFunction::FunctionType t = f->functionType();
        if (t == AbstractMetaFunction::CopyConstructorFunction || t == AbstractMetaFunction::AssignmentOperatorFunction)
            return f;
    }
    return {};
}

void AbstractMetaBuilderPrivate::setupClonable(AbstractMetaClass *cls)
{
    bool result = true;

    // find copy ctor for the current class
    auto copyCtor = findCopyCtor(cls);
    if (!copyCtor.isNull()) { // if exists a copy ctor in this class
        result = copyCtor->isPublic();
    } else { // else... lets find one in the parent class
        QQueue<AbstractMetaClass*> baseClasses;
        if (cls->baseClass())
            baseClasses.enqueue(cls->baseClass());

        while (!baseClasses.isEmpty()) {
            AbstractMetaClass* currentClass = baseClasses.dequeue();
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
    const auto &convOps = cls->operatorOverloads(OperatorQueryOption::ConversionOp);
    for (const auto &func : convOps) {
        if (func->isModifiedRemoved())
            continue;
        AbstractMetaClass *metaClass = AbstractMetaClass::findClass(m_metaClasses, func->type().typeEntry());
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
        qCWarning(lcShiboken, "%s", qPrintable(msgCannotOpenForWriting(f)));
        return;
    }

    QTextStream s(&f);


    for (int reason = 0; reason < AbstractMetaBuilder::NoReason; ++reason) {
        s << QString(72, QLatin1Char('*')) << Qt::endl;
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

        case AbstractMetaBuilder::Deprecated:
            s << "Deprecated";
            break;

        default:
            s << "unknown reason";
            break;
        }

        s << Qt::endl;

        for (QMap<QString, AbstractMetaBuilder::RejectReason>::const_iterator it = rejects.constBegin();
             it != rejects.constEnd(); ++it) {
            if (it.value() != reason)
                continue;
            s << " - " << it.key() << Qt::endl;
        }

        s << QString(72, QLatin1Char('*')) << Qt::endl << Qt::endl;
    }

}

void AbstractMetaBuilderPrivate::dumpLog() const
{
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_classes.log"), m_rejectedClasses);
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_enums.log"), m_rejectedEnums);
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_functions.log"), m_rejectedFunctions);
    writeRejectLogFile(m_logDirectory + QLatin1String("mjb_rejected_fields.log"), m_rejectedFields);
}

using ClassGraph = Graph<AbstractMetaClass *>;

// Add a dependency of the class associated with typeEntry on clazz
static bool addClassDependency(const AbstractMetaClassList &classList,
                               const TypeEntry *typeEntry,
                               AbstractMetaClass *clazz,
                               ClassGraph *graph)
{
    if (!typeEntry->isComplex() || typeEntry == clazz->typeEntry())
        return false;
    const auto c = AbstractMetaClass::findClass(classList, typeEntry);
    if (c == nullptr || c->enclosingClass() == clazz)
        return false;
    return graph->addEdge(c, clazz);
}

AbstractMetaClassList AbstractMetaBuilderPrivate::classesTopologicalSorted(const AbstractMetaClassList &classList,
                                                                           const Dependencies &additionalDependencies)
{
    ClassGraph graph(classList.cbegin(), classList.cend());

    for (const auto &dep : additionalDependencies) {
        if (!graph.addEdge(dep.parent, dep.child)) {
            qCWarning(lcShiboken).noquote().nospace()
                << "AbstractMetaBuilder::classesTopologicalSorted(): Invalid additional dependency: "
                << dep.child->name() << " -> " << dep.parent->name() << '.';
        }
    }

    for (AbstractMetaClass *clazz : classList) {
        if (auto enclosingC = clazz->enclosingClass()) {
            auto enclosing = const_cast<AbstractMetaClass *>(enclosingC);
            graph.addEdge(enclosing, clazz);
        }

        for (auto baseClass : clazz->baseClasses())
            graph.addEdge(baseClass, clazz);

        for (const auto &func : clazz->functions()) {
            const AbstractMetaArgumentList &arguments = func->arguments();
            for (const AbstractMetaArgument &arg : arguments) {
                // Check methods with default args: If a class is instantiated by value,
                // ("QString s = QString()"), add a dependency.
                if (!arg.originalDefaultValueExpression().isEmpty()
                    && arg.type().isValue()) {
                    addClassDependency(classList, arg.type().typeEntry(),
                                       clazz, &graph);
                }
            }
        }
        // Member fields need to be initialized
        for (const AbstractMetaField &field : clazz->fields()) {
            addClassDependency(classList, field.type().typeEntry(),
                               clazz, &graph);
        }
    }

    const auto result = graph.topologicalSort();
    if (!result.isValid() && graph.nodeCount()) {
        QTemporaryFile tempFile(QDir::tempPath() + QLatin1String("/cyclic_depXXXXXX.dot"));
        tempFile.setAutoRemove(false);
        tempFile.open();
        graph.dumpDot(tempFile.fileName(),
                      [] (const AbstractMetaClass *c) { return c->name(); });

        QString message;
        QTextStream str(&message);
        str << "Cyclic dependency of classes found:";
        for (auto c : result.cyclic)
            str << ' ' << c->name();
        str << ". Graph can be found at \"" << QDir::toNativeSeparators(tempFile.fileName()) << '"';
        qCWarning(lcShiboken, "%s", qPrintable(message));
    }

    return result.result;
}

void AbstractMetaBuilderPrivate::pushScope(const NamespaceModelItem &item)
{
    // For purposes of type lookup, join all namespaces of the same name
    // within the parent item.
    QList<NamespaceModelItem> candidates;
    const QString name = item->name();
    if (!m_scopes.isEmpty()) {
        for (const auto &n : m_scopes.constLast()->namespaces()) {
            if (n->name() == name)
                candidates.append(n);
        }
    }
    if (candidates.size() > 1) {
        NamespaceModelItem joined(new _NamespaceModelItem(m_scopes.constLast()->model(),
                                                          name, _CodeModelItem::Kind_Namespace));
        joined->setScope(item->scope());
        for (const auto &n : candidates)
            joined->appendNamespace(*n);
        m_scopes << joined;
    } else {
        m_scopes << item;
    }
}

AbstractMetaArgumentList AbstractMetaBuilderPrivate::reverseList(const AbstractMetaArgumentList &list)
{
    AbstractMetaArgumentList ret;

    int index = list.size();
    for (const AbstractMetaArgument &a : list) {
        AbstractMetaArgument arg = a;
        arg.setArgumentIndex(index);
        ret.prepend(arg);
        index--;
    }

    return ret;
}

void AbstractMetaBuilder::setGlobalHeaders(const QFileInfoList &globalHeaders)
{
    d->m_globalHeaders = globalHeaders;
}

void AbstractMetaBuilder::setHeaderPaths(const HeaderPaths &hp)
{
    for (const auto & h: hp) {
        if (h.type != HeaderType::Framework && h.type != HeaderType::FrameworkSystem)
            d->m_headerPaths.append(QFile::decodeName(h.path));
    }
}

void AbstractMetaBuilder::setSkipDeprecated(bool value)
{
    d->m_skipDeprecated = value;
}

// PYSIDE-975: When receiving an absolute path name from the code model, try
// to resolve it against the include paths set on shiboken in order to recreate
// relative paths like #include <foo/bar.h>.

static inline bool isFileSystemSlash(QChar c)
{
    return c == QLatin1Char('/') || c == QLatin1Char('\\');
}

static bool matchHeader(const QString &headerPath, const QString &fileName)
{
#if defined(Q_OS_WIN) || defined(Q_OS_DARWIN)
    static const Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
#else
    static const Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive;
#endif
    const int pathSize = headerPath.size();
    return fileName.size() > pathSize
        && isFileSystemSlash(fileName.at(pathSize))
        && fileName.startsWith(headerPath, caseSensitivity);
}

void AbstractMetaBuilderPrivate::setInclude(TypeEntry *te, const QString &path) const
{
    auto it = m_resolveIncludeHash.find(path);
    if (it == m_resolveIncludeHash.end()) {
        QFileInfo info(path);
        const QString fileName = info.fileName();
        if (std::any_of(m_globalHeaders.cbegin(), m_globalHeaders.cend(),
                        [fileName] (const QFileInfo &fi) {
                            return fi.fileName() == fileName; })) {
            return;
        }

        int bestMatchLength = 0;
        for (const auto &headerPath : m_headerPaths) {
            if (headerPath.size() > bestMatchLength && matchHeader(headerPath, path))
                bestMatchLength = headerPath.size();
        }
        const QString include = bestMatchLength > 0
            ? path.right(path.size() - bestMatchLength - 1) : fileName;
        it = m_resolveIncludeHash.insert(path, {Include::IncludePath, include});
    }
    te->setInclude(it.value());
}

#ifndef QT_NO_DEBUG_STREAM
template <class Container>
static void debugFormatSequence(QDebug &d, const char *key, const Container& c,
                                const char *separator = ", ")
{
    if (c.isEmpty())
        return;
    const auto begin = c.begin();
    const auto end = c.end();
    d << "\n  " << key << '[' << c.size() << "]=(";
    for (auto it = begin; it != end; ++it) {
        if (it != begin)
            d << separator;
        d << *it;
    }
    d << ')';
}

void AbstractMetaBuilder::formatDebug(QDebug &debug) const
{
    debug << "m_globalHeader=" << d->m_globalHeaders;
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
