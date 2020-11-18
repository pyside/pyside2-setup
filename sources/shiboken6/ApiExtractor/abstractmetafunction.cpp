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

#include "abstractmetafunction.h"
#include "abstractmetalang.h"
#include "abstractmetalang_helpers.h"
#include "abstractmetatype.h"
#include "documentation.h"
#include "messages.h"
#include "modifications.h"
#include "propertyspec.h"
#include "reporthandler.h"
#include "sourcelocation.h"
#include "typedatabase.h"
#include "typesystem.h"

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>

// Cache FunctionModificationList in a flat list per class (0 for global
// functions, or typically owner/implementing/declaring class.
struct ModificationCacheEntry
{
     const AbstractMetaClass *klass;
     FunctionModificationList modifications;
};

using ModificationCache = QList<ModificationCacheEntry>;

class AbstractMetaFunctionPrivate
{
public:
    AbstractMetaFunctionPrivate()
        : m_constant(false),
          m_reverse(false),
          m_explicit(false),
          m_pointerOperator(false),
          m_isCallOperator(false)
    {
    }

    QString signature() const;
    QString minimalSignature() const;
    QString modifiedName(const AbstractMetaFunction *q) const;
    int overloadNumber(const AbstractMetaFunction *q) const;

    const FunctionModificationList &modifications(const AbstractMetaFunction *q,
                                                  const AbstractMetaClass *implementor) const;

    QString m_name;
    QString m_originalName;
    Documentation m_doc;
    mutable QString m_cachedMinimalSignature;
    mutable QString m_cachedSignature;
    mutable QString m_cachedModifiedName;

    FunctionTypeEntry* m_typeEntry = nullptr;
    AbstractMetaFunction::FunctionType m_functionType = AbstractMetaFunction::NormalFunction;
    AbstractMetaType m_type;
    const AbstractMetaClass *m_class = nullptr;
    const AbstractMetaClass *m_implementingClass = nullptr;
    const AbstractMetaClass *m_declaringClass = nullptr;
    mutable ModificationCache m_modificationCache;
    int m_propertySpecIndex = -1;
    AbstractMetaArgumentList m_arguments;
    AddedFunctionPtr m_addedFunction;
    SourceLocation m_sourceLocation;
    uint m_constant                 : 1;
    uint m_reverse                  : 1;
    uint m_explicit                 : 1;
    uint m_pointerOperator          : 1;
    uint m_isCallOperator           : 1;
    mutable int m_cachedOverloadNumber = TypeSystem::OverloadNumberUnset;
    ExceptionSpecification m_exceptionSpecification = ExceptionSpecification::Unknown;
    TypeSystem::AllowThread m_allowThreadModification = TypeSystem::AllowThread::Unspecified;
    TypeSystem::ExceptionHandling m_exceptionHandlingModification = TypeSystem::ExceptionHandling::Unspecified;
};

AbstractMetaFunction::AbstractMetaFunction(const AddedFunctionPtr &addedFunc) :
    AbstractMetaFunction()
{
    d->m_addedFunction = addedFunc;
    setConstant(addedFunc->isConstant());
    setName(addedFunc->name());
    setOriginalName(addedFunc->name());
    auto atts = attributes() | AbstractMetaAttributes::FinalInTargetLang;
    switch (addedFunc->access()) {
    case AddedFunction::InvalidAccess:
        break;
    case AddedFunction::Protected:
        atts |= AbstractMetaAttributes::Protected;
        break;
    case AddedFunction::Public:
        atts |= AbstractMetaAttributes::Public;
        break;
    }
    if (addedFunc->isStatic())
        atts |= AbstractMetaFunction::Static;
    setAttributes(atts);
}

QString AbstractMetaFunction::name() const
{
    return d->m_name;
}

void AbstractMetaFunction::setName(const QString &name)
{
    d->m_name = name;
}

QString AbstractMetaFunction::originalName() const
{
    return d->m_originalName.isEmpty() ? name() : d->m_originalName;
}

void AbstractMetaFunction::setOriginalName(const QString &name)
{
    d->m_originalName = name;
}

const Documentation &AbstractMetaFunction::documentation() const
{
    return d->m_doc;
}

void AbstractMetaFunction::setDocumentation(const Documentation &doc)
{
    d->m_doc = doc;
}

bool AbstractMetaFunction::isReverseOperator() const
{
    return d->m_reverse;
}

void AbstractMetaFunction::setReverseOperator(bool reverse)
{
    d->m_reverse = reverse;
}

bool AbstractMetaFunction::isPointerOperator() const
{
    return d->m_pointerOperator;
}

void AbstractMetaFunction::setPointerOperator(bool value)
{
    d->m_pointerOperator = value;
}

bool AbstractMetaFunction::isExplicit() const
{
    return d->m_explicit;
}

void AbstractMetaFunction::setExplicit(bool isExplicit)
{
    d->m_explicit = isExplicit;
}

