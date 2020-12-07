/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "abstractmetalang.h"
#include "abstractmetalang_helpers.h"
#include "abstractmetaenum.h"
#include "abstractmetafunction.h"
#include "abstractmetafield.h"
#include "documentation.h"
#include "messages.h"
#include "modifications.h"
#include "propertyspec.h"
#include "reporthandler.h"
#include "sourcelocation.h"
#include "typedatabase.h"
#include "typesystem.h"

#include <QtCore/QDebug>

bool function_sorter(const AbstractMetaFunctionCPtr &a, const AbstractMetaFunctionCPtr &b)
{
    return a->signature() < b->signature();
}

class AbstractMetaClassPrivate
{
public:
    AbstractMetaClassPrivate()
        : m_hasVirtuals(false),
          m_isPolymorphic(false),
          m_hasNonpublic(false),
          m_hasNonPrivateConstructor(false),
          m_hasPrivateConstructor(false),
          m_functionsFixed(false),
          m_hasPrivateDestructor(false),
          m_hasProtectedDestructor(false),
          m_hasVirtualDestructor(false),
          m_hasHashFunction(false),
          m_hasEqualsOperator(false),
          m_hasCloneOperator(false),
          m_isTypeDef(false),
          m_hasToStringCapability(false)
    {
    }

    uint m_hasVirtuals : 1;
    uint m_isPolymorphic : 1;
    uint m_hasNonpublic : 1;
    uint m_hasNonPrivateConstructor : 1;
    uint m_hasPrivateConstructor : 1;
    uint m_functionsFixed : 1;
    uint m_hasPrivateDestructor : 1;
    uint m_hasProtectedDestructor : 1;
    uint m_hasVirtualDestructor : 1;
    uint m_hasHashFunction : 1;
    uint m_hasEqualsOperator : 1;
    uint m_hasCloneOperator : 1;
    uint m_isTypeDef : 1;
    uint m_hasToStringCapability : 1;

    Documentation m_doc;

    const AbstractMetaClass *m_enclosingClass = nullptr;
    AbstractMetaClass *m_defaultSuperclass = nullptr;
    AbstractMetaClassList m_baseClasses; // Real base classes after setting up inheritance
    AbstractMetaTypeList m_baseTemplateInstantiations;
    const AbstractMetaClass *m_extendedNamespace = nullptr;

    const AbstractMetaClass *m_templateBaseClass = nullptr;
    AbstractMetaFunctionCList m_functions;
    AbstractMetaFieldList m_fields;
    AbstractMetaEnumList m_enums;
    QList<QPropertySpec> m_propertySpecs;
    AbstractMetaClassList m_innerClasses;

    AbstractMetaFunctionCList m_externalConversionOperators;

    QStringList m_baseClassNames;  // Base class names from C++, including rejected
    TypeEntries m_templateArgs;
    ComplexTypeEntry *m_typeEntry = nullptr;
    SourceLocation m_sourceLocation;

    bool m_stream = false;
    uint m_toStringCapabilityIndirections = 0;
};

AbstractMetaClass::AbstractMetaClass() : d(new AbstractMetaClassPrivate)
{
}

AbstractMetaClass::~AbstractMetaClass() = default;

/*******************************************************************************
 * Returns true if this class is a subclass of the given class
 */
bool AbstractMetaClass::inheritsFrom(const AbstractMetaClass *cls) const
{
    Q_ASSERT(cls);

    const AbstractMetaClass *clazz = this;
    while (clazz) {
        if (clazz == cls)
            return true;

        clazz = clazz->baseClass();
    }

    return false;
}

bool AbstractMetaClass::isPolymorphic() const
{
    return d->m_isPolymorphic;
}

/*******************************************************************************
 * Returns a list of all the functions with a given name
 */
AbstractMetaFunctionCList AbstractMetaClass::queryFunctionsByName(const QString &name) const
{
    AbstractMetaFunctionCList returned;
    for (const auto &function : d->m_functions) {
        if (function->name() == name)
            returned.append(function);
    }

    return returned;
}

/*******************************************************************************
 * Returns a list of all the functions retrieved during parsing which should
 * be added to the API.
 */
AbstractMetaFunctionCList AbstractMetaClass::functionsInTargetLang() const
{
    FunctionQueryOptions default_flags = FunctionQueryOption::NormalFunctions
        | FunctionQueryOption::Visible | FunctionQueryOption::NotRemoved;

    // Only public functions in final classes
    // default_flags |= isFinal() ? WasPublic : 0;
    FunctionQueryOptions public_flags;
    if (isFinalInTargetLang())
        public_flags |= FunctionQueryOption::WasPublic;

    // Constructors
    AbstractMetaFunctionCList returned = queryFunctions(FunctionQueryOption::Constructors
                                                        | default_flags | public_flags);

    // Final functions
    returned += queryFunctions(FunctionQueryOption::FinalInTargetLangFunctions
                               | FunctionQueryOption::NonStaticFunctions
                               | default_flags | public_flags);

    // Virtual functions
    returned += queryFunctions(FunctionQueryOption::VirtualInTargetLangFunctions
                               | FunctionQueryOption::NonStaticFunctions
                               | default_flags | public_flags);

    // Static functions
    returned += queryFunctions(FunctionQueryOption::StaticFunctions
                               | default_flags | public_flags);

    // Empty, private functions, since they aren't caught by the other ones
    returned += queryFunctions(FunctionQueryOption::Empty | FunctionQueryOption::Invisible);

    return returned;
}

