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
#include "messages.h"
#include "propertyspec.h"
#include "reporthandler.h"
#include "typedatabase.h"
#include "typesystem.h"

#include <QtCore/QDebug>

AbstractMetaFunction::AbstractMetaFunction(const AddedFunctionPtr &addedFunc) :
    AbstractMetaFunction()
{
    m_addedFunction = addedFunc;
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

AbstractMetaFunction::AbstractMetaFunction()
    : m_constant(false),
      m_reverse(false),
      m_explicit(false),
      m_pointerOperator(false),
      m_isCallOperator(false)
{
}

AbstractMetaFunction::~AbstractMetaFunction() = default;

/*******************************************************************************
 * Indicates that this function has a modification that removes it
 */
bool AbstractMetaFunction::isModifiedRemoved(int types) const
{
    const FunctionModificationList &mods = modifications(implementingClass());
    for (const FunctionModification &mod : mods) {
        if (!mod.isRemoveModifier())
            continue;

        if ((mod.removal & types) == types)
            return true;
    }

    return false;
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
    cpy->setExceptionSpecification(m_exceptionSpecification);
    cpy->setAllowThreadModification(m_allowThreadModification);
    cpy->setExceptionHandlingModification(m_exceptionHandlingModification);
    cpy->m_addedFunction = m_addedFunction;
    cpy->m_arguments = m_arguments;

    return cpy;
}

bool AbstractMetaFunction::usesRValueReferences() const
{
    if (m_functionType == MoveConstructorFunction || m_functionType == MoveAssignmentOperatorFunction)
        return true;
    if (m_type.referenceType() == RValueReference)
        return true;
    for (const AbstractMetaArgument &a : m_arguments) {
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

QString AbstractMetaFunction::signature() const
{
    if (m_cachedSignature.isEmpty()) {
        m_cachedSignature = m_originalName;

        m_cachedSignature += QLatin1Char('(');

        for (int i = 0; i < m_arguments.count(); ++i) {
            const AbstractMetaArgument &a = m_arguments.at(i);
            const AbstractMetaType &t = a.type();
            if (!t.isVoid()) {
                if (i > 0)
                    m_cachedSignature += QLatin1String(", ");
                m_cachedSignature += t.cppSignature();
                // We need to have the argument names in the qdoc files
                m_cachedSignature += QLatin1Char(' ');
                m_cachedSignature += a.name();
            } else {
                qCWarning(lcShiboken).noquote().nospace()
                    << QString::fromLatin1("No abstract meta type found for argument '%1' while"
                                           "constructing signature for function '%2'.")
                                           .arg(a.name(), name());
            }
        }
        m_cachedSignature += QLatin1Char(')');

        if (isConstant())
            m_cachedSignature += QLatin1String(" const");
    }
    return m_cachedSignature;
}

bool AbstractMetaFunction::isUserAdded() const
{
    return !m_addedFunction.isNull() && !m_addedFunction->isDeclaration();
}

bool AbstractMetaFunction::isUserDeclared() const
{
    return !m_addedFunction.isNull() && m_addedFunction->isDeclaration();
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

    const FunctionModificationList &mods = this->modifications(cls);
    for (const FunctionModification &mod : mods) {
        for (const ArgumentModification &argumentMod : mod.argument_mods) {
            if (argumentMod.index != idx && idx != -2)
                continue;
            returned += argumentMod.referenceCounts;
        }
    }

    return returned;
}

ArgumentOwner AbstractMetaFunction::argumentOwner(const AbstractMetaClass *cls, int idx) const
{
    const FunctionModificationList &mods = this->modifications(cls);
    for (const FunctionModification &mod : mods) {
        for (const ArgumentModification &argumentMod : mod.argument_mods) {
            if (argumentMod.index != idx)
                continue;
            return argumentMod.owner;
        }
    }
    return ArgumentOwner();
}

QString AbstractMetaFunction::conversionRule(TypeSystem::Language language, int key) const
{
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
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
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
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
    return m_class && m_class->isInvisibleNamespace()
        ?  m_class->targetLangEnclosingClass() : m_class;
}

bool AbstractMetaFunction::isDeprecated() const
{
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        if (modification.isDeprecated())
            return true;
    }
    return false;
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
    const bool maybeGetter = m_constant != 0 && !isVoid() && m_arguments.isEmpty();
    return !maybeGetter;
}

SourceLocation AbstractMetaFunction::sourceLocation() const
{
    return m_sourceLocation;
}

void AbstractMetaFunction::setSourceLocation(const SourceLocation &sourceLocation)
{
    m_sourceLocation = sourceLocation;
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
    auto allowThreadModification = m_allowThreadModification;
    // If there is no modification on the function, check for a base class.
    if (m_class && allowThreadModification == TypeSystem::AllowThread::Unspecified) {
        if (auto base = recurseClassHierarchy(m_class, hasAllowThreadMod))
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
    const FunctionModificationList &modifications = this->modifications(cls);
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
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
    const FunctionModificationList &modifications = this->modifications(cls);
    for (const FunctionModification &modification : modifications) {
        if ((modification.removal & language) == language)
            return true;
    }

    return false;
}

QString AbstractMetaFunction::typeReplaced(int key) const
{
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
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
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
            if (argumentModification.index == argumentIndex && argumentModification.array != 0)
                return true;
        }
    }
    return false;
}

QString AbstractMetaFunction::minimalSignature() const
{
    if (!m_cachedMinimalSignature.isEmpty())
        return m_cachedMinimalSignature;

    QString minimalSignature = originalName() + QLatin1Char('(');
    AbstractMetaArgumentList arguments = this->arguments();

    for (int i = 0; i < arguments.count(); ++i) {
        const AbstractMetaType &t = arguments.at(i).type();
        if (!t.isVoid()) {
            if (i > 0)
                minimalSignature += QLatin1Char(',');
            minimalSignature += t.minimalSignature();
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << QString::fromLatin1("No abstract meta type found for argument '%1' while constructing"
                                       " minimal signature for function '%2'.")
                                       .arg(arguments.at(i).name(), name());
        }
    }
    minimalSignature += QLatin1Char(')');
    if (isConstant())
        minimalSignature += QLatin1String("const");

    minimalSignature = TypeDatabase::normalizedSignature(minimalSignature);
    m_cachedMinimalSignature = minimalSignature;

    return minimalSignature;
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

FunctionModificationList AbstractMetaFunction::modifications(const AbstractMetaClass *implementor) const
{
    if (!m_addedFunction.isNull())
        return m_addedFunction->modifications;
    if (!implementor)
        implementor = ownerClass();

    if (!implementor)
        return TypeDatabase::instance()->functionModifications(minimalSignature());

    FunctionModificationList mods;
    while (implementor) {
        mods += implementor->typeEntry()->functionModifications(minimalSignature());
        if ((implementor == implementor->baseClass()) ||
            (implementor == implementingClass() && !mods.isEmpty())) {
                break;
        }
        implementor = implementor->baseClass();
    }
    return mods;
}

QString AbstractMetaFunction::argumentName(int index,
                                           bool /* create */,
                                           const AbstractMetaClass * /* implementor */) const
{
    return m_arguments[--index].name();
}

bool AbstractMetaFunction::isCallOperator() const
{
    return m_name == QLatin1String("operator()");
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

CodeSnipList AbstractMetaFunction::injectedCodeSnips(TypeSystem::CodeSnipPosition position, TypeSystem::Language language) const
{
    CodeSnipList result;
    const FunctionModificationList &mods = modifications(ownerClass());
    for (const FunctionModification &mod : mods) {
        if (mod.isCodeInjection()) {
            for (const CodeSnip &snip : mod.snips) {
                if ((snip.language & language) && (snip.position == position || position == TypeSystem::CodeSnipPositionAny))
                    result << snip;
            }
        }
    }
    return result;
}

bool AbstractMetaFunction::hasSignatureModifications() const
{
    const FunctionModificationList &mods = modifications();
    for (const FunctionModification &mod : mods) {
        if (mod.isRenameModifier())
            return true;
        for (const ArgumentModification &argmod : mod.argument_mods) {
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
    return m_exceptionSpecification;
}

void AbstractMetaFunction::setExceptionSpecification(ExceptionSpecification e)
{
    m_exceptionSpecification = e;
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
    switch (m_functionType) {
    case AbstractMetaFunction::CopyConstructorFunction:
    case AbstractMetaFunction::MoveConstructorFunction:
    case AbstractMetaFunction::AssignmentOperatorFunction:
    case AbstractMetaFunction::MoveAssignmentOperatorFunction:
    case AbstractMetaFunction::DestructorFunction:
        return false;
    default:
        break;
    }

    auto exceptionHandlingModification = m_exceptionHandlingModification;
    // If there is no modification on the function, check for a base class.
    if (m_class && exceptionHandlingModification == TypeSystem::ExceptionHandling::Unspecified) {
        if (auto base = recurseClassHierarchy(m_class, hasExceptionMod))
            exceptionHandlingModification = exceptionMod(base);
    }

    bool result = false;
    switch (exceptionHandlingModification) {
    case TypeSystem::ExceptionHandling::On:
        result = true;
        break;
    case TypeSystem::ExceptionHandling::AutoDefaultToOn:
        result = m_exceptionSpecification != ExceptionSpecification::NoExcept;
        break;
    case TypeSystem::ExceptionHandling::AutoDefaultToOff:
        result = m_exceptionSpecification == ExceptionSpecification::Throws;
        break;
    case TypeSystem::ExceptionHandling::Unspecified:
    case TypeSystem::ExceptionHandling::Off:
        break;
    }
    return result;
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
    if (name == QLatin1String("operator*") && m_arguments.isEmpty())
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
    return m_functionType == AssignmentOperatorFunction
        || m_functionType == MoveAssignmentOperatorFunction;
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

    int arity = m_arguments.size();

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

QString AbstractMetaFunction::modifiedName() const
{
    if (m_cachedModifiedName.isEmpty()) {
        const FunctionModificationList &mods = modifications(implementingClass());
        for (const FunctionModification &mod : mods) {
            if (mod.isRenameModifier()) {
                m_cachedModifiedName = mod.renamedToName;
                break;
            }
        }
        if (m_cachedModifiedName.isEmpty())
            m_cachedModifiedName = name();
    }
    return m_cachedModifiedName;
}

AbstractMetaFunction *
AbstractMetaFunction::find(const AbstractMetaFunctionList &haystack,
                           const QString &needle)
{
    return findByName(haystack, needle);
}

int AbstractMetaFunction::overloadNumber() const
{
    if (m_cachedOverloadNumber == TypeSystem::OverloadNumberUnset) {
        m_cachedOverloadNumber = TypeSystem::OverloadNumberDefault;
        const FunctionModificationList &mods = modifications(implementingClass());
        for (const FunctionModification &mod : mods) {
            if (mod.overloadNumber() != TypeSystem::OverloadNumberUnset) {
                m_cachedOverloadNumber = mod.overloadNumber();
                break;
            }
        }
    }
    return m_cachedOverloadNumber;
}

#ifndef QT_NO_DEBUG_STREAM
void AbstractMetaFunction::formatDebugBrief(QDebug &d) const
{
    d << '"' << debugSignature() << '"';
}

void AbstractMetaFunction::formatDebugVerbose(QDebug &d) const
{
    d << m_functionType << ' ' << m_type << ' ' << m_name;
    switch (m_exceptionSpecification) {
    case ExceptionSpecification::Unknown:
        break;
    case ExceptionSpecification::NoExcept:
        d << " noexcept";
        break;
    case ExceptionSpecification::Throws:
        d << " throw(...)";
        break;
    }
    if (m_exceptionHandlingModification != TypeSystem::ExceptionHandling::Unspecified)
        d << " exeption-mod " << int(m_exceptionHandlingModification);
    d << '(';
    for (int i = 0, count = m_arguments.size(); i < count; ++i) {
        if (i)
            d << ", ";
        d <<  m_arguments.at(i);
    }
    d << "), signature=\"" << minimalSignature() << '"';
    if (m_constant)
        d << " [const]";
    if (m_reverse)
        d << " [reverse]";
    if (isUserAdded())
        d << " [userAdded]";
    if (isUserDeclared())
        d << " [userDeclared]";
    if (m_explicit)
        d << " [explicit]";
    if (attributes().testFlag(AbstractMetaAttributes::Deprecated))
        d << " [deprecated]";
    if (m_pointerOperator)
        d << " [operator->]";
    if (m_isCallOperator)
        d << " [operator()]";
    if (m_class)
        d << " class: " << m_class->name();
    if (m_implementingClass)
        d << " implementing class: " << m_implementingClass->name();
    if (m_declaringClass)
        d << " declaring class: " << m_declaringClass->name();
}

QDebug operator<<(QDebug d, const AbstractMetaFunction *af)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaFunction(";
    if (af) {
        if (d.verbosity() > 2) {
            af->formatDebugVerbose(d);
        } else {
            d << "signature=";
            af->formatDebugBrief(d);
        }
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