AbstractMetaFunction::AbstractMetaFunction() : d(new AbstractMetaFunctionPrivate)
{
}

AbstractMetaFunction::~AbstractMetaFunction() = default;

/*******************************************************************************
 * Indicates that this function has a modification that removes it
 */
bool AbstractMetaFunction::isModifiedRemoved(int types) const
{
    for (const auto &mod : modifications(implementingClass())) {
        if (!mod.isRemoveModifier())
            continue;

        if ((mod.removal() & types) == types)
            return true;
    }

    return false;
}

bool AbstractMetaFunction::isVoid() const
{
    return d->m_type.isVoid();
}

const AbstractMetaType &AbstractMetaFunction::type() const
{
    return d->m_type;
}

void AbstractMetaFunction::setType(const AbstractMetaType &type)
{
    d->m_type = type;
}

const AbstractMetaClass *AbstractMetaFunction::ownerClass() const
{
    return d->m_class;
}

void AbstractMetaFunction::setOwnerClass(const AbstractMetaClass *cls)
{
    d->m_class = cls;
}

bool AbstractMetaFunction::operator<(const AbstractMetaFunction &other) const
{
    return compareTo(&other) & NameLessThan;
}


/*!
    Returns a mask of CompareResult describing how this function is
    compares to another function
*/
AbstractMetaFunction::CompareResult AbstractMetaFunction::compareTo(const AbstractMetaFunction *other) const
{
    CompareResult result;

    // Enclosing class...
    if (ownerClass() == other->ownerClass())
        result |= EqualImplementor;

    // Attributes
    if (attributes() == other->attributes())
        result |= EqualAttributes;

    // Compare types
    if (type().name() == other->type().name())
        result |= EqualReturnType;

    // Compare names
    int cmp = originalName().compare(other->originalName());

    if (cmp < 0)
        result |= NameLessThan;
    else if (!cmp)
        result |= EqualName;

    // compare name after modification...
    cmp = modifiedName().compare(other->modifiedName());
    if (!cmp)
        result |= EqualModifiedName;

    // Compare arguments...
    AbstractMetaArgumentList minArguments;
    AbstractMetaArgumentList maxArguments;
    if (arguments().size() < other->arguments().size()) {
        minArguments = arguments();
        maxArguments = other->arguments();
    } else {
        minArguments = other->arguments();
        maxArguments = arguments();
    }

    int minCount = minArguments.size();
    int maxCount = maxArguments.size();
    bool same = true;
    for (int i = 0; i < maxCount; ++i) {
        if (i < minCount) {
            const AbstractMetaArgument &min_arg = minArguments.at(i);
            const AbstractMetaArgument &max_arg = maxArguments.at(i);
            if (min_arg.type().name() != max_arg.type().name()
                && (min_arg.defaultValueExpression().isEmpty() || max_arg.defaultValueExpression().isEmpty())) {
                same = false;
                break;
            }
        } else {
            if (maxArguments.at(i).defaultValueExpression().isEmpty()) {
                same = false;
                break;
            }
        }
    }

    if (same)
        result |= minCount == maxCount ? EqualArguments : EqualDefaultValueOverload;

    return result;
}

AbstractMetaFunction *AbstractMetaFunction::copy() const
{
    auto *cpy = new AbstractMetaFunction;
    cpy->assignMetaAttributes(*this);
    cpy->setName(name());
    cpy->setOriginalName(originalName());
    cpy->setOwnerClass(ownerClass());
    cpy->setImplementingClass(implementingClass());
    cpy->setFunctionType(functionType());
    cpy->setDeclaringClass(declaringClass());
    cpy->setType(type());
    cpy->setConstant(isConstant());
    cpy->setExceptionSpecification(d->m_exceptionSpecification);
    cpy->setAllowThreadModification(d->m_allowThreadModification);
    cpy->setExceptionHandlingModification(d->m_exceptionHandlingModification);
    cpy->d->m_addedFunction = d->m_addedFunction;
    cpy->d->m_arguments = d->m_arguments;

    return cpy;
}

bool AbstractMetaFunction::usesRValueReferences() const
{
    if (d->m_functionType == MoveConstructorFunction || d->m_functionType == MoveAssignmentOperatorFunction)
        return true;
    if (d->m_type.referenceType() == RValueReference)
        return true;
    for (const AbstractMetaArgument &a : d->m_arguments) {
        if (a.type().referenceType() == RValueReference)
            return true;
    }
    return false;
}

QStringList AbstractMetaFunction::introspectionCompatibleSignatures(const QStringList &resolvedArguments) const
{
    AbstractMetaArgumentList arguments = this->arguments();
    if (arguments.size() == resolvedArguments.size()) {
        QString signature = name() + QLatin1Char('(') + resolvedArguments.join(QLatin1Char(',')) + QLatin1Char(')');
        return QStringList(TypeDatabase::normalizedSignature(signature));
    }
    QStringList returned;

    const AbstractMetaArgument &argument = arguments.at(resolvedArguments.size());
    QStringList minimalTypeSignature = argument.type().minimalSignature().split(QLatin1String("::"));
    for (int i = 0; i < minimalTypeSignature.size(); ++i) {
        returned += introspectionCompatibleSignatures(QStringList(resolvedArguments)
                                                      << QStringList(minimalTypeSignature.mid(minimalTypeSignature.size() - i - 1)).join(QLatin1String("::")));
    }

    return returned;
}