AbstractMetaFunctionCList AbstractMetaClass::implicitConversions() const
{
    if (!hasCloneOperator() && !hasExternalConversionOperators())
        return {};

    AbstractMetaFunctionCList returned;
    const auto list = queryFunctions(FunctionQueryOption::Constructors) + externalConversionOperators();

    // Exclude anything that uses rvalue references, be it a move
    // constructor "QPolygon(QPolygon &&)" or something else like
    // "QPolygon(QVector<QPoint> &&)".
    for (const auto &f : list) {
        if ((f->actualMinimumArgumentCount() == 1 || f->arguments().size() == 1 || f->isConversionOperator())
            && !f->isExplicit()
            && f->functionType() != AbstractMetaFunction::CopyConstructorFunction
            && !f->usesRValueReferences()
            && !f->isModifiedRemoved()
            && (f->originalAttributes() & Public)) {
            returned += f;
        }
    }
    return returned;
}

AbstractMetaFunctionCList AbstractMetaClass::operatorOverloads(OperatorQueryOptions query) const
{
    const auto &list = queryFunctions(FunctionQueryOption::OperatorOverloads
                                      | FunctionQueryOption::Visible);
    AbstractMetaFunctionCList returned;
    for (const auto &f : list) {
        if ((query.testFlag(OperatorQueryOption::ArithmeticOp) && f->isArithmeticOperator())
            || (query.testFlag(OperatorQueryOption::BitwiseOp) && f->isBitwiseOperator())
            || (query.testFlag(OperatorQueryOption::ComparisonOp) && f->isComparisonOperator())
            || (query.testFlag(OperatorQueryOption::LogicalOp) && f->isLogicalOperator())
            || (query.testFlag(OperatorQueryOption::SubscriptionOp) && f->isSubscriptOperator())
            || (query.testFlag(OperatorQueryOption::AssignmentOp) && f->isAssignmentOperator())
            || (query.testFlag(OperatorQueryOption::ConversionOp) && f->isConversionOperator())
            || (query.testFlag(OperatorQueryOption::OtherOp) && f->isOtherOperator()))
            returned += f;
    }

    return returned;
}

