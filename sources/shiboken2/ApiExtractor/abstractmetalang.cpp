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
#include "messages.h"
#include "reporthandler.h"
#include "typedatabase.h"
#include "typesystem.h"

#include <parser/codemodel.h>

#ifndef QT_NO_DEBUG_STREAM
#  include <QtCore/QMetaEnum>
#  include <QtCore/QMetaObject>
#endif

#include <QtCore/QRegularExpression>
#include <QtCore/QStack>

#include <algorithm>

#include <algorithm>

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaAttributes *aa)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaAttributes(";
    if (aa)
        d << aa->attributes();
    else
        d << '0';
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

template <class MetaClass>
MetaClass *findByName(QVector<MetaClass *> haystack, QStringView needle)
{
    for (MetaClass *c : haystack) {
        if (c->name() == needle)
            return c;
    }
    return nullptr;
}

// Helper for recursing the base classes of an AbstractMetaClass.
// Returns the class for which the predicate is true.
template <class Predicate>
const AbstractMetaClass *recurseClassHierarchy(const AbstractMetaClass *klass,
                                               Predicate pred)
{
    if (pred(klass))
        return klass;
    for (auto base : klass->baseClasses()) {
        if (auto r = recurseClassHierarchy(base, pred))
            return r;
    }
    return nullptr;
}

/*******************************************************************************
 * Documentation
 */

Documentation::Documentation(const QString &value, Documentation::Type t, Documentation::Format fmt)
{
    setValue(value, t, fmt);
}

bool Documentation::isEmpty() const
{
    for (int i = 0; i < Type::Last; i++) {
        if (!m_data.value(static_cast<Type>(i)).isEmpty())
            return false;
    }
    return true;
}

QString Documentation::value(Documentation::Type t) const
{
    return m_data.value(t);
}

void Documentation::setValue(const QString &value, Documentation::Type t, Documentation::Format fmt)
{
    const QString v = value.trimmed();
    if (v.isEmpty())
        m_data.remove(t);
    else
        m_data[t] = value.trimmed();
    m_format = fmt;
}

Documentation::Format Documentation::format() const
{
    return m_format;
}

void Documentation::setFormat(Documentation::Format f)
{
    m_format = f;
}

/*******************************************************************************
 * AbstractMetaVariable
 */

AbstractMetaVariable::AbstractMetaVariable() = default;

AbstractMetaVariable::~AbstractMetaVariable()
{
    delete m_type;
}

void AbstractMetaVariable::assignMetaVariable(const AbstractMetaVariable &other)
{
    m_originalName = other.m_originalName;
    m_name = other.m_name;
    m_type = other.m_type->copy();
    m_hasName = other.m_hasName;
    m_doc = other.m_doc;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaVariable *av)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaVariable(";
    if (av) {
        d << av->type()->name() << ' ' << av->name();
    } else {
        d << '0';
      }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

/*******************************************************************************
 * AbstractMetaAttributes
 */

AbstractMetaAttributes::AbstractMetaAttributes() = default;
AbstractMetaAttributes::~AbstractMetaAttributes() = default;

void AbstractMetaAttributes::assignMetaAttributes(const AbstractMetaAttributes &other)
{
    m_attributes = other.m_attributes;
    m_originalAttributes = other.m_originalAttributes;
    m_doc = other.m_doc;
}

/*******************************************************************************
 * AbstractMetaType
 */

AbstractMetaType::AbstractMetaType() :
    m_constant(false),
    m_volatile(false),
    m_cppInstantiation(true),
    m_reserved(0)
{
}

AbstractMetaType::~AbstractMetaType()
{
    qDeleteAll(m_children);
    m_instantiations.clear();
}

QString AbstractMetaType::package() const
{
    return m_typeEntry->targetLangPackage();
}

QString AbstractMetaType::name() const
{
    return m_typeEntry->targetLangEntryName();
}

QString AbstractMetaType::fullName() const
{
    return m_typeEntry->qualifiedTargetLangName();
}

AbstractMetaType *AbstractMetaType::copy() const
{
    auto *cpy = new AbstractMetaType;

    cpy->setTypeUsagePattern(typeUsagePattern());
    cpy->setConstant(isConstant());
    cpy->setVolatile(isVolatile());
    cpy->setReferenceType(referenceType());
    cpy->setIndirectionsV(indirectionsV());
    cpy->setInstantiations(instantiations());
    cpy->setArrayElementCount(arrayElementCount());
    cpy->setOriginalTypeDescription(originalTypeDescription());
    cpy->setOriginalTemplateType(originalTemplateType() ? originalTemplateType()->copy() : nullptr);

    cpy->setArrayElementType(arrayElementType() ? arrayElementType()->copy() : nullptr);

    cpy->setTypeEntry(typeEntry());

    return cpy;
}

// For applying the <array> function argument modification: change into a type
// where "int *" becomes "int[]".
bool AbstractMetaType::applyArrayModification(QString *errorMessage)
{
    if (m_pattern == AbstractMetaType::NativePointerAsArrayPattern) {
        *errorMessage = QLatin1String("<array> modification already applied.");
        return false;
    }
    if (m_arrayElementType != nullptr)  {
        QTextStream(errorMessage) << "The type \"" << cppSignature()
            << "\" is an array of " << m_arrayElementType->name() << '.';
        return false;
    }
    if (m_indirections.isEmpty()) {
        QTextStream(errorMessage) << "The type \"" << cppSignature()
            << "\" does not have indirections.";
        return false;
    }
    // Element type to be used for ArrayHandle<>, strip constness.
    auto elementType = copy();
    elementType->m_indirections.pop_front();
    elementType->setConstant(false);
    elementType->setVolatile(false);
    elementType->decideUsagePattern();
    m_arrayElementType = elementType;
    setTypeUsagePattern(AbstractMetaType::NativePointerAsArrayPattern);
    return true;
}

AbstractMetaTypeCList AbstractMetaType::nestedArrayTypes() const
{
    AbstractMetaTypeCList result;
    switch (m_pattern) {
    case ArrayPattern:
        for (const AbstractMetaType *t = this; t->typeUsagePattern() == ArrayPattern; ) {
            const AbstractMetaType *elt = t->arrayElementType();
            result.append(elt);
            t = elt;
        }
        break;
    case NativePointerAsArrayPattern:
        result.append(m_arrayElementType);
        break;
    default:
        break;
    }
    return result;
}

bool AbstractMetaType::passByConstRef() const
{
    return isConstant() && m_referenceType == LValueReference && indirections() == 0;
}

bool AbstractMetaType::passByValue() const
{
    return m_referenceType == NoReference && indirections() == 0;
}

QString AbstractMetaType::cppSignature() const
{
    if (m_cachedCppSignature.isEmpty())
        m_cachedCppSignature = formatSignature(false);
    return m_cachedCppSignature;
}

QString AbstractMetaType::pythonSignature() const
{
    // PYSIDE-921: Handle container returntypes correctly.
    // This is now a clean reimplementation.
    if (m_cachedPythonSignature.isEmpty())
        m_cachedPythonSignature = formatPythonSignature();
    return m_cachedPythonSignature;
}

AbstractMetaType::TypeUsagePattern AbstractMetaType::determineUsagePattern() const
{
    if (m_typeEntry->isTemplateArgument() || m_referenceType == RValueReference)
        return InvalidPattern;

    if (m_typeEntry->isPrimitive() && (actualIndirections() == 0 || passByConstRef()))
        return PrimitivePattern;

    if (m_typeEntry->isVoid())
        return NativePointerPattern;

    if (m_typeEntry->isVarargs())
        return VarargsPattern;

    if (m_typeEntry->isEnum() && (actualIndirections() == 0 || passByConstRef()))
        return EnumPattern;

    if (m_typeEntry->isObject()) {
        if (indirections() == 0 && m_referenceType == NoReference)
            return ValuePattern;
        return ObjectPattern;
    }

    if (m_typeEntry->isContainer() && indirections() == 0)
        return ContainerPattern;

    if (m_typeEntry->isSmartPointer() && indirections() == 0)
        return SmartPointerPattern;

    if (m_typeEntry->isFlags() && (actualIndirections() == 0 || passByConstRef()))
        return FlagsPattern;

    if (m_typeEntry->isArray())
        return ArrayPattern;

    if (m_typeEntry->isValue())
        return indirections() == 1 ? ValuePointerPattern : ValuePattern;

    return NativePointerPattern;
}

void AbstractMetaType::decideUsagePattern()
{
    TypeUsagePattern pattern = determineUsagePattern();
    if (m_typeEntry->isObject() && indirections() == 1
        && m_referenceType == LValueReference && isConstant()) {
        // const-references to pointers can be passed as pointers
        setReferenceType(NoReference);
        setConstant(false);
        pattern = ObjectPattern;
    }
    setTypeUsagePattern(pattern);
}

bool AbstractMetaType::hasTemplateChildren() const
{
    QStack<AbstractMetaType *> children;
    children << m_children;

    // Recursively iterate over the children / descendants of the type, to check if any of them
    // corresponds to a template argument type.
    while (!children.isEmpty()) {
        AbstractMetaType *child = children.pop();
        if (child->typeEntry()->isTemplateArgument())
            return true;
        children << child->m_children;
    }

    return false;
}

bool AbstractMetaType::compare(const AbstractMetaType &rhs, ComparisonFlags flags) const
{
    if (m_typeEntry != rhs.m_typeEntry
        || m_indirections != rhs.m_indirections
        || m_instantiations.size() != rhs.m_instantiations.size()
        || m_arrayElementCount != rhs.m_arrayElementCount) {
        return false;
    }

    if (m_constant != rhs.m_constant || m_referenceType != rhs.m_referenceType) {
        if (!flags.testFlag(ConstRefMatchesValue)
            || !(passByValue() || passByConstRef())
            || !(rhs.passByValue() || rhs.passByConstRef())) {
            return false;
        }
    }

    if ((m_arrayElementType != nullptr) != (rhs.m_arrayElementType != nullptr)
        || (m_arrayElementType != nullptr && !m_arrayElementType->compare(*rhs.m_arrayElementType, flags))) {
        return false;
    }
    for (int i = 0, size = m_instantiations.size(); i < size; ++i) {
        if (!m_instantiations.at(i)->compare(*rhs.m_instantiations.at(i), flags))
                return false;
    }
    return true;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaType *at)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaType(";
    if (at) {
        d << at->name();
        if (d.verbosity() > 2) {
            d << ", typeEntry=" << at->typeEntry() << ", signature=\""
                << at->cppSignature() << "\", pattern="
                << at->typeUsagePattern();
            const auto indirections = at->indirectionsV();
            if (!indirections.isEmpty()) {
                d << ", indirections=";
                for (auto i : indirections)
                    d << ' ' << TypeInfo::indirectionKeyword(i);
            }
            if (at->referenceType())
                d << ", reftype=" << at->referenceType();
            if (at->isConstant())
                d << ", [const]";
            if (at->isVolatile())
                d << ", [volatile]";
            if (at->isArray()) {
                d << ", array of \"" << at->arrayElementType()->cppSignature()
                    << "\", arrayElementCount="  << at->arrayElementCount();
            }
            const auto &instantiations = at->instantiations();
            if (const int instantiationsSize = instantiations.size()) {
                d << ", instantiations[" << instantiationsSize << "]=<";
                for (int i = 0; i < instantiationsSize; ++i) {
                    if (i)
                        d << ", ";
                    d << instantiations.at(i);
                }
            }
            d << '>';
        }
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

/*******************************************************************************
 * AbstractMetaArgument
 */

AbstractMetaArgument::AbstractMetaArgument() = default;

void AbstractMetaArgument::assignMetaArgument(const AbstractMetaArgument &other)
{
    assignMetaVariable(other);
    m_expression = other.m_expression;
    m_originalExpression = other.m_originalExpression;
    m_argumentIndex = other.m_argumentIndex;
}

AbstractMetaArgument *AbstractMetaArgument::copy() const
{
    auto *copy = new AbstractMetaArgument;
    copy->assignMetaArgument(*this);
    return copy;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaArgument *aa)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaArgument(";
    if (aa)
        d << aa->toString();
    else
        d << '0';
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

/*******************************************************************************
 * AbstractMetaFunction
 */

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

AbstractMetaFunction::~AbstractMetaFunction()
{
    qDeleteAll(m_arguments);
    delete m_type;
}

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
    AbstractMetaType *t = type();
    AbstractMetaType *ot = other->type();
    if ((!t && !ot) || ((t && ot && t->name() == ot->name())))
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
            const AbstractMetaArgument *min_arg = minArguments.at(i);
            const AbstractMetaArgument *max_arg = maxArguments.at(i);
            if (min_arg->type()->name() != max_arg->type()->name()
                && (min_arg->defaultValueExpression().isEmpty() || max_arg->defaultValueExpression().isEmpty())) {
                same = false;
                break;
            }
        } else {
            if (maxArguments.at(i)->defaultValueExpression().isEmpty()) {
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
    if (type())
        cpy->setType(type()->copy());
    cpy->setConstant(isConstant());
    cpy->setExceptionSpecification(m_exceptionSpecification);
    cpy->setAllowThreadModification(m_allowThreadModification);
    cpy->setExceptionHandlingModification(m_exceptionHandlingModification);
    cpy->m_addedFunction = m_addedFunction;

    for (AbstractMetaArgument *arg : m_arguments)
    cpy->addArgument(arg->copy());

    Q_ASSERT((!type() && !cpy->type())
             || (type()->instantiations() == cpy->type()->instantiations()));

    return cpy;
}

bool AbstractMetaFunction::usesRValueReferences() const
{
    if (m_functionType == MoveConstructorFunction || m_functionType == MoveAssignmentOperatorFunction)
        return true;
    if (m_type && m_type->referenceType() == RValueReference)
        return true;
    for (const AbstractMetaArgument *a : m_arguments) {
        if (a->type()->referenceType() == RValueReference)
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

    AbstractMetaArgument *argument = arguments.at(resolvedArguments.size());
    QStringList minimalTypeSignature = argument->type()->minimalSignature().split(QLatin1String("::"));
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
            AbstractMetaArgument *a = m_arguments.at(i);
            AbstractMetaType *t = a->type();
            if (t) {
                if (i > 0)
                    m_cachedSignature += QLatin1String(", ");
                m_cachedSignature += t->cppSignature();
                // We need to have the argument names in the qdoc files
                m_cachedSignature += QLatin1Char(' ');
                m_cachedSignature += a->name();
            } else {
                qCWarning(lcShiboken).noquote().nospace()
                    << QString::fromLatin1("No abstract meta type found for argument '%1' while"
                                           "constructing signature for function '%2'.")
                                           .arg(a->name(), name());
            }
        }
        m_cachedSignature += QLatin1Char(')');

        if (isConstant())
            m_cachedSignature += QLatin1String(" const");
    }
    return m_cachedSignature;
}

int AbstractMetaFunction::actualMinimumArgumentCount() const
{
    AbstractMetaArgumentList arguments = this->arguments();

    int count = 0;
    for (int i = 0; i < arguments.size(); ++i && ++count) {
        if (argumentRemoved(i + 1))
            --count;
        else if (!arguments.at(i)->defaultValueExpression().isEmpty())
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
    const bool maybeGetter = m_constant != 0 && m_type != nullptr
        && m_arguments.isEmpty();
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
        AbstractMetaType *t = arguments.at(i)->type();
        if (t) {
            if (i > 0)
                minimalSignature += QLatin1Char(',');
            minimalSignature += t->minimalSignature();
        } else {
            qCWarning(lcShiboken).noquote().nospace()
                << QString::fromLatin1("No abstract meta type found for argument '%1' while constructing"
                                       " minimal signature for function '%2'.")
                                       .arg(arguments.at(i)->name(), name());
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
    return m_arguments[--index]->name();
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

bool function_sorter(AbstractMetaFunction *a, AbstractMetaFunction *b)
{
    return a->signature() < b->signature();
}

AbstractMetaFunction *
AbstractMetaFunction::find(const AbstractMetaFunctionList &haystack,
                           const QString &needle)
{
    return findByName(haystack, needle);
}

#ifndef QT_NO_DEBUG_STREAM
static inline void formatMetaFunctionBrief(QDebug &d, const AbstractMetaFunction *af)
{
    d << '"' << af->debugSignature() << '"';
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
            formatMetaFunctionBrief(d, af);
        }
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

/*******************************************************************************
 * AbstractMetaClass
 */

AbstractMetaClass::AbstractMetaClass()
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

AbstractMetaClass::~AbstractMetaClass()
{
    qDeleteAll(m_functions);
    qDeleteAll(m_fields);
    qDeleteAll(m_enums);
    if (hasTemplateBaseClassInstantiations())
        qDeleteAll(templateBaseClassInstantiations());
}

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

/*******************************************************************************
 * Returns a list of all the functions with a given name
 */
AbstractMetaFunctionList AbstractMetaClass::queryFunctionsByName(const QString &name) const
{
    AbstractMetaFunctionList returned;
    for (AbstractMetaFunction *function : m_functions) {
        if (function->name() == name)
            returned.append(function);
    }

    return returned;
}

/*******************************************************************************
 * Returns a list of all the functions retrieved during parsing which should
 * be added to the API.
 */
AbstractMetaFunctionList AbstractMetaClass::functionsInTargetLang() const
{
    FunctionQueryOptions default_flags = NormalFunctions | Visible | NotRemovedFromTargetLang;

    // Only public functions in final classes
    // default_flags |= isFinal() ? WasPublic : 0;
    FunctionQueryOptions public_flags;
    if (isFinalInTargetLang())
        public_flags |= WasPublic;

    // Constructors
    AbstractMetaFunctionList returned = queryFunctions(Constructors | default_flags | public_flags);

    // Final functions
    returned += queryFunctions(FinalInTargetLangFunctions | NonStaticFunctions | default_flags | public_flags);

    // Virtual functions
    returned += queryFunctions(VirtualInTargetLangFunctions | NonStaticFunctions | default_flags | public_flags);

    // Static functions
    returned += queryFunctions(StaticFunctions | default_flags | public_flags);

    // Empty, private functions, since they aren't caught by the other ones
    returned += queryFunctions(Empty | Invisible);

    return returned;
}

AbstractMetaFunctionList AbstractMetaClass::implicitConversions() const
{
    if (!hasCloneOperator() && !hasExternalConversionOperators())
        return AbstractMetaFunctionList();

    AbstractMetaFunctionList returned;
    const AbstractMetaFunctionList list = queryFunctions(Constructors) + externalConversionOperators();

    // Exclude anything that uses rvalue references, be it a move
    // constructor "QPolygon(QPolygon &&)" or something else like
    // "QPolygon(QVector<QPoint> &&)".
    for (AbstractMetaFunction *f : list) {
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

AbstractMetaFunctionList AbstractMetaClass::operatorOverloads(OperatorQueryOptions query) const
{
    const AbstractMetaFunctionList &list = queryFunctions(OperatorOverloads | Visible);
    AbstractMetaFunctionList returned;
    for (AbstractMetaFunction *f : list) {
        if (((query & ArithmeticOp) && f->isArithmeticOperator())
            || ((query & BitwiseOp) && f->isBitwiseOperator())
            || ((query & ComparisonOp) && f->isComparisonOperator())
            || ((query & LogicalOp) && f->isLogicalOperator())
            || ((query & SubscriptionOp) && f->isSubscriptOperator())
            || ((query & AssignmentOp) && f->isAssignmentOperator())
            || ((query & ConversionOp) && f->isConversionOperator())
            || ((query & OtherOp) && f->isOtherOperator()))
            returned += f;
    }

    return returned;
}

bool AbstractMetaClass::hasArithmeticOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isArithmeticOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasBitwiseOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isBitwiseOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasComparisonOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isComparisonOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasLogicalOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isLogicalOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

void AbstractMetaClass::sortFunctions()
{
    std::sort(m_functions.begin(), m_functions.end(), function_sorter);
}

void AbstractMetaClass::setFunctions(const AbstractMetaFunctionList &functions)
{
    m_functions = functions;

    // Functions must be sorted by name before next loop
    sortFunctions();

    for (AbstractMetaFunction *f : qAsConst(m_functions)) {
        f->setOwnerClass(this);
        if (!f->isPublic())
            m_hasNonpublic = true;
    }
}

bool AbstractMetaClass::hasFieldAccessors() const
{
    for (const AbstractMetaField *field : m_fields) {
        if (field->getter() || field->setter())
            return true;
    }

    return false;
}

bool AbstractMetaClass::hasDefaultToStringFunction() const
{
    const AbstractMetaFunctionList &funcs = queryFunctionsByName(QLatin1String("toString"));
    for (const AbstractMetaFunction *f : funcs) {
        if (!f->actualMinimumArgumentCount())
            return true;
    }
    return false;
}

void AbstractMetaClass::addFunction(AbstractMetaFunction *function)
{
    Q_ASSERT(!function->signature().startsWith(QLatin1Char('(')));
    function->setOwnerClass(this);

    if (!function->isDestructor())
        m_functions << function;
    else
        Q_ASSERT(false); //memory leak

    m_hasVirtuals |= function->isVirtual();
    m_isPolymorphic |= m_hasVirtuals;
    m_hasNonpublic |= !function->isPublic();
}

bool AbstractMetaClass::hasSignal(const AbstractMetaFunction *other) const
{
    if (!other->isSignal())
        return false;

    for (const AbstractMetaFunction *f : m_functions) {
        if (f->isSignal() && f->compareTo(other) & AbstractMetaFunction::EqualName)
            return other->modifiedName() == f->modifiedName();
    }

    return false;
}


QString AbstractMetaClass::name() const
{
    return m_typeEntry->targetLangEntryName();
}

void AbstractMetaClass::addBaseClass(AbstractMetaClass *baseClass)
{
    Q_ASSERT(baseClass);
    m_baseClasses.append(baseClass);
    m_isPolymorphic |= baseClass->isPolymorphic();
}

void AbstractMetaClass::setBaseClass(AbstractMetaClass *baseClass)
{
    if (baseClass) {
        m_baseClasses.prepend(baseClass);
        m_isPolymorphic |= baseClass->isPolymorphic();
    }
}

QString AbstractMetaClass::package() const
{
    return m_typeEntry->targetLangPackage();
}

bool AbstractMetaClass::isNamespace() const
{
    return m_typeEntry->isNamespace();
}

static bool qObjectPredicate(const AbstractMetaClass *c)
{
    return c->qualifiedCppName() == QLatin1String("QObject");
}

bool AbstractMetaClass::isQObject() const
{
    return qObjectPredicate(this) || recurseClassHierarchy(this, qObjectPredicate) != nullptr;
}

QString AbstractMetaClass::qualifiedCppName() const
{
    return m_typeEntry->qualifiedCppName();
}

bool AbstractMetaClass::hasFunction(const QString &str) const
{
    return findFunction(str);
}

const AbstractMetaFunction *AbstractMetaClass::findFunction(const QString &functionName) const
{
    return AbstractMetaFunction::find(m_functions, functionName);
}

bool AbstractMetaClass::hasProtectedFunctions() const
{
    for (AbstractMetaFunction *func : m_functions) {
        if (func->isProtected())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasProtectedFields() const
{
    for (const AbstractMetaField *field : m_fields) {
        if (field->isProtected())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasProtectedMembers() const
{
    return hasProtectedFields() || hasProtectedFunctions();
}

QPropertySpec *AbstractMetaClass::propertySpecForRead(const QString &name) const
{
    for (const auto &propertySpec : m_propertySpecs) {
        if (name == propertySpec->read())
            return propertySpec;
    }
    return nullptr;
}

QPropertySpec *AbstractMetaClass::propertySpecForWrite(const QString &name) const
{
    for (const auto &propertySpec : m_propertySpecs) {
        if (name == propertySpec->write())
            return propertySpec;
    }
    return nullptr;
}

QPropertySpec *AbstractMetaClass::propertySpecForReset(const QString &name) const
{
    for (const auto &propertySpec : m_propertySpecs) {
        if (name == propertySpec->reset())
            return propertySpec;
    }
    return nullptr;
}

using AbstractMetaClassBaseTemplateInstantiationsMap = QHash<const AbstractMetaClass *, AbstractMetaTypeList>;
Q_GLOBAL_STATIC(AbstractMetaClassBaseTemplateInstantiationsMap, metaClassBaseTemplateInstantiations);

bool AbstractMetaClass::hasTemplateBaseClassInstantiations() const
{
    if (!templateBaseClass())
        return false;
    return metaClassBaseTemplateInstantiations()->contains(this);
}

AbstractMetaTypeList AbstractMetaClass::templateBaseClassInstantiations() const
{
    if (!templateBaseClass())
        return AbstractMetaTypeList();
    return metaClassBaseTemplateInstantiations()->value(this);
}

void AbstractMetaClass::setTemplateBaseClassInstantiations(AbstractMetaTypeList &instantiations)
{
    if (!templateBaseClass())
        return;
    metaClassBaseTemplateInstantiations()->insert(this, instantiations);
}

// Does any of the base classes require deletion in the main thread?
bool AbstractMetaClass::deleteInMainThread() const
{
    return typeEntry()->deleteInMainThread()
        || (!m_baseClasses.isEmpty() && m_baseClasses.constFirst()->deleteInMainThread());
}

static bool functions_contains(const AbstractMetaFunctionList &l, const AbstractMetaFunction *func)
{
    for (const AbstractMetaFunction *f : l) {
        if ((f->compareTo(func) & AbstractMetaFunction::PrettySimilar) == AbstractMetaFunction::PrettySimilar)
            return true;
    }
    return false;
}

AbstractMetaField::AbstractMetaField() = default;

AbstractMetaField::~AbstractMetaField()
{
    delete m_setter;
    delete m_getter;
}

AbstractMetaField *AbstractMetaField::copy() const
{
    auto *returned = new AbstractMetaField;
    returned->assignMetaVariable(*this);
    returned->assignMetaAttributes(*this);
    returned->setEnclosingClass(nullptr);
    return returned;
}

AbstractMetaField *AbstractMetaField::find(const AbstractMetaFieldList &haystack,
                                           const QString &needle)
{
    return findByName(haystack, needle);
}
/*******************************************************************************
 * Indicates that this field has a modification that removes it
 */
bool AbstractMetaField::isModifiedRemoved(int types) const
{
    const FieldModificationList &mods = modifications();
    for (const FieldModification &mod : mods) {
        if (!mod.isRemoveModifier())
            continue;

        if ((mod.removal & types) == types)
            return true;
    }

    return false;
}

static QString upCaseFirst(const QString &str)
{
    Q_ASSERT(!str.isEmpty());
    QString s = str;
    s[0] = s.at(0).toUpper();
    return s;
}

static AbstractMetaFunction *createXetter(const AbstractMetaField *g, const QString &name,
                                          AbstractMetaAttributes::Attributes type)
{
    auto *f = new AbstractMetaFunction;

    f->setName(name);
    f->setOriginalName(name);
    f->setOwnerClass(g->enclosingClass());
    f->setImplementingClass(g->enclosingClass());
    f->setDeclaringClass(g->enclosingClass());

    AbstractMetaAttributes::Attributes attr = AbstractMetaAttributes::FinalInTargetLang | type;
    if (g->isStatic())
        attr |= AbstractMetaAttributes::Static;
    if (g->isPublic())
        attr |= AbstractMetaAttributes::Public;
    else if (g->isProtected())
        attr |= AbstractMetaAttributes::Protected;
    else
        attr |= AbstractMetaAttributes::Private;
    f->setAttributes(attr);
    f->setOriginalAttributes(attr);

    const FieldModificationList &mods = g->modifications();
    for (const FieldModification &mod : mods) {
        if (mod.isRenameModifier())
            f->setName(mod.renamedTo());
        if (mod.isAccessModifier()) {
            if (mod.isPrivate())
                f->setVisibility(AbstractMetaAttributes::Private);
            else if (mod.isProtected())
                f->setVisibility(AbstractMetaAttributes::Protected);
            else if (mod.isPublic())
                f->setVisibility(AbstractMetaAttributes::Public);
            else if (mod.isFriendly())
                f->setVisibility(AbstractMetaAttributes::Friendly);
        }
    }
    return f;
}

FieldModificationList AbstractMetaField::modifications() const
{
    const FieldModificationList &mods = enclosingClass()->typeEntry()->fieldModifications();
    FieldModificationList returned;

    for (const FieldModification &mod : mods) {
        if (mod.name == name())
            returned += mod;
    }

    return returned;
}

const AbstractMetaFunction *AbstractMetaField::setter() const
{
    if (!m_setter) {
        m_setter = createXetter(this,
                                QLatin1String("set") + upCaseFirst(name()),
                                AbstractMetaAttributes::SetterFunction);
        AbstractMetaArgumentList arguments;
        auto *argument = new AbstractMetaArgument;
        argument->setType(type()->copy());
        argument->setName(name());
        arguments.append(argument);
        m_setter->setArguments(arguments);
    }
    return m_setter;
}

const AbstractMetaClass *EnclosingClassMixin::targetLangEnclosingClass() const
{
    auto result = m_enclosingClass;
    while (result && !NamespaceTypeEntry::isVisibleScope(result->typeEntry()))
        result = result->enclosingClass();
    return result;
}

const AbstractMetaFunction *AbstractMetaField::getter() const
{
    if (!m_getter) {
        m_getter = createXetter(this,
                                name(),
                                AbstractMetaAttributes::GetterFunction);
        m_getter->setType(type());
    }

    return m_getter;
}

#ifndef QT_NO_DEBUG_STREAM
static void formatMetaAttributes(QDebug &d, AbstractMetaAttributes::Attributes value)
{
    static const int meIndex = AbstractMetaAttributes::staticMetaObject.indexOfEnumerator("Attribute");
    Q_ASSERT(meIndex >= 0);
    const QMetaEnum me = AbstractMetaAttributes::staticMetaObject.enumerator(meIndex);
    d << me.valueToKeys(value);
}

static void formatMetaField(QDebug &d, const AbstractMetaField *af)
{
    formatMetaAttributes(d, af->attributes());
    d << ' ' << af->type()->name() << " \"" << af->name() << '"';
}

QDebug operator<<(QDebug d, const AbstractMetaField *af)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaField(";
    if (af)
        formatMetaField(d, af);
    else
        d << '0';
    d << ')';
    return d;
}

static void formatMetaEnumValue(QDebug &d, const AbstractMetaEnumValue *v)
{
    const QString &name = v->stringValue();
    if (!name.isEmpty())
        d << name << '=';
    d << v->value();
}

QDebug operator<<(QDebug d, const AbstractMetaEnumValue *v)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaEnumValue(";
    if (v)
        formatMetaEnumValue(d, v);
    else
        d << '0';
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const AbstractMetaEnum *ae)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaEnum(";
    if (ae) {
        d << ae->fullName();
        if (!ae->isSigned())
            d << " (unsigned) ";
        d << '[';
        const AbstractMetaEnumValueList &values = ae->values();
        for (int i = 0, count = values.size(); i < count; ++i) {
            if (i)
                d << ' ';
            formatMetaEnumValue(d, values.at(i));
        }
        d << ']';
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

bool AbstractMetaClass::hasConstructors() const
{
    return AbstractMetaClass::queryFirstFunction(m_functions, Constructors) != nullptr;
}

const AbstractMetaFunction *AbstractMetaClass::copyConstructor() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->functionType() == AbstractMetaFunction::CopyConstructorFunction)
            return f;
    }
    return nullptr;
}

bool AbstractMetaClass::hasPrivateCopyConstructor() const
{
    const AbstractMetaFunction *copyCt = copyConstructor();
    return copyCt && copyCt->isPrivate();
}

void AbstractMetaClass::addDefaultConstructor()
{
    auto *f = new AbstractMetaFunction;
    f->setOriginalName(name());
    f->setName(name());
    f->setOwnerClass(this);
    f->setFunctionType(AbstractMetaFunction::ConstructorFunction);
    f->setArguments(AbstractMetaArgumentList());
    f->setDeclaringClass(this);

    f->setAttributes(Public | FinalInTargetLang | AddedMethod);
    f->setImplementingClass(this);
    f->setOriginalAttributes(f->attributes());

    addFunction(f);
    this->setHasNonPrivateConstructor(true);
}

void AbstractMetaClass::addDefaultCopyConstructor(bool isPrivate)
{
    auto f = new AbstractMetaFunction;
    f->setOriginalName(name());
    f->setName(name());
    f->setOwnerClass(this);
    f->setFunctionType(AbstractMetaFunction::CopyConstructorFunction);
    f->setDeclaringClass(this);

    auto argType = new AbstractMetaType;
    argType->setTypeEntry(typeEntry());
    argType->setReferenceType(LValueReference);
    argType->setConstant(true);
    argType->setTypeUsagePattern(AbstractMetaType::ValuePattern);

    auto arg = new AbstractMetaArgument;
    arg->setType(argType);
    arg->setName(name());
    f->addArgument(arg);

    AbstractMetaAttributes::Attributes attr = FinalInTargetLang | AddedMethod;
    if (isPrivate)
        attr |= AbstractMetaAttributes::Private;
    else
        attr |= AbstractMetaAttributes::Public;
    f->setAttributes(attr);
    f->setImplementingClass(this);
    f->setOriginalAttributes(f->attributes());

    addFunction(f);
}

void AbstractMetaClass::setHasVirtualDestructor(bool value)
{
    m_hasVirtualDestructor = value;
    if (value)
        m_hasVirtuals = m_isPolymorphic = 1;
}

bool AbstractMetaClass::hasFunction(const AbstractMetaFunction *f) const
{
    return functions_contains(m_functions, f);
}

bool AbstractMetaClass::generateExceptionHandling() const
{
    return queryFirstFunction(m_functions, AbstractMetaClass::Visible
                              | AbstractMetaClass::GenerateExceptionHandling) != nullptr;
}
/* Goes through the list of functions and returns a list of all
   functions matching all of the criteria in \a query.
 */

bool AbstractMetaClass::queryFunction(const AbstractMetaFunction *f, FunctionQueryOptions query)
{
    if ((query & NotRemovedFromTargetLang)
        && f->isRemovedFrom(f->implementingClass(), TypeSystem::TargetLangCode)) {
        return false;
    }

    if ((query & NotRemovedFromTargetLang) && f->isVirtual()
        && f->isRemovedFrom(f->declaringClass(), TypeSystem::TargetLangCode)) {
        return false;
    }

    if ((query & Visible) && f->isPrivate())
        return false;

    if ((query & VirtualInTargetLangFunctions) && f->isFinalInTargetLang())
        return false;

    if ((query & Invisible) && !f->isPrivate())
        return false;

    if ((query & Empty) && !f->isEmptyFunction())
        return false;

    if ((query & WasPublic) && !f->wasPublic())
        return false;

    if ((query & ClassImplements) && f->ownerClass() != f->implementingClass())
        return false;

    if ((query & FinalInTargetLangFunctions) && !f->isFinalInTargetLang())
        return false;

    if ((query & VirtualInCppFunctions) && !f->isVirtual())
        return false;

    if ((query & Signals) && (!f->isSignal()))
        return false;

    if ((query & Constructors) && (!f->isConstructor() || f->ownerClass() != f->implementingClass()))
        return false;

    if (!(query & Constructors) && f->isConstructor())
        return false;

    // Destructors are never included in the functions of a class currently
    /*
           if ((query & Destructors) && (!f->isDestructor()
                                       || f->ownerClass() != f->implementingClass())
            || f->isDestructor() && (query & Destructors) == 0) {
            return false;
        }*/

    if ((query & StaticFunctions) && (!f->isStatic() || f->isSignal()))
        return false;

    if ((query & NonStaticFunctions) && (f->isStatic()))
        return false;

    if ((query & NormalFunctions) && (f->isSignal()))
        return false;

    if ((query & OperatorOverloads) && !f->isOperatorOverload())
        return false;

    if ((query & GenerateExceptionHandling) && !f->generateExceptionHandling())
        return false;

    if (query.testFlag(GetAttroFunction)
        && f->functionType() != AbstractMetaFunction::GetAttroFunction) {
        return false;
    }

    if (query.testFlag(SetAttroFunction)
        && f->functionType() != AbstractMetaFunction::SetAttroFunction) {
        return false;
    }

    return true;
}

AbstractMetaFunctionList AbstractMetaClass::queryFunctionList(const AbstractMetaFunctionList &list,
                                                              FunctionQueryOptions query)
{
    AbstractMetaFunctionList result;
    for (AbstractMetaFunction *f : list) {
        if (queryFunction(f, query))
            result.append(f);
    }
    return result;
}

const AbstractMetaFunction *AbstractMetaClass::queryFirstFunction(const AbstractMetaFunctionList &list,
                                                               FunctionQueryOptions query)
{
    AbstractMetaFunctionList result;
    for (AbstractMetaFunction *f : list) {
        if (queryFunction(f, query))
            return f;
    }
    return nullptr;
}

AbstractMetaFunctionList AbstractMetaClass::queryFunctions(FunctionQueryOptions query) const
{
    return AbstractMetaClass::queryFunctionList(m_functions, query);
}

bool AbstractMetaClass::hasSignals() const
{
    return queryFirstFunction(m_functions, Signals | Visible | NotRemovedFromTargetLang) != nullptr;
}

AbstractMetaFunctionList AbstractMetaClass::cppSignalFunctions() const
{
    return queryFunctions(Signals | Visible | NotRemovedFromTargetLang);
}

AbstractMetaField *AbstractMetaClass::findField(const QString &name) const
{
    return AbstractMetaField::find(m_fields, name);
}

AbstractMetaEnum *AbstractMetaClass::findEnum(const QString &enumName)
{
    if (AbstractMetaEnum *e = findByName(m_enums, enumName))
        return e;
    return nullptr;
}

/*!  Recursively searches for the enum value named \a enumValueName in
  this class and its superclasses and interfaces.
*/
AbstractMetaEnumValue *AbstractMetaClass::findEnumValue(const QString &enumValueName)
{
    for (AbstractMetaEnum *e : qAsConst(m_enums)) {
        if (AbstractMetaEnumValue *v = e->findEnumValue(enumValueName))
            return v;
    }
    if (baseClass())
        return baseClass()->findEnumValue(enumValueName);

    return nullptr;
}


static void addExtraIncludeForType(AbstractMetaClass *metaClass, const AbstractMetaType *type)
{
    if (!type)
        return;

    Q_ASSERT(metaClass);
    const TypeEntry *entry = (type ? type->typeEntry() : nullptr);
    if (entry && entry->isComplex()) {
        const auto *centry = static_cast<const ComplexTypeEntry *>(entry);
        ComplexTypeEntry *class_entry = metaClass->typeEntry();
        if (class_entry && centry->include().isValid())
            class_entry->addExtraInclude(centry->include());
    }

    if (type->hasInstantiations()) {
        const AbstractMetaTypeList &instantiations = type->instantiations();
        for (const AbstractMetaType *instantiation : instantiations)
            addExtraIncludeForType(metaClass, instantiation);
    }
}

static void addExtraIncludesForFunction(AbstractMetaClass *metaClass, const AbstractMetaFunction *meta_function)
{
    Q_ASSERT(metaClass);
    Q_ASSERT(meta_function);
    addExtraIncludeForType(metaClass, meta_function->type());

    const AbstractMetaArgumentList &arguments = meta_function->arguments();
    for (AbstractMetaArgument *argument : arguments)
        addExtraIncludeForType(metaClass, argument->type());
}

void AbstractMetaClass::fixFunctions()
{
    if (m_functionsFixed)
        return;

    m_functionsFixed = true;

    AbstractMetaFunctionList funcs = functions();

    for (auto superClass : m_baseClasses) {
        superClass->fixFunctions();
        // Since we always traverse the complete hierarchy we are only
        // interrested in what each super class implements, not what
        // we may have propagated from their base classes again.
        AbstractMetaFunctionList superFuncs;
        // Super classes can never be final
        if (superClass->isFinalInTargetLang()) {
            qCWarning(lcShiboken).noquote().nospace()
                << "Final class '" << superClass->name() << "' set to non-final, as it is extended by other classes";
            *superClass -= AbstractMetaAttributes::FinalInTargetLang;
        }
        superFuncs = superClass->queryFunctions(AbstractMetaClass::ClassImplements);
        AbstractMetaFunctionList virtuals = superClass->queryFunctions(AbstractMetaClass::VirtualInCppFunctions);
        superFuncs += virtuals;

        QSet<AbstractMetaFunction *> funcsToAdd;
        for (auto sf : qAsConst(superFuncs)) {
            if (sf->isRemovedFromAllLanguages(sf->implementingClass()))
                continue;

            // skip functions added in base classes
            if (sf->isUserAdded() && sf->declaringClass() != this)
                continue;

            // we generally don't care about private functions, but we have to get the ones that are
            // virtual in case they override abstract functions.
            bool add = (sf->isNormal() || sf->isSignal() || sf->isEmptyFunction());
            for (AbstractMetaFunction *f : funcs) {
                if (f->isRemovedFromAllLanguages(f->implementingClass()))
                    continue;


                const AbstractMetaFunction::CompareResult cmp = f->compareTo(sf);

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
                            QString warn = QStringLiteral("visibility of function '%1' modified in class '%2'")
                                                          .arg(f->name(), name());
                            qCWarning(lcShiboken).noquote().nospace() << warn;
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
                                    qCWarning(lcShiboken).noquote().nospace()
                                        << QStringLiteral("Shadowing: %1::%2 and %3::%4")
                                           .arg(sf->implementingClass()->name(), sf->signature(),
                                                f->implementingClass()->name(), f->signature());
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

        for (AbstractMetaFunction *f : qAsConst(funcsToAdd)) {
            AbstractMetaFunction *copy = f->copy();
            (*copy) += AddedMethod;
            funcs.append(copy);
        }
    }

    bool hasPrivateConstructors = false;
    bool hasPublicConstructors = false;
    for (AbstractMetaFunction *func : qAsConst(funcs)) {
        const FunctionModificationList &mods = func->modifications(this);
        for (const FunctionModification &mod : mods) {
            if (mod.isRenameModifier()) {
                func->setName(mod.renamedTo());
            }
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

        if (!func->isRemovedFrom(this, TypeSystem::ShellCode))
            addExtraIncludesForFunction(this, func);
    }

    if (hasPrivateConstructors && !hasPublicConstructors) {
        (*this) += AbstractMetaAttributes::Abstract;
        (*this) -= AbstractMetaAttributes::FinalInTargetLang;
    }

    setFunctions(funcs);
}

static inline QString formatArraySize(int e)
{
    QString result;
    result += QLatin1Char('[');
    if (e >= 0)
        result += QString::number(e);
    result += QLatin1Char(']');
    return result;
}

QString AbstractMetaType::formatSignature(bool minimal) const
{
    QString result;
    if (isConstant())
        result += QLatin1String("const ");
    if (isVolatile())
        result += QLatin1String("volatile ");
    if (isArray()) {
        // Build nested array dimensions a[2][3] in correct order
        result += m_arrayElementType->minimalSignature();
        const int arrayPos = result.indexOf(QLatin1Char('['));
        if (arrayPos != -1)
            result.insert(arrayPos, formatArraySize(m_arrayElementCount));
        else
            result.append(formatArraySize(m_arrayElementCount));
    } else {
        result += typeEntry()->qualifiedCppName();
    }
    if (!m_instantiations.isEmpty()) {
        result += QLatin1Char('<');
        if (minimal)
            result += QLatin1Char(' ');
        for (int i = 0, size = m_instantiations.size(); i < size; ++i) {
            if (i > 0)
                result += QLatin1Char(',');
            result += m_instantiations.at(i)->minimalSignature();
        }
        result += QLatin1String(" >");
    }

    if (!minimal && (!m_indirections.isEmpty() || m_referenceType != NoReference))
        result += QLatin1Char(' ');
    for (Indirection i : m_indirections)
        result += TypeInfo::indirectionKeyword(i);
    switch (referenceType()) {
    case NoReference:
        break;
    case LValueReference:
        result += QLatin1Char('&');
        break;
    case RValueReference:
        result += QLatin1String("&&");
        break;
    }
    return result;
}

QString AbstractMetaType::formatPythonSignature() const
{
    /*
     * This is a version of the above, more suitable for Python.
     * We avoid extra keywords that are not needed in Python.
     * We prepend the package name, unless it is a primitive type.
     *
     * Primitive types like 'int', 'char' etc.:
     * When we have a primitive with an indirection, we use that '*'
     * character for later postprocessing, since those indirections
     * need to be modified into a result tuple.
     */
    QString result;
    if (m_pattern == AbstractMetaType::NativePointerAsArrayPattern)
        result += QLatin1String("array ");
    // We no longer use the "const" qualifier for heuristics. Instead,
    // NativePointerAsArrayPattern indicates when we have <array> in XML.
    // if (m_typeEntry->isPrimitive() && isConstant())
    //     result += QLatin1String("const ");
    if (!m_typeEntry->isPrimitive() && !package().isEmpty())
        result += package() + QLatin1Char('.');
    if (isArray()) {
        // Build nested array dimensions a[2][3] in correct order
        result += m_arrayElementType->formatPythonSignature();
        const int arrayPos = result.indexOf(QLatin1Char('['));
        if (arrayPos != -1)
            result.insert(arrayPos, formatArraySize(m_arrayElementCount));
        else
            result.append(formatArraySize(m_arrayElementCount));
    } else {
        result += typeEntry()->targetLangName();
    }
    if (!m_instantiations.isEmpty()) {
        result += QLatin1Char('[');
        for (int i = 0, size = m_instantiations.size(); i < size; ++i) {
            if (i > 0)
                result += QLatin1String(", ");
            result += m_instantiations.at(i)->formatPythonSignature();
        }
        result += QLatin1Char(']');
    }
    if (m_typeEntry->isPrimitive())
        for (Indirection i : m_indirections)
            result += TypeInfo::indirectionKeyword(i);
    // If it is a flags type, we replace it with the full name:
    // "PySide2.QtCore.Qt.ItemFlags" instead of "PySide2.QtCore.QFlags<Qt.ItemFlag>"
    if (m_typeEntry->isFlags())
        result = fullName();
    result.replace(QLatin1String("::"), QLatin1String("."));
    return result;
}

bool AbstractMetaType::isCppPrimitive() const
{
    return m_pattern == PrimitivePattern && m_typeEntry->isCppPrimitive();
}

/*******************************************************************************
 * Other stuff...
 */


AbstractMetaEnum *AbstractMetaClass::findEnum(const AbstractMetaClassList &classes,
                                              const EnumTypeEntry *entry)
{
    Q_ASSERT(entry->isEnum());

    QString qualifiedName = entry->qualifiedCppName();
    int pos = qualifiedName.lastIndexOf(QLatin1String("::"));

    QString enumName;
    QString className;

    if (pos > 0) {
        enumName = qualifiedName.mid(pos + 2);
        className = qualifiedName.mid(0, pos);
    } else {
        enumName = qualifiedName;
        className = TypeDatabase::globalNamespaceClassName(entry);
    }

    AbstractMetaClass *metaClass = AbstractMetaClass::findClass(classes, className);
    if (!metaClass) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("AbstractMeta::findEnum(), unknown class '%1' in '%2'")
                              .arg(className, entry->qualifiedCppName());
        return nullptr;
    }

    return metaClass->findEnum(enumName);
}

AbstractMetaEnumValue *AbstractMetaClass::findEnumValue(const AbstractMetaClassList &classes,
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
        if (AbstractMetaEnumValue *enumValue = metaClass->findEnumValue(name))
            return enumValue;
    }

    qCWarning(lcShiboken).noquote().nospace()
        << QStringLiteral("no matching enum '%1'").arg(name);
    return nullptr;
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

#ifndef QT_NO_DEBUG_STREAM

void AbstractMetaClass::format(QDebug &d) const
{
    if (d.verbosity() > 2)
        d << static_cast<const void *>(this) << ", ";
    d << '"' << qualifiedCppName();
    if (const int count = m_templateArgs.size()) {
        for (int i = 0; i < count; ++i)
            d << (i ? ',' : '<') << m_templateArgs.at(i)->qualifiedCppName();
        d << '>';
    }
    d << '"';
    if (isNamespace())
        d << " [namespace]";
    if (attributes() & AbstractMetaAttributes::FinalCppClass)
        d << " [final]";
    if (attributes().testFlag(AbstractMetaAttributes::Deprecated))
        d << " [deprecated]";
    if (!m_baseClasses.isEmpty()) {
        d << ", inherits ";
        for (auto b : m_baseClasses)
            d << " \"" << b->name() << '"';
    }
    if (auto templateBase = templateBaseClass()) {
        const auto instantiatedTypes = templateBaseClassInstantiations();
        d << ", instantiates \"" << templateBase->name();
        for (int i = 0, count = instantiatedTypes.size(); i < count; ++i)
            d << (i ? ',' : '<') << instantiatedTypes.at(i)->name();
        d << ">\"";
    }
    if (const int count = m_propertySpecs.size()) {
        d << ", properties (" << count << "): [";
        for (int i = 0; i < count; ++i) {
            if (i)
                d << ", ";
            m_propertySpecs.at(i)->formatDebug(d);
        }
        d << ']';
    }
}

void AbstractMetaClass::formatMembers(QDebug &d) const
{
    if (!m_enums.isEmpty())
        d << ", enums[" << m_enums.size() << "]=" << m_enums;
    if (!m_functions.isEmpty()) {
        const int count = m_functions.size();
        d << ", functions=[" << count << "](";
        for (int i = 0; i < count; ++i) {
            if (i)
                d << ", ";
            formatMetaFunctionBrief(d, m_functions.at(i));
        }
        d << ')';
    }
    if (const int count = m_fields.size()) {
        d << ", fields=[" << count << "](";
        for (int i = 0; i < count; ++i) {
            if (i)
                d << ", ";
            formatMetaField(d, m_fields.at(i));
        }
        d << ')';
    }
}

SourceLocation AbstractMetaClass::sourceLocation() const
{
    return m_sourceLocation;
}

void AbstractMetaClass::setSourceLocation(const SourceLocation &sourceLocation)
{
    m_sourceLocation = sourceLocation;
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

/*******************************************************************************
* AbstractMetaEnum
*/

AbstractMetaEnum::AbstractMetaEnum() :
    m_hasQenumsDeclaration(false), m_signed(true)
{
}

AbstractMetaEnum::~AbstractMetaEnum()
{
    qDeleteAll(m_enumValues);
}

template <class String>
AbstractMetaEnumValue *findMatchingEnumValue(const AbstractMetaEnumValueList &list, const String &value)
{
    for (AbstractMetaEnumValue *enumValue : list) {
        if (enumValue->name() == value)
            return enumValue;
    }
    return nullptr;
}

// Find enum values for "enum Enum { e1 }" either for "e1" or "Enum::e1"
AbstractMetaEnumValue *AbstractMetaEnum::findEnumValue(const QString &value) const
{
    if (isAnonymous())
        return findMatchingEnumValue(m_enumValues, value);
    const int sepPos = value.indexOf(QLatin1String("::"));
    if (sepPos == -1)
        return findMatchingEnumValue(m_enumValues, value);
    return name() == QStringView{value}.left(sepPos)
        ? findMatchingEnumValue(m_enumValues, QStringView{value}.right(value.size() - sepPos - 2))
        : nullptr;
}

QString AbstractMetaEnum::name() const
{
    return m_typeEntry->targetLangEntryName();
}

QString AbstractMetaEnum::qualifier() const
{
    return m_typeEntry->targetLangQualifier();
}

QString AbstractMetaEnum::package() const
{
    return m_typeEntry->targetLangPackage();
}

bool QPropertySpec::isValid() const
{
    return m_type != nullptr && !m_name.isEmpty() && !m_read.isEmpty();
}

#ifndef QT_NO_DEBUG_STREAM
void QPropertySpec::formatDebug(QDebug &d) const
{
    d << '#' << m_index << " \"" << m_name << "\" (" << m_type->qualifiedCppName();
    for (int i = 0; i < m_indirections; ++i)
        d << '*';
    d << "), read=" << m_read;
    if (!m_write.isEmpty())
        d << ", write=" << m_write;
    if (!m_reset.isEmpty())
          d << ", reset=" << m_reset;
    if (!m_designable.isEmpty())
          d << ", designable=" << m_designable;
}

QDebug operator<<(QDebug d, const QPropertySpec &p)
{
    QDebugStateSaver s(d);
    d.noquote();
    d.nospace();
    d << "QPropertySpec(";
    p.formatDebug(d);
    d << ')';
    return d;
}
#endif // QT_NO_DEBUG_STREAM