QString AbstractMetaFunctionPrivate::signature() const
{
    if (m_cachedSignature.isEmpty()) {
        m_cachedSignature = m_originalName;

        m_cachedSignature += QLatin1Char('(');

        for (int i = 0; i < m_arguments.count(); ++i) {
            const AbstractMetaArgument &a = m_arguments.at(i);
            const AbstractMetaType &t = a.type();
            if (i > 0)
                m_cachedSignature += QLatin1String(", ");
            m_cachedSignature += t.cppSignature();
            // We need to have the argument names in the qdoc files
            m_cachedSignature += QLatin1Char(' ');
            m_cachedSignature += a.name();
        }
        m_cachedSignature += QLatin1Char(')');

        if (m_constant)
            m_cachedSignature += QLatin1String(" const");
    }
    return m_cachedSignature;
}

QString AbstractMetaFunction::signature() const
{
    return d->signature();
}

bool AbstractMetaFunction::isConstant() const
{
    return d->m_constant;
}

void AbstractMetaFunction::setConstant(bool constant)
{
    d->m_constant = constant;
}

bool AbstractMetaFunction::isUserAdded() const
{
    return !d->m_addedFunction.isNull() && !d->m_addedFunction->isDeclaration();
}

bool AbstractMetaFunction::isUserDeclared() const
{
    return !d->m_addedFunction.isNull() && d->m_addedFunction->isDeclaration();
}

int AbstractMetaFunction::actualMinimumArgumentCount() const
{
    AbstractMetaArgumentList arguments = this->arguments();

    int count = 0;
    for (int i = 0; i < arguments.size(); ++i && ++count) {
        if (argumentRemoved(i + 1))
            --count;
        else if (!arguments.at(i).defaultValueExpression().isEmpty())
            break;
    }

    return count;
}

// Returns reference counts for argument at idx, or all arguments if idx == -2
QVector<ReferenceCount> AbstractMetaFunction::referenceCounts(const AbstractMetaClass *cls, int idx) const
{
    QVector<ReferenceCount> returned;

    for (const auto &mod : modifications(cls)) {
        for (const ArgumentModification &argumentMod : mod.argument_mods()) {
            if (argumentMod.index != idx && idx != -2)
                continue;
            returned += argumentMod.referenceCounts;
        }
    }

    return returned;
}

ArgumentOwner AbstractMetaFunction::argumentOwner(const AbstractMetaClass *cls, int idx) const
{
    for (const auto &mod : modifications(cls)) {
        for (const ArgumentModification &argumentMod : mod.argument_mods()) {
            if (argumentMod.index != idx)
                continue;
            return argumentMod.owner;
        }
    }
    return ArgumentOwner();
}

QString AbstractMetaFunction::conversionRule(TypeSystem::Language language, int key) const
{
    for (const auto &modification : modifications(declaringClass())) {
        for (const ArgumentModification &argumentModification : modification.argument_mods()) {
            if (argumentModification.index != key)
                continue;

            for (const CodeSnip &snip : argumentModification.conversion_rules) {
                if (snip.language == language && !snip.code().isEmpty())
                    return snip.code();
            }
        }
    }

    return QString();
}

// FIXME If we remove a arg. in the method at the base class, it will not reflect here.
bool AbstractMetaFunction::argumentRemoved(int key) const
{
    for (const auto &modification : modifications(declaringClass())) {
        for (const ArgumentModification &argumentModification : modification.argument_mods()) {
            if (argumentModification.index == key) {
                if (argumentModification.removed)
                    return true;
            }
        }
    }

    return false;
}

const AbstractMetaClass *AbstractMetaFunction::targetLangOwner() const
{
    return d->m_class && d->m_class->isInvisibleNamespace()
        ?  d->m_class->targetLangEnclosingClass() : d->m_class;
}

const AbstractMetaClass *AbstractMetaFunction::declaringClass() const
{
    return d->m_declaringClass;
}

void AbstractMetaFunction::setDeclaringClass(const AbstractMetaClass *cls)
{
    d->m_declaringClass = cls;
}

const AbstractMetaClass *AbstractMetaFunction::implementingClass() const
{
    return d->m_implementingClass;
}

void AbstractMetaFunction::setImplementingClass(const AbstractMetaClass *cls)
{
    d->m_implementingClass = cls;
}

const AbstractMetaArgumentList &AbstractMetaFunction::arguments() const
{
    return d->m_arguments;
}

AbstractMetaArgumentList &AbstractMetaFunction::arguments()
{
    return d->m_arguments;
}