bool AbstractMetaClass::hasArithmeticOperatorOverload() const
{
    for (const auto & f: d->m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isArithmeticOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasBitwiseOperatorOverload() const
{
    for (const auto & f: d->m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isBitwiseOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasComparisonOperatorOverload() const
{
    for (const auto &f : d->m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isComparisonOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasLogicalOperatorOverload() const
{
    for (const auto &f : d->m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isLogicalOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

const AbstractMetaFieldList &AbstractMetaClass::fields() const
{
    return d->m_fields;
}

AbstractMetaFieldList &AbstractMetaClass::fields()
{
    return d->m_fields;
}

void AbstractMetaClass::setFields(const AbstractMetaFieldList &fields)
{
    d->m_fields = fields;
}

void AbstractMetaClass::addField(const AbstractMetaField &field)
{
    d->m_fields << field;
}

void AbstractMetaClass::sortFunctions()
{
    std::sort(d->m_functions.begin(), d->m_functions.end(), function_sorter);
}

const AbstractMetaClass *AbstractMetaClass::templateBaseClass() const
{
    return d->m_templateBaseClass;
}

void AbstractMetaClass::setTemplateBaseClass(const AbstractMetaClass *cls)
{
    d->m_templateBaseClass = cls;
}

const AbstractMetaFunctionCList &AbstractMetaClass::functions() const
{
    return d->m_functions;
}

void AbstractMetaClass::setFunctions(const AbstractMetaFunctionCList &functions)
{
    d->m_functions = functions;

    // Functions must be sorted by name before next loop
    sortFunctions();

    for (const auto &f : qAsConst(d->m_functions)) {
        qSharedPointerConstCast<AbstractMetaFunction>(f)->setOwnerClass(this);
        if (!f->isPublic())
            d->m_hasNonpublic = true;
    }
}

bool AbstractMetaClass::hasDefaultToStringFunction() const
{
    const auto &funcs = queryFunctionsByName(QLatin1String("toString"));
    for (const auto &f : funcs) {
        if (!f->actualMinimumArgumentCount())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasEqualsOperator() const
{
    return d->m_hasEqualsOperator;
}

void AbstractMetaClass::setHasEqualsOperator(bool on)
{
    d->m_hasEqualsOperator = on;
}

bool AbstractMetaClass::hasCloneOperator() const
{
    return d->m_hasCloneOperator;
}

void AbstractMetaClass::setHasCloneOperator(bool on)
{
    d->m_hasCloneOperator = on;
}

const QList<QPropertySpec> &AbstractMetaClass::propertySpecs() const
{
    return d->m_propertySpecs;
}

void AbstractMetaClass::addPropertySpec(const QPropertySpec &spec)
{
    d->m_propertySpecs << spec;
}

void AbstractMetaClass::addFunction(const AbstractMetaFunctionCPtr &function)
{
    Q_ASSERT(!function->signature().startsWith(QLatin1Char('(')));
    qSharedPointerConstCast<AbstractMetaFunction>(function)->setOwnerClass(this);

    if (!function->isDestructor())
        d->m_functions << function;
    else
        Q_ASSERT(false); //memory leak

    d->m_hasVirtuals |= function->isVirtual();
    d->m_isPolymorphic |= d->m_hasVirtuals;
    d->m_hasNonpublic |= !function->isPublic();
}

bool AbstractMetaClass::hasSignal(const AbstractMetaFunction *other) const
{
    if (!other->isSignal())
        return false;

    for (const auto &f : d->m_functions) {
        if (f->isSignal() && f->compareTo(other) & AbstractMetaFunction::EqualName)
            return other->modifiedName() == f->modifiedName();
    }

    return false;
}


QString AbstractMetaClass::name() const
{
    return d->m_typeEntry->targetLangEntryName();
}

const Documentation &AbstractMetaClass::documentation() const
{
    return d->m_doc;
}

void AbstractMetaClass::setDocumentation(const Documentation &doc)
{
    d->m_doc = doc;
}

QString AbstractMetaClass::baseClassName() const
{
    return d->m_baseClasses.isEmpty() ? QString() : d->m_baseClasses.constFirst()->name();
}

// Attribute "default-superclass"
AbstractMetaClass *AbstractMetaClass::defaultSuperclass() const
{
    return d->m_defaultSuperclass;
}

void AbstractMetaClass::setDefaultSuperclass(AbstractMetaClass *s)
{
    d->m_defaultSuperclass = s;
}

AbstractMetaClass *AbstractMetaClass::baseClass() const
{
    return d->m_baseClasses.value(0, nullptr);
}

const AbstractMetaClassList &AbstractMetaClass::baseClasses() const
{
    return d->m_baseClasses;
}

// base classes including "defaultSuperclass".
AbstractMetaClassList AbstractMetaClass::typeSystemBaseClasses() const
{
    AbstractMetaClassList result = d->m_baseClasses;
    if (d->m_defaultSuperclass != nullptr) {
        result.removeAll(d->m_defaultSuperclass);
        result.prepend(d->m_defaultSuperclass);
    }
    return result;
}

// Recursive list of all base classes including defaultSuperclass
AbstractMetaClassList AbstractMetaClass::allTypeSystemAncestors() const
{
    AbstractMetaClassList result;
    const AbstractMetaClassList baseClasses = typeSystemBaseClasses();
    for (AbstractMetaClass *base : baseClasses) {
        result.append(base);
        result.append(base->allTypeSystemAncestors());
    }
    return result;
}

void AbstractMetaClass::addBaseClass(AbstractMetaClass *baseClass)
{
    Q_ASSERT(baseClass);
    d->m_baseClasses.append(baseClass);
    d->m_isPolymorphic |= baseClass->isPolymorphic();
}

void AbstractMetaClass::setBaseClass(AbstractMetaClass *baseClass)
{
    if (baseClass) {
        d->m_baseClasses.prepend(baseClass);
        d->m_isPolymorphic |= baseClass->isPolymorphic();
    }
}

const AbstractMetaClass *AbstractMetaClass::extendedNamespace() const
{
    return d->m_extendedNamespace;
}

void AbstractMetaClass::setExtendedNamespace(const AbstractMetaClass *e)
{
    d->m_extendedNamespace = e;
}

const AbstractMetaClassList &AbstractMetaClass::innerClasses() const
{
    return d->m_innerClasses;
}

void AbstractMetaClass::addInnerClass(AbstractMetaClass *cl)
{
    d->m_innerClasses << cl;
}

void AbstractMetaClass::setInnerClasses(const AbstractMetaClassList &innerClasses)
{
    d->m_innerClasses = innerClasses;
}

QString AbstractMetaClass::package() const
{
    return d->m_typeEntry->targetLangPackage();
}

bool AbstractMetaClass::isNamespace() const
{
    return d->m_typeEntry->isNamespace();
}

// Is an invisible namespaces whose functions/enums
// should be mapped to the global space.
bool AbstractMetaClass::isInvisibleNamespace() const
{
    return d->m_typeEntry->isNamespace() && d->m_typeEntry->generateCode()
        && !NamespaceTypeEntry::isVisibleScope(d->m_typeEntry);
}

static bool qObjectPredicate(const AbstractMetaClass *c)
{
    return c->qualifiedCppName() == QLatin1String("QObject");
}

bool AbstractMetaClass::isQObject() const
{
    return qObjectPredicate(this) || recurseClassHierarchy(this, qObjectPredicate) != nullptr;
}

bool AbstractMetaClass::isQtNamespace() const
{
    return isNamespace() && name() == QLatin1String("Qt");
}

QString AbstractMetaClass::qualifiedCppName() const
{
    return d->m_typeEntry->qualifiedCppName();
}

bool AbstractMetaClass::hasFunction(const QString &str) const
{
    return !findFunction(str).isNull();
}

AbstractMetaFunctionCPtr AbstractMetaClass::findFunction(const QString &functionName) const
{
    return AbstractMetaFunction::find(d->m_functions, functionName);
}

bool AbstractMetaClass::hasProtectedFunctions() const
{
    for (const auto &func : d->m_functions) {
        if (func->isProtected())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasProtectedFields() const
{
    for (const AbstractMetaField &field : d->m_fields) {
        if (field.isProtected())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasProtectedMembers() const
{
    return hasProtectedFields() || hasProtectedFunctions();
}

const TypeEntries &AbstractMetaClass::templateArguments() const
{
    return d->m_templateArgs;
}

void AbstractMetaClass::setTemplateArguments(const TypeEntries &args)
{
    d->m_templateArgs = args;
}

const QStringList &AbstractMetaClass::baseClassNames() const
{
    return d->m_baseClassNames;
}

void AbstractMetaClass::setBaseClassNames(const QStringList &names)
{
    d->m_baseClassNames = names;
}

const ComplexTypeEntry *AbstractMetaClass::typeEntry() const
{
    return d->m_typeEntry;
}

ComplexTypeEntry *AbstractMetaClass::typeEntry()
{
    return d->m_typeEntry;
}

void AbstractMetaClass::setTypeEntry(ComplexTypeEntry *type)
{
    d->m_typeEntry = type;
}

void AbstractMetaClass::setHasHashFunction(bool on)
{
    d->m_hasHashFunction = on;
}

bool AbstractMetaClass::hasHashFunction() const
{
    return d->m_hasHashFunction;
}

// Search whether a functions is a property setter/getter/reset
AbstractMetaClass::PropertyFunctionSearchResult
    AbstractMetaClass::searchPropertyFunction(const QString &name) const
{
    for (int i = 0, size = d->m_propertySpecs.size(); i < size; ++i) {
        const auto &propertySpec = d->m_propertySpecs.at(i);
        if (name == propertySpec.read())
            return PropertyFunctionSearchResult{i, PropertyFunction::Read};
        if (name == propertySpec.write())
            return PropertyFunctionSearchResult{i, PropertyFunction::Write};
        if (name == propertySpec.reset())
            return PropertyFunctionSearchResult{i, PropertyFunction::Reset};
    }
    return PropertyFunctionSearchResult{-1, PropertyFunction::Read};
}

std::optional<QPropertySpec>
    AbstractMetaClass::propertySpecByName(const QString &name) const
{
    for (const auto &propertySpec : d->m_propertySpecs) {
        if (name == propertySpec.name())
            return propertySpec;
    }
    return {};
}

const AbstractMetaFunctionCList &AbstractMetaClass::externalConversionOperators() const
{
    return d->m_externalConversionOperators;
}

void AbstractMetaClass::addExternalConversionOperator(const AbstractMetaFunctionCPtr &conversionOp)
{
    if (!d->m_externalConversionOperators.contains(conversionOp))
        d->m_externalConversionOperators.append(conversionOp);
}

bool AbstractMetaClass::hasExternalConversionOperators() const
{
    return !d->m_externalConversionOperators.isEmpty();
}

bool AbstractMetaClass::hasTemplateBaseClassInstantiations() const
{
    return d->m_templateBaseClass != nullptr && !d->m_baseTemplateInstantiations.isEmpty();
}

const AbstractMetaTypeList &AbstractMetaClass::templateBaseClassInstantiations() const
{
    return d->m_baseTemplateInstantiations;
}

void AbstractMetaClass::setTemplateBaseClassInstantiations(const AbstractMetaTypeList &instantiations)
{
    Q_ASSERT(d->m_templateBaseClass != nullptr);
    d->m_baseTemplateInstantiations = instantiations;
}

void AbstractMetaClass::setTypeDef(bool typeDef)
{
    d->m_isTypeDef = typeDef;
}

bool AbstractMetaClass::isTypeDef() const
{
    return d->m_isTypeDef;
}

bool AbstractMetaClass::isStream() const
{
    return d->m_stream;
}

void AbstractMetaClass::setStream(bool stream)
{
    d->m_stream = stream;
}

bool AbstractMetaClass::hasToStringCapability() const
{
    return d->m_hasToStringCapability;
}

void AbstractMetaClass::setToStringCapability(bool value, uint indirections)
{
    d->m_hasToStringCapability = value;
    d->m_toStringCapabilityIndirections = indirections;
}

uint AbstractMetaClass::toStringCapabilityIndirections() const
{
    return d->m_toStringCapabilityIndirections;
}

// Does any of the base classes require deletion in the main thread?
bool AbstractMetaClass::deleteInMainThread() const
{
    return typeEntry()->deleteInMainThread()
            || (!d->m_baseClasses.isEmpty() && d->m_baseClasses.constFirst()->deleteInMainThread());
}

bool AbstractMetaClass::hasConstructors() const
{
    return AbstractMetaClass::queryFirstFunction(d->m_functions,
                                                 FunctionQueryOption::Constructors) != nullptr;
}

AbstractMetaFunctionCPtr AbstractMetaClass::copyConstructor() const
{
    for (const auto &f : d->m_functions) {
        if (f->functionType() == AbstractMetaFunction::CopyConstructorFunction)
            return f;
    }
    return {};
}

bool AbstractMetaClass::hasCopyConstructor() const
{
    return copyConstructor() != nullptr;
}

bool AbstractMetaClass::hasPrivateCopyConstructor() const
{
    const auto copyCt = copyConstructor();
    return !copyCt.isNull() && copyCt->isPrivate();
}

void AbstractMetaClass::addDefaultConstructor()
{
    auto *f = new AbstractMetaFunction;
    f->setType(AbstractMetaType::createVoid());
    f->setOriginalName(name());
    f->setName(name());
    f->setOwnerClass(this);
    f->setFunctionType(AbstractMetaFunction::ConstructorFunction);
    f->setArguments(AbstractMetaArgumentList());
    f->setDeclaringClass(this);

    f->setAttributes(Public | FinalInTargetLang | AddedMethod);
    f->setImplementingClass(this);
    f->setOriginalAttributes(f->attributes());

    addFunction(AbstractMetaFunctionCPtr(f));
    this->setHasNonPrivateConstructor(true);
}

void AbstractMetaClass::addDefaultCopyConstructor(bool isPrivate)
{
    auto f = new AbstractMetaFunction;
    f->setType(AbstractMetaType::createVoid());
    f->setOriginalName(name());
    f->setName(name());
    f->setOwnerClass(this);
    f->setFunctionType(AbstractMetaFunction::CopyConstructorFunction);
    f->setDeclaringClass(this);

    AbstractMetaType argType(typeEntry());
    argType.setReferenceType(LValueReference);
    argType.setConstant(true);
    argType.setTypeUsagePattern(AbstractMetaType::ValuePattern);

    AbstractMetaArgument arg;
    arg.setType(argType);
    arg.setName(name());
    f->addArgument(arg);

    AbstractMetaAttributes::Attributes attr = FinalInTargetLang | AddedMethod;
    if (isPrivate)
        attr |= AbstractMetaAttributes::Private;
    else
        attr |= AbstractMetaAttributes::Public;
    f->setAttributes(attr);
    f->setImplementingClass(this);
    f->setOriginalAttributes(f->attributes());

    addFunction(AbstractMetaFunctionCPtr(f));
}

bool AbstractMetaClass::hasNonPrivateConstructor() const
{
    return d->m_hasNonPrivateConstructor;
}

void AbstractMetaClass::setHasNonPrivateConstructor(bool value)
{
    d->m_hasNonPrivateConstructor = value;
}

bool AbstractMetaClass::hasPrivateConstructor() const
{
    return d->m_hasPrivateConstructor;
}

void AbstractMetaClass::setHasPrivateConstructor(bool value)
{
    d->m_hasPrivateConstructor = value;
}

bool AbstractMetaClass::hasPrivateDestructor() const
{
    return d->m_hasPrivateDestructor;
}

void AbstractMetaClass::setHasPrivateDestructor(bool value)
{
    d->m_hasPrivateDestructor = value;
}

bool AbstractMetaClass::hasProtectedDestructor() const
{
    return d->m_hasProtectedDestructor;
}

void AbstractMetaClass::setHasProtectedDestructor(bool value)
{
    d->m_hasProtectedDestructor = value;
}

bool AbstractMetaClass::hasVirtualDestructor() const
{
    return d->m_hasVirtualDestructor;
}

void AbstractMetaClass::setHasVirtualDestructor(bool value)
{
    d->m_hasVirtualDestructor = value;
    if (value)
        d->m_hasVirtuals = d->m_isPolymorphic = 1;
}

bool AbstractMetaClass::isConstructible() const
{
    return (hasNonPrivateConstructor() || !hasPrivateConstructor()) && !hasPrivateDestructor();
}

bool AbstractMetaClass::generateExceptionHandling() const
{
    return queryFirstFunction(d->m_functions, FunctionQueryOption::Visible
                              | FunctionQueryOption::GenerateExceptionHandling) != nullptr;
}
/* Goes through the list of functions and returns a list of all
   functions matching all of the criteria in \a query.
 */

bool AbstractMetaClass::queryFunction(const AbstractMetaFunction *f, FunctionQueryOptions query)
{
    if ((query.testFlag(FunctionQueryOption::NotRemoved))) {
        if (f->isModifiedRemoved())
            return false;
        if (f->isVirtual() && f->isModifiedRemoved(f->declaringClass()))
            return false;
    }

    if (query.testFlag(FunctionQueryOption::Visible) && f->isPrivate())
        return false;

    if (query.testFlag(FunctionQueryOption::VirtualInTargetLangFunctions) && f->isFinalInTargetLang())
        return false;

    if (query.testFlag(FunctionQueryOption::Invisible) && !f->isPrivate())
        return false;

    if (query.testFlag(FunctionQueryOption::Empty) && !f->isEmptyFunction())
        return false;

    if (query.testFlag(FunctionQueryOption::WasPublic) && !f->wasPublic())
        return false;

    if (query.testFlag(FunctionQueryOption::ClassImplements) && f->ownerClass() != f->implementingClass())
        return false;

    if (query.testFlag(FunctionQueryOption::FinalInTargetLangFunctions) && !f->isFinalInTargetLang())
        return false;

    if (query.testFlag(FunctionQueryOption::VirtualInCppFunctions) && !f->isVirtual())
        return false;

    if (query.testFlag(FunctionQueryOption::Signals) && (!f->isSignal()))
        return false;

    if (query.testFlag(FunctionQueryOption::Constructors)
         && (!f->isConstructor() || f->ownerClass() != f->implementingClass())) {
        return false;
    }

    if (!query.testFlag(FunctionQueryOption::Constructors) && f->isConstructor())
        return false;

    // Destructors are never included in the functions of a class currently
    /*
           if ((query & Destructors) && (!f->isDestructor()
                                       || f->ownerClass() != f->implementingClass())
            || f->isDestructor() && (query & Destructors) == 0) {
            return false;
        }*/

    if (query.testFlag(FunctionQueryOption::StaticFunctions) && (!f->isStatic() || f->isSignal()))
        return false;

    if (query.testFlag(FunctionQueryOption::NonStaticFunctions) && (f->isStatic()))
        return false;

    if (query.testFlag(FunctionQueryOption::NormalFunctions) && (f->isSignal()))
        return false;

    if (query.testFlag(FunctionQueryOption::OperatorOverloads) && !f->isOperatorOverload())
        return false;

    if (query.testFlag(FunctionQueryOption::GenerateExceptionHandling) && !f->generateExceptionHandling())
        return false;

    if (query.testFlag(FunctionQueryOption::GetAttroFunction)
        && f->functionType() != AbstractMetaFunction::GetAttroFunction) {
        return false;
    }

    if (query.testFlag(FunctionQueryOption::SetAttroFunction)
        && f->functionType() != AbstractMetaFunction::SetAttroFunction) {
        return false;
    }

    return true;
}

AbstractMetaFunctionCList AbstractMetaClass::queryFunctionList(const AbstractMetaFunctionCList &list,
                                                              FunctionQueryOptions query)
{
    AbstractMetaFunctionCList result;
    for (const auto &f : list) {
        if (queryFunction(f.data(), query))
            result.append(f);
    }
    return result;
}

AbstractMetaFunctionCPtr AbstractMetaClass::queryFirstFunction(const AbstractMetaFunctionCList &list,
                                                               FunctionQueryOptions query)
{
    AbstractMetaFunctionCList result;
    for (const auto &f : list) {
        if (queryFunction(f.data(), query))
            return f;
    }
    return {};
}

AbstractMetaFunctionCList AbstractMetaClass::queryFunctions(FunctionQueryOptions query) const
{
    return AbstractMetaClass::queryFunctionList(d->m_functions, query);
}

bool AbstractMetaClass::hasSignals() const
{
    return queryFirstFunction(d->m_functions,
                              FunctionQueryOption::Signals
                              | FunctionQueryOption::Visible
                              | FunctionQueryOption::NotRemoved) != nullptr;
}

AbstractMetaFunctionCList AbstractMetaClass::cppSignalFunctions() const
{
    return queryFunctions(FunctionQueryOption::Signals
                          | FunctionQueryOption::Visible
                          | FunctionQueryOption::NotRemoved);
}

std::optional<AbstractMetaField>
    AbstractMetaClass::findField(const QString &name) const
{
    return AbstractMetaField::find(d->m_fields, name);
}

const AbstractMetaEnumList &AbstractMetaClass::enums() const
{
    return d->m_enums;
}

AbstractMetaEnumList &AbstractMetaClass::enums()
{
    return d->m_enums;
}

void AbstractMetaClass::setEnums(const AbstractMetaEnumList &enums)
{
    d->m_enums = enums;
}

void AbstractMetaClass::addEnum(const AbstractMetaEnum &e)
{
    d->m_enums << e;
}

std::optional<AbstractMetaEnum>
    AbstractMetaClass::findEnum(const QString &enumName) const
{
    for (const auto &e : d->m_enums) {
        if (e.name() == enumName)
            return e;
    }
    return {};
}

/*!  Recursively searches for the enum value named \a enumValueName in
  this class and its superclasses and interfaces.
*/
std::optional<AbstractMetaEnumValue>
    AbstractMetaClass::findEnumValue(const QString &enumValueName) const
{
    for (const AbstractMetaEnum &e : qAsConst(d->m_enums)) {
        auto v = e.findEnumValue(enumValueName);
        if (v.has_value())
            return v;
    }
    if (baseClass())
        return baseClass()->findEnumValue(enumValueName);

    return {};
}

void AbstractMetaClass::getEnumsToBeGenerated(AbstractMetaEnumList *enumList) const
{
    for (const AbstractMetaEnum &metaEnum : d->m_enums) {
        if (!metaEnum.isPrivate() && metaEnum.typeEntry()->generateCode())
            enumList->append(metaEnum);
    }
}

void AbstractMetaClass::getEnumsFromInvisibleNamespacesToBeGenerated(AbstractMetaEnumList *enumList) const
{
    if (isNamespace()) {
        invisibleNamespaceRecursion([enumList](AbstractMetaClass *c) {
            c->getEnumsToBeGenerated(enumList);
        });
    }
}

void AbstractMetaClass::getFunctionsFromInvisibleNamespacesToBeGenerated(AbstractMetaFunctionCList *funcList) const
{
    if (isNamespace()) {
        invisibleNamespaceRecursion([funcList](AbstractMetaClass *c) {
            funcList->append(c->functions());
        });
    }
}

QString AbstractMetaClass::fullName() const
{
    return package() + QLatin1Char('.') + name();
}

static void addExtraIncludeForType(AbstractMetaClass *metaClass, const AbstractMetaType &type)
{

    Q_ASSERT(metaClass);
    const TypeEntry *entry = type.typeEntry();
    if (entry && entry->isComplex()) {
        const auto *centry = static_cast<const ComplexTypeEntry *>(entry);
        ComplexTypeEntry *class_entry = metaClass->typeEntry();
        if (class_entry && centry->include().isValid())
            class_entry->addExtraInclude(centry->include());
    }

    if (type.hasInstantiations()) {
        for (const AbstractMetaType &instantiation : type.instantiations())
            addExtraIncludeForType(metaClass, instantiation);
    }
}

static void addExtraIncludesForFunction(AbstractMetaClass *metaClass,
                                        const AbstractMetaFunctionCPtr &meta_function)
{
    Q_ASSERT(metaClass);
    Q_ASSERT(meta_function);
    addExtraIncludeForType(metaClass, meta_function->type());

    const AbstractMetaArgumentList &arguments = meta_function->arguments();
    for (const AbstractMetaArgument &argument : arguments)
        addExtraIncludeForType(metaClass, argument.type());
}

void AbstractMetaClass::fixFunctions()
{
    if (d->m_functionsFixed)
        return;

    d->m_functionsFixed = true;

    AbstractMetaFunctionCList funcs = functions();
    AbstractMetaFunctionCList nonRemovedFuncs;
    nonRemovedFuncs.reserve(funcs.size());
    for (auto f : qAsConst(funcs)) {
        // Fishy: Setting up of implementing/declaring/base classes changes
        // the applicable modifications; clear cached ones.
        qSharedPointerConstCast<AbstractMetaFunction>(f)->clearModificationsCache();
        if (!f->isModifiedRemoved())
            nonRemovedFuncs.append(f);
    }

    for (auto superClass : d->m_baseClasses) {
        superClass->fixFunctions();
        // Since we always traverse the complete hierarchy we are only
        // interrested in what each super class implements, not what
        // we may have propagated from their base classes again.
        AbstractMetaFunctionCList superFuncs;
        // Super classes can never be final
        if (superClass->isFinalInTargetLang()) {
            qCWarning(lcShiboken).noquote().nospace()
                << "Final class '" << superClass->name() << "' set to non-final, as it is extended by other classes";
            *superClass -= AbstractMetaAttributes::FinalInTargetLang;
        }
        superFuncs = superClass->queryFunctions(FunctionQueryOption::ClassImplements);
        const auto virtuals = superClass->queryFunctions(FunctionQueryOption::VirtualInCppFunctions);
        superFuncs += virtuals;

        QSet<AbstractMetaFunctionCPtr> funcsToAdd;
        for (const auto &sf : qAsConst(superFuncs)) {
            if (sf->isModifiedRemoved())
                continue;

            // skip functions added in base classes
            if (sf->isUserAdded() && sf->declaringClass() != this)
                continue;

            // we generally don't care about private functions, but we have to get the ones that are
            // virtual in case they override abstract functions.
            bool add = (sf->isNormal() || sf->isSignal() || sf->isEmptyFunction());
            for (const auto &cf : qAsConst(nonRemovedFuncs)) {
                AbstractMetaFunctionPtr f(qSharedPointerConstCast<AbstractMetaFunction>(cf));
                const AbstractMetaFunction::CompareResult cmp = cf->compareTo(sf.data());

                if (cmp & AbstractMetaFunction::EqualModifiedName) {
                    add = false;
                    if (cmp & AbstractMetaFunction::EqualArguments) {
                        // Same function, propegate virtual...
                        if (!(cmp & AbstractMetaFunction::EqualAttributes)) {
                            if (!f->isEmptyFunction()) {
                                if (!sf->isFinalInTargetLang() && f->isFinalInTargetLang()) {
                                    *f -= AbstractMetaAttributes::FinalInTargetLang;
                                }
#if 0
                                if (!f->isFinalInTargetLang() && f->isPrivate()) {
                                    f->setFunctionType(AbstractMetaFunction::EmptyFunction);
                                    f->setVisibility(AbstractMetaAttributes::Protected);
                                    *f += AbstractMetaAttributes::FinalInTargetLang;
                                    qCWarning(lcShiboken).noquote().nospace()
                                        << QStringLiteral("private virtual function '%1' in '%2'")
                                                          .arg(f->signature(), f->implementingClass()->name());
                                }
#endif
                            }
                        }

                        if (f->visibility() != sf->visibility()) {
                            qCWarning(lcShiboken, "%s",
                                      qPrintable(msgFunctionVisibilityModified(this, f.data())));
#if 0
                            // If new visibility is private, we can't
                            // do anything. If it isn't, then we
                            // prefer the parent class's visibility
                            // setting for the function.
                            if (!f->isPrivate() && !sf->isPrivate())
                                f->setVisibility(sf->visibility());
#endif
                            // Private overrides of abstract functions have to go into the class or
                            // the subclasses will not compile as non-abstract classes.
                            // But they don't need to be implemented, since they can never be called.
                            if (f->isPrivate()) {
                                f->setFunctionType(AbstractMetaFunction::EmptyFunction);
                                *f += AbstractMetaAttributes::FinalInTargetLang;
                            }
                        }

                        // Set the class which first declares this function, afawk
                        f->setDeclaringClass(sf->declaringClass());

                        if (sf->isFinalInTargetLang() && !sf->isPrivate() && !f->isPrivate() && !sf->isStatic() && !f->isStatic()) {
                            // Shadowed funcion, need to make base class
                            // function non-virtual
                            if (f->implementingClass() != sf->implementingClass() && f->implementingClass()->inheritsFrom(sf->implementingClass())) {

                                // Check whether the superclass method has been redefined to non-final

                                bool hasNonFinalModifier = false;
                                bool isBaseImplPrivate = false;
                                const FunctionModificationList &mods = sf->modifications(sf->implementingClass());
                                for (const FunctionModification &mod : mods) {
                                    if (mod.isNonFinal()) {
                                        hasNonFinalModifier = true;
                                        break;
                                    }
                                    if (mod.isPrivate()) {
                                        isBaseImplPrivate = true;
                                        break;
                                    }
                                }

                                if (!hasNonFinalModifier && !isBaseImplPrivate) {
                                    qCWarning(lcShiboken, "%s",
                                              qPrintable(msgShadowingFunction(sf.data(), f.data())));
                                }
                            }
                        }

                    }

                    if (cmp & AbstractMetaFunction::EqualDefaultValueOverload) {
                        AbstractMetaArgumentList arguments;
                        if (f->arguments().size() < sf->arguments().size())
                            arguments = sf->arguments();
                        else
                            arguments = f->arguments();
                        //TODO: fix this
                        //for (int i=0; i<arguments.size(); ++i)
                        //    arguments[i]->setDefaultValueExpression("<#>" + QString());
                    }


                    // Otherwise we have function shadowing and we can
                    // skip the thing...
                } else if (cmp & AbstractMetaFunction::EqualName && !sf->isSignal()) {
                    // In the case of function shadowing where the function name has been altered to
                    // avoid conflict, we don't copy in the original.
                    add = false;
                }
            }

            if (add)
                funcsToAdd << sf;
        }

        for (const auto &f : qAsConst(funcsToAdd)) {
            AbstractMetaFunction *copy = f->copy();
            (*copy) += AddedMethod;
            funcs.append(AbstractMetaFunctionCPtr(copy));
        }
    }

    bool hasPrivateConstructors = false;
    bool hasPublicConstructors = false;
    for (const auto &func : qAsConst(funcs)) {
        for (const auto &mod : func->modifications(this)) {
            if (mod.isRenameModifier())
                qSharedPointerConstCast<AbstractMetaFunction>(func)->setName(mod.renamedToName());
        }

        // Make sure class is abstract if one of the functions is
        if (func->isAbstract()) {
            (*this) += AbstractMetaAttributes::Abstract;
            (*this) -= AbstractMetaAttributes::FinalInTargetLang;
        }

        if (func->isConstructor()) {
            if (func->isPrivate())
                hasPrivateConstructors = true;
            else
                hasPublicConstructors = true;
        }



        // Make sure that we include files for all classes that are in use
        addExtraIncludesForFunction(this, func);
    }

    if (hasPrivateConstructors && !hasPublicConstructors) {
        (*this) += AbstractMetaAttributes::Abstract;
        (*this) -= AbstractMetaAttributes::FinalInTargetLang;
    }

    setFunctions(funcs);
}

/*******************************************************************************
 * Other stuff...
 */


std::optional<AbstractMetaEnum>
    AbstractMetaClass::findEnum(const AbstractMetaClassList &classes,
                                const EnumTypeEntry *entry)
{
    Q_ASSERT(entry->isEnum());

    auto scopeEntry = entry->parent();
    AbstractMetaClass *metaClass = AbstractMetaClass::findClass(classes, scopeEntry);
    if (!metaClass) {
        qCWarning(lcShiboken, "%s", qPrintable(msgClassOfEnumNotFound(entry)));
        return {};
    }

    QString qualifiedName = entry->qualifiedCppName();
    const int pos = qualifiedName.lastIndexOf(QLatin1String("::"));
    const QString enumName = pos > 0 ? qualifiedName.mid(pos + 2) : qualifiedName;
    return metaClass->findEnum(enumName);
}

std::optional<AbstractMetaEnumValue>
    AbstractMetaClass::findEnumValue(const AbstractMetaClassList &classes,
                                     const QString &name)
{
    const auto lst = QStringView{name}.split(u"::");

    if (lst.size() > 1) {
        const auto &prefixName = lst.at(0);
        const auto &enumName = lst.at(1);
        if (AbstractMetaClass *cl = findClass(classes, prefixName.toString()))
            return cl->findEnumValue(enumName.toString());
    }

    for (AbstractMetaClass *metaClass : classes) {
        auto enumValue = metaClass->findEnumValue(name);
        if (enumValue.has_value())
            return enumValue;
    }

    qCWarning(lcShiboken, "no matching enum '%s'", qPrintable(name));
    return {};
}

/*!
 * Searches the list after a class that mathces \a name; either as
 * C++, Target language base name or complete Target language package.class name.
 */

AbstractMetaClass *AbstractMetaClass::findClass(const AbstractMetaClassList &classes,
                                                const QString &name)
{
    if (name.isEmpty())
        return nullptr;

    for (AbstractMetaClass *c : classes) {
        if (c->qualifiedCppName() == name)
            return c;
    }

    for (AbstractMetaClass *c : classes) {
        if (c->fullName() == name)
            return c;
    }

    for (AbstractMetaClass *c : classes) {
        if (c->name() == name)
            return c;
    }

    return nullptr;
}

AbstractMetaClass *AbstractMetaClass::findClass(const AbstractMetaClassList &classes,
                                                const TypeEntry *typeEntry)
{
    for (AbstractMetaClass *c : classes) {
        if (c->typeEntry() == typeEntry)
            return c;
    }
    return nullptr;
}

// Query functions for generators
bool AbstractMetaClass::isObjectType() const
{
    return d->m_typeEntry->isObject();
}

bool AbstractMetaClass::isCopyable() const
{
    if (isNamespace() || d->m_typeEntry->isObject())
        return false;
    auto copyable = d->m_typeEntry->copyable();
    return copyable == ComplexTypeEntry::CopyableSet
        || (copyable == ComplexTypeEntry::Unknown && hasCloneOperator());
}

bool AbstractMetaClass::isValueTypeWithCopyConstructorOnly() const
{
    if (!typeEntry()->isValue())
        return false;
    if (attributes().testFlag(AbstractMetaAttributes::HasRejectedDefaultConstructor))
        return false;
    const auto ctors = queryFunctions(FunctionQueryOption::Constructors);
    bool copyConstructorFound = false;
    for (auto ctor : ctors) {
        switch (ctor->functionType()) {
        case AbstractMetaFunction::ConstructorFunction:
            return false;
        case AbstractMetaFunction::CopyConstructorFunction:
            copyConstructorFound = true;
            break;
        case AbstractMetaFunction::MoveConstructorFunction:
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
    return copyConstructorFound;
}

#ifndef QT_NO_DEBUG_STREAM

void AbstractMetaClass::format(QDebug &debug) const
{
    if (debug.verbosity() > 2)
        debug << static_cast<const void *>(this) << ", ";
    debug << '"' << qualifiedCppName();
    if (const int count = d->m_templateArgs.size()) {
        for (int i = 0; i < count; ++i)
            debug << (i ? ',' : '<') << d->m_templateArgs.at(i)->qualifiedCppName();
        debug << '>';
    }
    debug << '"';
    if (isNamespace())
        debug << " [namespace]";
    if (attributes() & AbstractMetaAttributes::FinalCppClass)
        debug << " [final]";
    if (attributes().testFlag(AbstractMetaAttributes::Deprecated))
        debug << " [deprecated]";
    if (!d->m_baseClasses.isEmpty()) {
        debug << ", inherits ";
        for (auto b : d->m_baseClasses)
            debug << " \"" << b->name() << '"';
    }
    if (auto templateBase = templateBaseClass()) {
        const auto &instantiatedTypes = templateBaseClassInstantiations();
        debug << ", instantiates \"" << templateBase->name();
        for (int i = 0, count = instantiatedTypes.size(); i < count; ++i)
            debug << (i ? ',' : '<') << instantiatedTypes.at(i).name();
        debug << ">\"";
    }
    if (const int count = d->m_propertySpecs.size()) {
        debug << ", properties (" << count << "): [";
        for (int i = 0; i < count; ++i) {
            if (i)
                debug << ", ";
            d->m_propertySpecs.at(i).formatDebug(debug);
        }
        debug << ']';
    }
}

void AbstractMetaClass::formatMembers(QDebug &debug) const
{
    if (!d->m_enums.isEmpty())
        debug << ", enums[" << d->m_enums.size() << "]=" << d->m_enums;
    if (!d->m_functions.isEmpty()) {
        const int count = d->m_functions.size();
        debug << ", functions=[" << count << "](";
        for (int i = 0; i < count; ++i) {
            if (i)
                debug << ", ";
            d->m_functions.at(i)->formatDebugBrief(debug);
        }
        debug << ')';
    }
    if (const int count = d->m_fields.size()) {
        debug << ", fields=[" << count << "](";
        for (int i = 0; i < count; ++i) {
            if (i)
                debug << ", ";
            d->m_fields.at(i).formatDebug(debug);
        }
        debug << ')';
    }
}

SourceLocation AbstractMetaClass::sourceLocation() const
{
    return d->m_sourceLocation;
}

void AbstractMetaClass::setSourceLocation(const SourceLocation &sourceLocation)
{
    d->m_sourceLocation = sourceLocation;
}

QDebug operator<<(QDebug d, const AbstractMetaClass *ac)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaClass(";
    if (ac) {
        ac->format(d);
        if (d.verbosity() > 2)
            ac->formatMembers(d);
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