void AbstractMetaFunction::setArguments(const AbstractMetaArgumentList &arguments)
{
    d->m_arguments = arguments;
}

void AbstractMetaFunction::addArgument(const AbstractMetaArgument &argument)
{
    d->m_arguments << argument;
}

bool AbstractMetaFunction::isDeprecated() const
{
    for (const auto &modification : modifications(declaringClass())) {
        if (modification.isDeprecated())
            return true;
    }
    return false;
}

bool AbstractMetaFunction::isConstructor() const
{
    return d->m_functionType == ConstructorFunction || d->m_functionType == CopyConstructorFunction
            || d->m_functionType == MoveConstructorFunction;
}

bool AbstractMetaFunction::isNormal() const
{
    return functionType() == NormalFunction || isSlot() || isInGlobalScope();
}

AbstractMetaFunction::FunctionType AbstractMetaFunction::functionType() const
{
    return d->m_functionType;
}

void AbstractMetaFunction::setFunctionType(AbstractMetaFunction::FunctionType type)
{
    d->m_functionType = type;
}

// Auto-detect whether a function should be wrapped into
// Py_BEGIN_ALLOW_THREADS/Py_END_ALLOW_THREADS, that is, temporarily release
// the GIL (global interpreter lock). Doing so is required for any thread-wait
// functions, anything that might call a virtual function (potentially
// reimplemented in Python), and recommended for lengthy I/O or similar.
// It has performance costs, though.
bool AbstractMetaFunction::autoDetectAllowThread() const
{
    // Disallow for simple getter functions.
    const bool maybeGetter = d->m_constant != 0 && !isVoid() && d->m_arguments.isEmpty();
    return !maybeGetter;
}

SourceLocation AbstractMetaFunction::sourceLocation() const
{
    return d->m_sourceLocation;
}

void AbstractMetaFunction::setSourceLocation(const SourceLocation &sourceLocation)
{
    d->m_sourceLocation = sourceLocation;
}

static inline TypeSystem::AllowThread allowThreadMod(const AbstractMetaClass *klass)
{
    return klass->typeEntry()->allowThread();
}

static inline bool hasAllowThreadMod(const AbstractMetaClass *klass)
{
    return allowThreadMod(klass) != TypeSystem::AllowThread::Unspecified;
}

bool AbstractMetaFunction::allowThread() const
{
    auto allowThreadModification = d->m_allowThreadModification;
    // If there is no modification on the function, check for a base class.
    if (d->m_class && allowThreadModification == TypeSystem::AllowThread::Unspecified) {
        if (auto base = recurseClassHierarchy(d->m_class, hasAllowThreadMod))
            allowThreadModification = allowThreadMod(base);
    }

    bool result = true;
    switch (allowThreadModification) {
    case TypeSystem::AllowThread::Disallow:
        result = false;
        break;
    case TypeSystem::AllowThread::Allow:
        break;
    case TypeSystem::AllowThread::Auto:
        result = autoDetectAllowThread();
        break;
    case TypeSystem::AllowThread::Unspecified:
        result = false;
        break;
    }
    if (!result && ReportHandler::isDebug(ReportHandler::MediumDebug))
        qCInfo(lcShiboken).noquote() << msgDisallowThread(this);
    return result;
}

TypeSystem::Ownership AbstractMetaFunction::ownership(const AbstractMetaClass *cls, TypeSystem::Language language, int key) const
{
    for (const auto &modification : modifications(cls)) {
        for (const ArgumentModification &argumentModification : modification.argument_mods()) {
            if (argumentModification.index == key)
                return argumentModification.ownerships.value(language, TypeSystem::InvalidOwnership);
        }
    }

    return TypeSystem::InvalidOwnership;
}

bool AbstractMetaFunction::isRemovedFromAllLanguages(const AbstractMetaClass *cls) const
{
    return isRemovedFrom(cls, TypeSystem::All);
}

bool AbstractMetaFunction::isRemovedFrom(const AbstractMetaClass *cls, TypeSystem::Language language) const
{
    for (const auto &modification : modifications(cls)) {
        if ((modification.removal() & language) == language)
            return true;
    }

    return false;
}

QString AbstractMetaFunction::typeReplaced(int key) const
{
    for (const auto &modification : modifications(declaringClass())) {
        for (const ArgumentModification &argumentModification : modification.argument_mods()) {
            if (argumentModification.index == key
                && !argumentModification.modified_type.isEmpty()) {
                return argumentModification.modified_type;
            }
        }
    }

    return QString();
}

bool AbstractMetaFunction::isModifiedToArray(int argumentIndex) const
{
    for (const auto &modification : modifications(declaringClass())) {
        for (const ArgumentModification &argumentModification : modification.argument_mods()) {
            if (argumentModification.index == argumentIndex && argumentModification.array != 0)
                return true;
        }
    }
    return false;
}

QString AbstractMetaFunctionPrivate::minimalSignature() const
{
    if (!m_cachedMinimalSignature.isEmpty())
        return m_cachedMinimalSignature;

    QString minimalSignature = m_originalName + QLatin1Char('(');
    for (int i = 0; i < m_arguments.count(); ++i) {
        const AbstractMetaType &t = m_arguments.at(i).type();
        if (i > 0)
            minimalSignature += QLatin1Char(',');
        minimalSignature += t.minimalSignature();
    }
    minimalSignature += QLatin1Char(')');
    if (m_constant)
        minimalSignature += QLatin1String("const");

    minimalSignature = TypeDatabase::normalizedSignature(minimalSignature);
    m_cachedMinimalSignature = minimalSignature;

    return minimalSignature;
}

QString AbstractMetaFunction::minimalSignature() const
{
    return d->minimalSignature();
}

QString AbstractMetaFunction::debugSignature() const
{
    QString result;
    const bool isOverride = attributes() & AbstractMetaFunction::OverriddenCppMethod;
    const bool isFinal = attributes() & AbstractMetaFunction::FinalCppMethod;
    if (!isOverride && !isFinal && (attributes() & AbstractMetaFunction::VirtualCppMethod))
        result += QLatin1String("virtual ");
    result += minimalSignature();
    if (isOverride)
        result += QLatin1String(" override");
    if (isFinal)
        result += QLatin1String(" final");
    return result;
}

FunctionModificationList AbstractMetaFunction::findClassModifications(const AbstractMetaFunction *f,
                                                                      const AbstractMetaClass *implementor)
{
    const QString signature = f->minimalSignature();
    FunctionModificationList mods;
    while (implementor) {
        mods += implementor->typeEntry()->functionModifications(signature);
        if ((implementor == implementor->baseClass()) ||
            (implementor == f->implementingClass() && !mods.isEmpty())) {
                break;
        }
        implementor = implementor->baseClass();
    }
    return mods;
}

FunctionModificationList AbstractMetaFunction::findGlobalModifications(const AbstractMetaFunction *f)
{
    return TypeDatabase::instance()->functionModifications(f->minimalSignature());
}

const FunctionModificationList &
    AbstractMetaFunctionPrivate::modifications(const AbstractMetaFunction *q,
                                               const AbstractMetaClass *implementor) const
{
    if (!m_addedFunction.isNull())
        return m_addedFunction->modifications;
    for (const auto &ce : m_modificationCache) {
        if (ce.klass == implementor)
            return ce.modifications;
    }
    auto modifications = m_class == nullptr
        ? AbstractMetaFunction::findGlobalModifications(q)
        : AbstractMetaFunction::findClassModifications(q, implementor);

    m_modificationCache.append({implementor, modifications});
    return m_modificationCache.constLast().modifications;
}

const FunctionModificationList &
    AbstractMetaFunction::modifications(const AbstractMetaClass *implementor) const
{
    if (implementor == nullptr)
        implementor = d->m_class;
    return d->modifications(this, implementor);
}

void AbstractMetaFunction::clearModificationsCache()
{
    d->m_modificationCache.clear();
}

QString AbstractMetaFunction::argumentName(int index,
                                           bool /* create */,
                                           const AbstractMetaClass * /* implementor */) const
{
    return d->m_arguments[--index].name();
}

int AbstractMetaFunction::propertySpecIndex() const
{
    return d->m_propertySpecIndex;
}

void AbstractMetaFunction::setPropertySpecIndex(int i)
{
    d->m_propertySpecIndex = i;
}

FunctionTypeEntry *AbstractMetaFunction::typeEntry() const
{
    return d->m_typeEntry;
}

void AbstractMetaFunction::setTypeEntry(FunctionTypeEntry *typeEntry)
{
    d->m_typeEntry = typeEntry;
}

bool AbstractMetaFunction::isCallOperator() const
{
    return d->m_name == QLatin1String("operator()");
}

bool AbstractMetaFunction::hasInjectedCode() const
{
    const FunctionModificationList &mods = modifications(ownerClass());
    for (const FunctionModification &mod : mods) {
        if (mod.isCodeInjection())
            return true;
    }
    return false;
}

// Traverse the code snippets, return true if predicate returns true
template <class Predicate>
bool AbstractMetaFunction::traverseCodeSnips(Predicate predicate,
                                             TypeSystem::CodeSnipPosition position,
                                             TypeSystem::Language language) const
{
    for (const FunctionModification &mod : modifications(ownerClass())) {
        if (mod.isCodeInjection()) {
            for (const CodeSnip &snip : mod.snips()) {
                if ((snip.language & language) != 0
                    && (snip.position == position || position == TypeSystem::CodeSnipPositionAny)
                    && predicate(snip)) {
                    return true;
                }
            }
        }
    }
    return false;
}

CodeSnipList AbstractMetaFunction::injectedCodeSnips(TypeSystem::CodeSnipPosition position,
                                                     TypeSystem::Language language) const
{
    CodeSnipList result;
    traverseCodeSnips([&result] (const CodeSnip &s) {
                           result.append(s);
                           return false;
                      }, position, language);
    return result;
}

bool AbstractMetaFunction::injectedCodeContains(const QRegularExpression &pattern,
                                                TypeSystem::CodeSnipPosition position,
                                                TypeSystem::Language language) const
{
    return traverseCodeSnips([pattern] (const CodeSnip &s) {
                                 return s.code().contains(pattern);
                             }, position, language);
}

bool AbstractMetaFunction::injectedCodeContains(QStringView pattern,
                                                TypeSystem::CodeSnipPosition position,
                                                TypeSystem::Language language) const
{
    return traverseCodeSnips([pattern] (const CodeSnip &s) {
                                 return s.code().contains(pattern);
                             }, position, language);
}

bool AbstractMetaFunction::hasSignatureModifications() const
{
    const FunctionModificationList &mods = modifications();
    for (const FunctionModification &mod : mods) {
        if (mod.isRenameModifier())
            return true;
        for (const ArgumentModification &argmod : mod.argument_mods()) {
            // since zero represents the return type and we're
            // interested only in checking the function arguments,
            // it will be ignored.
            if (argmod.index > 0)
                return true;
        }
    }
    return false;
}

bool AbstractMetaFunction::isConversionOperator(const QString &funcName)
{
    static const QRegularExpression opRegEx(QStringLiteral("^operator(?:\\s+(?:const|volatile))?\\s+(\\w+\\s*)&?$"));
    Q_ASSERT(opRegEx.isValid());
    return opRegEx.match(funcName).hasMatch();
}

ExceptionSpecification AbstractMetaFunction::exceptionSpecification() const
{
    return d->m_exceptionSpecification;
}

void AbstractMetaFunction::setExceptionSpecification(ExceptionSpecification e)
{
    d->m_exceptionSpecification = e;
}

static inline TypeSystem::ExceptionHandling exceptionMod(const AbstractMetaClass *klass)
{
    return klass->typeEntry()->exceptionHandling();
}

static inline bool hasExceptionMod(const AbstractMetaClass *klass)
{
    return exceptionMod(klass) != TypeSystem::ExceptionHandling::Unspecified;
}

bool AbstractMetaFunction::generateExceptionHandling() const
{
    switch (d->m_functionType) {
    case AbstractMetaFunction::CopyConstructorFunction:
    case AbstractMetaFunction::MoveConstructorFunction:
    case AbstractMetaFunction::AssignmentOperatorFunction:
    case AbstractMetaFunction::MoveAssignmentOperatorFunction:
    case AbstractMetaFunction::DestructorFunction:
        return false;
    default:
        break;
    }

    auto exceptionHandlingModification = d->m_exceptionHandlingModification;
    // If there is no modification on the function, check for a base class.
    if (d->m_class && exceptionHandlingModification == TypeSystem::ExceptionHandling::Unspecified) {
        if (auto base = recurseClassHierarchy(d->m_class, hasExceptionMod))
            exceptionHandlingModification = exceptionMod(base);
    }

    bool result = false;
    switch (exceptionHandlingModification) {
    case TypeSystem::ExceptionHandling::On:
        result = true;
        break;
    case TypeSystem::ExceptionHandling::AutoDefaultToOn:
        result = d->m_exceptionSpecification != ExceptionSpecification::NoExcept;
        break;
    case TypeSystem::ExceptionHandling::AutoDefaultToOff:
        result = d->m_exceptionSpecification == ExceptionSpecification::Throws;
        break;
    case TypeSystem::ExceptionHandling::Unspecified:
    case TypeSystem::ExceptionHandling::Off:
        break;
    }
    return result;
}

bool AbstractMetaFunction::isConversionOperator() const
{
    return isConversionOperator(originalName());
}

bool AbstractMetaFunction::isOperatorOverload(const QString &funcName)
{
    if (isConversionOperator(funcName))
        return true;

    static const QRegularExpression opRegEx(QLatin1String("^operator([+\\-\\*/%=&\\|\\^\\<>!][=]?"
                    "|\\+\\+|\\-\\-|&&|\\|\\||<<[=]?|>>[=]?|~"
                    "|\\[\\]|\\s+delete\\[?\\]?"
                    "|\\(\\)"
                    "|\\s+new\\[?\\]?)$"));
    Q_ASSERT(opRegEx.isValid());
    return opRegEx.match(funcName).hasMatch();
}

bool AbstractMetaFunction::isOperatorOverload() const
{
    return isOperatorOverload(originalName());
}

bool AbstractMetaFunction::isCastOperator() const
{
    return originalName().startsWith(QLatin1String("operator "));
}

bool AbstractMetaFunction::isArithmeticOperator() const
{
    if (!isOperatorOverload())
        return false;

    QString name = originalName();

    // It's a dereference operator!
    if (name == QLatin1String("operator*") && d->m_arguments.isEmpty())
        return false;

    return name == QLatin1String("operator+") || name == QLatin1String("operator+=")
            || name == QLatin1String("operator-") || name == QLatin1String("operator-=")
            || name == QLatin1String("operator*") || name == QLatin1String("operator*=")
            || name == QLatin1String("operator/") || name == QLatin1String("operator/=")
            || name == QLatin1String("operator%") || name == QLatin1String("operator%=")
            || name == QLatin1String("operator++") || name == QLatin1String("operator--");
}

bool AbstractMetaFunction::isBitwiseOperator() const
{
    if (!isOperatorOverload())
        return false;

    QString name = originalName();
    return name == QLatin1String("operator<<") || name == QLatin1String("operator<<=")
            || name == QLatin1String("operator>>") || name == QLatin1String("operator>>=")
            || name == QLatin1String("operator&") || name == QLatin1String("operator&=")
            || name == QLatin1String("operator|") || name == QLatin1String("operator|=")
            || name == QLatin1String("operator^") || name == QLatin1String("operator^=")
            || name == QLatin1String("operator~");
}

bool AbstractMetaFunction::isComparisonOperator() const
{
    if (!isOperatorOverload())
        return false;

    QString name = originalName();
    return name == QLatin1String("operator<") || name == QLatin1String("operator<=")
            || name == QLatin1String("operator>") || name == QLatin1String("operator>=")
            || name == QLatin1String("operator==") || name == QLatin1String("operator!=");
}

bool AbstractMetaFunction::isLogicalOperator() const
{
    if (!isOperatorOverload())
        return false;

    QString name = originalName();
    return name == QLatin1String("operator!")
            || name == QLatin1String("operator&&")
            || name == QLatin1String("operator||");
}

bool AbstractMetaFunction::isSubscriptOperator() const
{
    if (!isOperatorOverload())
        return false;

    return originalName() == QLatin1String("operator[]");
}

bool AbstractMetaFunction::isAssignmentOperator() const
{
    return d->m_functionType == AssignmentOperatorFunction
        || d->m_functionType == MoveAssignmentOperatorFunction;
}

bool AbstractMetaFunction::isOtherOperator() const
{
    if (!isOperatorOverload())
        return false;

    return !isArithmeticOperator()
            && !isBitwiseOperator()
            && !isComparisonOperator()
            && !isLogicalOperator()
            && !isConversionOperator()
            && !isSubscriptOperator()
            && !isAssignmentOperator();
}

int AbstractMetaFunction::arityOfOperator() const
{
    if (!isOperatorOverload() || isCallOperator())
        return -1;

    int arity = d->m_arguments.size();

    // Operator overloads that are class members
    // implicitly includes the instance and have
    // one parameter less than their arity,
    // so we increment it.
    if (ownerClass() && arity < 2)
        arity++;

    return arity;
}

bool AbstractMetaFunction::isInplaceOperator() const
{
    if (!isOperatorOverload())
        return false;

    QString name = originalName();
    return name == QLatin1String("operator+=") || name == QLatin1String("operator&=")
           || name == QLatin1String("operator-=") || name == QLatin1String("operator|=")
           || name == QLatin1String("operator*=") || name == QLatin1String("operator^=")
           || name == QLatin1String("operator/=") || name == QLatin1String("operator<<=")
           || name == QLatin1String("operator%=") || name == QLatin1String("operator>>=");
}

bool AbstractMetaFunction::isVirtual() const
{
    return attributes() & AbstractMetaAttributes::VirtualCppMethod;
}

QString AbstractMetaFunctionPrivate::modifiedName(const AbstractMetaFunction *q) const
{
    if (m_cachedModifiedName.isEmpty()) {
        for (const auto &mod : q->modifications(q->implementingClass())) {
            if (mod.isRenameModifier()) {
                m_cachedModifiedName = mod.renamedToName();
                break;
            }
        }
        if (m_cachedModifiedName.isEmpty())
            m_cachedModifiedName = m_name;
    }
    return m_cachedModifiedName;
}

QString AbstractMetaFunction::modifiedName() const
{
    return d->modifiedName(this);
}

AbstractMetaFunction *
AbstractMetaFunction::find(const AbstractMetaFunctionList &haystack,
                           const QString &needle)
{
    return findByName(haystack, needle);
}

void AbstractMetaFunction::setAllowThreadModification(TypeSystem::AllowThread am)
{
    d->m_allowThreadModification = am;
}

void AbstractMetaFunction::setExceptionHandlingModification(TypeSystem::ExceptionHandling em)
{
    d->m_exceptionHandlingModification = em;
}

int AbstractMetaFunctionPrivate::overloadNumber(const AbstractMetaFunction *q) const
{
    if (m_cachedOverloadNumber == TypeSystem::OverloadNumberUnset) {
        m_cachedOverloadNumber = TypeSystem::OverloadNumberDefault;
        for (const auto &mod : q->modifications(q->implementingClass())) {
            if (mod.overloadNumber() != TypeSystem::OverloadNumberUnset) {
                m_cachedOverloadNumber = mod.overloadNumber();
                break;
            }
        }
    }
    return m_cachedOverloadNumber;
}

int AbstractMetaFunction::overloadNumber() const
{
    return d->overloadNumber(this);
}

// Query functions for generators
bool AbstractMetaFunction::injectedCodeUsesPySelf() const
{
    return injectedCodeContains(u"%PYSELF", TypeSystem::CodeSnipPositionAny, TypeSystem::NativeCode);
}

bool AbstractMetaFunction::injectedCodeCallsPythonOverride() const
{
    static const QRegularExpression
        overrideCallRegexCheck(QStringLiteral(R"(PyObject_Call\s*\(\s*%PYTHON_METHOD_OVERRIDE\s*,)"));
    Q_ASSERT(overrideCallRegexCheck.isValid());
    return injectedCodeContains(overrideCallRegexCheck, TypeSystem::CodeSnipPositionAny,
                                TypeSystem::NativeCode);
}

bool AbstractMetaFunction::injectedCodeHasReturnValueAttribution(TypeSystem::Language language) const
{
    if (language == TypeSystem::TargetLangCode) {
        static const QRegularExpression
            retValAttributionRegexCheck_target(QStringLiteral(R"(%PYARG_0\s*=[^=]\s*.+)"));
        Q_ASSERT(retValAttributionRegexCheck_target.isValid());
        return injectedCodeContains(retValAttributionRegexCheck_target, TypeSystem::CodeSnipPositionAny, language);
    }

    static const QRegularExpression
        retValAttributionRegexCheck_native(QStringLiteral(R"(%0\s*=[^=]\s*.+)"));
    Q_ASSERT(retValAttributionRegexCheck_native.isValid());
    return injectedCodeContains(retValAttributionRegexCheck_native, TypeSystem::CodeSnipPositionAny, language);
}

bool AbstractMetaFunction::injectedCodeUsesArgument(int argumentIndex) const
{
    const QRegularExpression argRegEx = CodeSnipAbstract::placeHolderRegex(argumentIndex + 1);

    return traverseCodeSnips([argRegEx](const CodeSnip &s) {
                                 const QString code = s.code();
                                 return code.contains(u"%ARGUMENT_NAMES") || code.contains(argRegEx);
                             }, TypeSystem::CodeSnipPositionAny);
}

bool AbstractMetaFunction::isVisibilityModifiedToPrivate() const
{
    for (const auto &mod : modifications()) {
        if (mod.modifiers().testFlag(Modification::Private))
            return true;
    }
    return false;
}

#ifndef QT_NO_DEBUG_STREAM
void AbstractMetaFunction::formatDebugBrief(QDebug &debug) const
{
    debug << '"' << debugSignature() << '"';
}

void AbstractMetaFunction::formatDebugVerbose(QDebug &debug) const
{
    debug << d->m_functionType << ' ' << d->m_type << ' ' << d->m_name;
    switch (d->m_exceptionSpecification) {
    case ExceptionSpecification::Unknown:
        break;
    case ExceptionSpecification::NoExcept:
        debug << " noexcept";
        break;
    case ExceptionSpecification::Throws:
        debug << " throw(...)";
        break;
    }
    if (d->m_exceptionHandlingModification != TypeSystem::ExceptionHandling::Unspecified)
        debug << " exeption-mod " << int(d->m_exceptionHandlingModification);
    debug << '(';
    for (int i = 0, count = d->m_arguments.size(); i < count; ++i) {
        if (i)
            debug << ", ";
        debug <<  d->m_arguments.at(i);
    }
    debug << "), signature=\"" << minimalSignature() << '"';
    if (d->m_constant)
        debug << " [const]";
    if (d->m_reverse)
        debug << " [reverse]";
    if (isUserAdded())
        debug << " [userAdded]";
    if (isUserDeclared())
        debug << " [userDeclared]";
    if (d->m_explicit)
        debug << " [explicit]";
    if (attributes().testFlag(AbstractMetaAttributes::Deprecated))
        debug << " [deprecated]";
    if (d->m_pointerOperator)
        debug << " [operator->]";
    if (d->m_isCallOperator)
        debug << " [operator()]";
    if (d->m_class)
        debug << " class: " << d->m_class->name();
    if (d->m_implementingClass)
        debug << " implementing class: " << d->m_implementingClass->name();
    if (d->m_declaringClass)
        debug << " declaring class: " << d->m_declaringClass->name();
}

QDebug operator<<(QDebug debug, const AbstractMetaFunction *af)
{
    QDebugStateSaver saver(debug);
    debug.noquote();
    debug.nospace();
    debug << "AbstractMetaFunction(";
    if (af) {
        if (debug.verbosity() > 2) {
            af->formatDebugVerbose(debug);
        } else {
            debug << "signature=";
            af->formatDebugBrief(debug);
        }
    } else {
        debug << '0';
    }
    debug << ')';
    return debug;
}
#endif // !QT_NO_DEBUG_STREAM
