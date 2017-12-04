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

#include "abstractmetalang.h"
#include "reporthandler.h"
#include "typedatabase.h"
#include "typesystem.h"

#ifndef QT_NO_DEBUG_STREAM
#  include <QtCore/QMetaEnum>
#  include <QtCore/QMetaObject>
#endif

#include <QtCore/QRegularExpression>
#include <QtCore/QStack>

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

/*******************************************************************************
 * AbstractMetaVariable
 */

AbstractMetaVariable::AbstractMetaVariable(const AbstractMetaVariable &other)
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
 * AbstractMetaType
 */

AbstractMetaType::AbstractMetaType()
    :m_typeEntry(0),
    m_arrayElementCount(-1),
    m_arrayElementType(0),
    m_originalTemplateType(0),
    m_pattern(InvalidPattern),
    m_constant(false),
    m_cppInstantiation(true),
    m_indirections(0),
    m_reserved(0),
    m_referenceType(NoReference)
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

static QString lastNameSegment(QString name)
{
    const int index = name.lastIndexOf(QStringLiteral("::"));
    if (index >= 0)
        name.remove(0, index + 2);
    return name;
}

QString AbstractMetaType::name() const
{
    if (m_name.isNull())
        m_name = lastNameSegment(m_typeEntry->targetLangName());
    return m_name;
}

QString AbstractMetaType::fullName() const
{
    return m_typeEntry->qualifiedTargetLangName();
}

AbstractMetaType *AbstractMetaType::copy() const
{
    AbstractMetaType *cpy = new AbstractMetaType;

    cpy->setTypeUsagePattern(typeUsagePattern());
    cpy->setConstant(isConstant());
    cpy->setReferenceType(referenceType());
    cpy->setIndirections(indirections());
    cpy->setInstantiations(instantiations());
    cpy->setArrayElementCount(arrayElementCount());
    cpy->setOriginalTypeDescription(originalTypeDescription());
    cpy->setOriginalTemplateType(originalTemplateType() ? originalTemplateType()->copy() : 0);

    cpy->setArrayElementType(arrayElementType() ? arrayElementType()->copy() : 0);

    cpy->setTypeEntry(typeEntry());

    return cpy;
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

QString AbstractMetaType::cppSignature() const
{
    if (m_cachedCppSignature.isEmpty())
        m_cachedCppSignature = formatSignature(false);
    return m_cachedCppSignature;
}

AbstractMetaType::TypeUsagePattern AbstractMetaType::determineUsagePattern() const
{
    if (m_typeEntry->isTemplateArgument() || m_referenceType == RValueReference)
        return InvalidPattern;

    if (m_typeEntry->isPrimitive() && (!actualIndirections()
        || (isConstant() && m_referenceType == LValueReference && !indirections()))) {
        return PrimitivePattern;
    }

    if (m_typeEntry->isVoid())
        return NativePointerPattern;

    if (m_typeEntry->isVarargs())
        return VarargsPattern;

    if (m_typeEntry->isString() && indirections() == 0
        && (isConstant() == (m_referenceType == LValueReference)
        || isConstant())) {
        return StringPattern;
    }

    if (m_typeEntry->isChar()
        && indirections() == 0
        && isConstant() == (m_referenceType == LValueReference)) {
        return CharPattern;
    }

    if (m_typeEntry->isJObjectWrapper()
        && indirections() == 0
        && isConstant() == (m_referenceType == LValueReference)) {
        return JObjectWrapperPattern;
    }

    if (m_typeEntry->isVariant()
        && indirections() == 0
        && isConstant() == (m_referenceType == LValueReference)) {
        return VariantPattern;
    }

    if (m_typeEntry->isEnum() && actualIndirections() == 0)
        return EnumPattern;

    if (m_typeEntry->isObject()) {
        if (indirections() == 0 && m_referenceType == NoReference)
            return ValuePattern;
        return static_cast<const ComplexTypeEntry *>(m_typeEntry)->isQObject()
            ? QObjectPattern : ObjectPattern;
    }

    if (m_typeEntry->isContainer() && indirections() == 0)
        return ContainerPattern;

    if (m_typeEntry->isSmartPointer() && indirections() == 0)
        return SmartPointerPattern;

    if (m_typeEntry->isFlags() && indirections() == 0
        && (isConstant() == (m_referenceType == LValueReference)))
        return FlagsPattern;

    if (m_typeEntry->isArray())
        return ArrayPattern;

    if (m_typeEntry->isThread()) {
        Q_ASSERT(indirections() == 1);
        return ThreadPattern;
    }

    if (m_typeEntry->isValue())
        return indirections() == 1 ? ValuePointerPattern : ValuePattern;

    if (ReportHandler::isDebug(ReportHandler::FullDebug)) {
        qCDebug(lcShiboken)
            << QStringLiteral("native pointer pattern for '%1'").arg(cppSignature());
    }
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
        pattern = static_cast<const ComplexTypeEntry *>(m_typeEntry)->isQObject()
            ? QObjectPattern : ObjectPattern;
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
            if (at->indirections())
                d << ", indirections=" << at->indirections();
            if (at->referenceType())
                d << ", reftype=" << at->referenceType();
            if (at->isConstant())
                d << ", [const]";
            if (at->isArray()) {
                d << ", array of \"" << at->arrayElementType()->cppSignature()
                    << "\", arrayElementCount="  << at->arrayElementCount();
            }
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
AbstractMetaArgument *AbstractMetaArgument::copy() const
{
    return new AbstractMetaArgument(*this);
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

bool AbstractMetaFunction::needsCallThrough() const
{
    if (ownerClass()->isInterface())
        return false;
    if (referenceCounts(implementingClass()).size() > 0)
        return true;
    if (argumentsHaveNativeId() || !isStatic())
        return true;

    for (const AbstractMetaArgument *arg : m_arguments) {
        if (arg->type()->isArray() || arg->type()->isTargetLangEnum() || arg->type()->isTargetLangFlags())
            return true;
    }

    if (type() && (type()->isArray() || type()->isTargetLangEnum() || type()->isTargetLangFlags()))
        return true;

    for (int i = -1; i <= arguments().size(); ++i) {
        TypeSystem::Ownership owner = this->ownership(implementingClass(), TypeSystem::TargetLangCode, i);
        if (owner != TypeSystem::InvalidOwnership)
            return true;
    }

    return false;
}

bool AbstractMetaFunction::needsSuppressUncheckedWarning() const
{
    for (int i = -1; i <= arguments().size(); ++i) {
        const QVector<ReferenceCount> &referenceCounts = this->referenceCounts(implementingClass(), i);
        for (const ReferenceCount &referenceCount : referenceCounts) {
            if (referenceCount.action != ReferenceCount::Set)
                return true;
        }
    }
    return false;
}

QString AbstractMetaFunction::marshalledName() const
{
    QString returned = QLatin1String("__qt_") + name();
    for (const AbstractMetaArgument *arg : m_arguments) {
        returned += QLatin1Char('_');
        if (arg->type()->isNativePointer()) {
            returned += QLatin1String("nativepointer");
        } else if (arg->type()->isIntegerEnum() || arg->type()->isIntegerFlags()) {
            returned += QLatin1String("int");
        } else {
            QString a = arg->type()->name();
            a.replace(QLatin1String("[]"), QLatin1String("_3"));
            a.replace(QLatin1Char('.'), QLatin1Char('_'));
            returned += a;
        }
    }
    return returned;
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
    CompareResult result = 0;

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
    AbstractMetaFunction *cpy = new AbstractMetaFunction;
    cpy->setName(name());
    cpy->setOriginalName(originalName());
    cpy->setOwnerClass(ownerClass());
    cpy->setImplementingClass(implementingClass());
    cpy->setFunctionType(functionType());
    cpy->setAttributes(attributes());
    cpy->setDeclaringClass(declaringClass());
    if (type())
        cpy->setType(type()->copy());
    cpy->setConstant(isConstant());
    cpy->setOriginalAttributes(originalAttributes());

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
    } else {
        QStringList returned;

        AbstractMetaArgument *argument = arguments.at(resolvedArguments.size());
        QStringList minimalTypeSignature = argument->type()->minimalSignature().split(QLatin1String("::"));
        for (int i = 0; i < minimalTypeSignature.size(); ++i) {
            returned += introspectionCompatibleSignatures(QStringList(resolvedArguments)
                                                          << QStringList(minimalTypeSignature.mid(minimalTypeSignature.size() - i - 1)).join(QLatin1String("::")));
        }

        return returned;
    }
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


QString AbstractMetaFunction::replacedDefaultExpression(const AbstractMetaClass *cls, int key) const
{
    const FunctionModificationList &modifications = this->modifications(cls);
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
            if (argumentModification.index == key
                && !argumentModification.replacedDefaultExpression.isEmpty()) {
                return argumentModification.replacedDefaultExpression;
            }
        }
    }

    return QString();
}

bool AbstractMetaFunction::removedDefaultExpression(const AbstractMetaClass *cls, int key) const
{
    const FunctionModificationList &modifications = this->modifications(cls);
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
            if (argumentModification.index == key
                && argumentModification.removedDefaultExpression) {
                return true;
            }
        }
    }

    return false;
}

bool AbstractMetaFunction::resetObjectAfterUse(int argumentIdx) const
{
    const AbstractMetaClass *cls = declaringClass();
    const FunctionModificationList &modifications = this->modifications(cls);
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
            if (argumentModification.index == argumentIdx && argumentModification.resetAfterUse)
                return true;
        }
    }

    return false;
}

QString AbstractMetaFunction::nullPointerDefaultValue(const AbstractMetaClass *mainClass, int argumentIdx) const
{
    Q_ASSERT(nullPointersDisabled(mainClass, argumentIdx));

    const AbstractMetaClass *cls = mainClass;
    if (!cls)
        cls = implementingClass();

    do {
        const FunctionModificationList &modifications = this->modifications(cls);
        for (const FunctionModification &modification : modifications) {
            for (const ArgumentModification &argumentModification : modification.argument_mods) {
                if (argumentModification.index == argumentIdx
                    && argumentModification.noNullPointers) {
                    return argumentModification.nullPointerDefaultValue;
                }
            }
        }
        cls = cls->baseClass();
    } while (cls && !mainClass); // Once when mainClass, or once for all base classes of implementing class

    return QString();

}

bool AbstractMetaFunction::nullPointersDisabled(const AbstractMetaClass *mainClass, int argumentIdx) const
{
    const AbstractMetaClass *cls = mainClass;
    if (!cls)
        cls = implementingClass();

    do {
        const FunctionModificationList &modifications = this->modifications(cls);
        for (const FunctionModification &modification : modifications) {
            for (const ArgumentModification &argumentModification : modification.argument_mods) {
                if (argumentModification.index == argumentIdx
                    && argumentModification.noNullPointers) {
                    return true;
                }
            }
        }

        cls = cls->baseClass();
    } while (cls && !mainClass); // Once when mainClass, or once for all base classes of implementing class

    return false;
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

QString AbstractMetaFunction::argumentReplaced(int key) const
{
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
            if (argumentModification.index == key && !argumentModification.replace_value.isEmpty())
                return argumentModification.replace_value;
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

bool AbstractMetaFunction::isVirtualSlot() const
{
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        if (modification.isVirtualSlot())
            return true;
    }

    return false;
}

bool AbstractMetaFunction::disabledGarbageCollection(const AbstractMetaClass *cls, int key) const
{
    typedef QHash<TypeSystem::Language, TypeSystem::Ownership>::const_iterator OwnershipMapIt;

    const FunctionModificationList &modifications = this->modifications(cls);
    for (const FunctionModification &modification : modifications) {
        for (const ArgumentModification &argumentModification : modification.argument_mods) {
            if (argumentModification.index != key)
                continue;

            for (OwnershipMapIt it = argumentModification.ownerships.cbegin(), end = argumentModification.ownerships.cend(); it != end; ++it) {
                if (it.value() == TypeSystem::CppOwnership)
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

bool AbstractMetaFunction::isThread() const
{
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        if (modification.isThread())
            return true;
    }
    return false;
}

bool AbstractMetaFunction::allowThread() const
{
    const FunctionModificationList &modifications = this->modifications(declaringClass());
    for (const FunctionModification &modification : modifications) {
        if (modification.allowThread())
            return true;
    }
    return false;
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

FunctionModificationList AbstractMetaFunction::modifications(const AbstractMetaClass* implementor) const
{
    if (!implementor)
        implementor = ownerClass();

    if (!implementor)
        return TypeDatabase::instance()->functionModifications(minimalSignature());

    FunctionModificationList mods;
    while (implementor) {
        mods += implementor->typeEntry()->functionModifications(minimalSignature());
        if ((implementor == implementor->baseClass()) ||
            (implementor == implementingClass() && (mods.size() > 0)))
                break;
        const AbstractMetaClassList &interfaces = implementor->interfaces();
        for (const AbstractMetaClass *interface : interfaces)
            mods += this->modifications(interface);
        implementor = implementor->baseClass();
    }
    return mods;
}

bool AbstractMetaFunction::hasModifications(const AbstractMetaClass *implementor) const
{
    return !modifications(implementor).isEmpty();
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

bool AbstractMetaFunction::isConversionOperator(QString funcName)
{
    static const QRegularExpression opRegEx(QStringLiteral("^operator(?:\\s+(?:const|volatile))?\\s+(\\w+\\s*)&?$"));
    Q_ASSERT(opRegEx.isValid());
    return opRegEx.match(funcName).hasMatch();
}

bool AbstractMetaFunction::isOperatorOverload(QString funcName)
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

QString AbstractMetaFunction::targetLangSignature(bool minimal) const
{
    QString s;

    // Attributes...
    if (!minimal) {
        // Return type
        if (type())
            s += type()->name() + QLatin1Char(' ');
        else
            s += QLatin1String("void ");
    }

    s += modifiedName();
    s += QLatin1Char('(');

    int j = 0;
    for (int i = 0; i < m_arguments.size(); ++i) {
        if (argumentRemoved(i + 1))
            continue;
        if (j) {
            s += QLatin1Char(',');
            if (!minimal)
                s += QLatin1Char(' ');
        }
        s += m_arguments.at(i)->type()->name();

        if (!minimal) {
            s += QLatin1Char(' ');
            s += m_arguments.at(i)->name();
        }
        ++j;
    }

    s += QLatin1Char(')');

    return s;
}


bool function_sorter(AbstractMetaFunction *a, AbstractMetaFunction *b)
{
    return a->signature() < b->signature();
}

#ifndef QT_NO_DEBUG_STREAM
static inline void formatMetaFunctionBrief(QDebug &d, const AbstractMetaFunction *af)
{
    d << '"' << af->debugSignature() << '"';
}

void AbstractMetaFunction::formatDebugVerbose(QDebug &d) const
{
    d << m_functionType << ' ' << m_type << ' ' << m_name << '(';
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
    if (m_userAdded)
        d << " [userAdded]";
    if (m_explicit)
        d << " [explicit]";
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        if (d.verbosity() > 2) {
            af->formatDebugVerbose(d);
        } else {
#endif
            d << "signature=";
            formatMetaFunctionBrief(d, af);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        }
#endif
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
 * Constructs an interface based on the functions and enums in this
 * class and returns it...
 */
AbstractMetaClass *AbstractMetaClass::extractInterface()
{
    Q_ASSERT(typeEntry()->designatedInterface());

    if (!m_extractedInterface) {
        AbstractMetaClass *iface = new AbstractMetaClass;
        iface->setAttributes(attributes());
        iface->setBaseClass(0);

        iface->setTypeEntry(typeEntry()->designatedInterface());

        for (AbstractMetaFunction *function : qAsConst(m_functions)) {
            if (!function->isConstructor())
                iface->addFunction(function->copy());
        }

//         iface->setEnums(enums());
//         setEnums(AbstractMetaEnumList());

        for (const AbstractMetaField *field : qAsConst(m_fields)) {
            if (field->isPublic()) {
                AbstractMetaField *new_field = field->copy();
                new_field->setEnclosingClass(iface);
                iface->addField(new_field);
            }
        }

        m_extractedInterface = iface;
        addInterface(iface);
    }

    return m_extractedInterface;
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

    // Interfaces don't implement functions
    if (isInterface())
        default_flags |= ClassImplements;

    // Only public functions in final classes
    // default_flags |= isFinal() ? WasPublic : 0;
    FunctionQueryOptions public_flags;
    if (isFinal())
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

AbstractMetaFunctionList AbstractMetaClass::virtualFunctions() const
{
    const AbstractMetaFunctionList &list = functionsInShellClass();

    AbstractMetaFunctionList returned;
    for (AbstractMetaFunction *f : list) {
        if (!f->isFinalInCpp() || f->isVirtualSlot())
            returned += f;
    }

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

bool AbstractMetaClass::hasOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isOperatorOverload() && !f->isPrivate())
            return true;
    }
    return false;
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

bool AbstractMetaClass::hasSubscriptOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isSubscriptOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasAssignmentOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isAssignmentOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasConversionOperatorOverload() const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->ownerClass() == f->implementingClass() && f->isConversionOperator() && !f->isPrivate())
            return true;
    }
    return false;
}

/*******************************************************************************
 * Returns a list of all functions that should be declared and implemented in
 * the shell class which is generated as a wrapper on top of the actual C++ class
 */
AbstractMetaFunctionList AbstractMetaClass::functionsInShellClass() const
{
    // Only functions and only protected and public functions
    FunctionQueryOptions default_flags = NormalFunctions | Visible | WasVisible | NotRemovedFromShell;

    // All virtual functions
    AbstractMetaFunctionList returned = queryFunctions(VirtualFunctions | default_flags);

    // All functions explicitly set to be implemented by the shell class
    // (mainly superclass functions that are hidden by other declarations)
    returned += queryFunctions(ForcedShellFunctions | default_flags);

    // All functions explicitly set to be virtual slots
    returned += queryFunctions(VirtualSlots | default_flags);

    return returned;
}

/*******************************************************************************
 * Returns a list of all functions that require a public override function to
 * be generated in the shell class. This includes all functions that were originally
 * protected in the superclass.
 */
AbstractMetaFunctionList AbstractMetaClass::publicOverrideFunctions() const
{
    return queryFunctions(NormalFunctions | WasProtected | FinalInCppFunctions | NotRemovedFromTargetLang)
           + queryFunctions(Signals | WasProtected | FinalInCppFunctions | NotRemovedFromTargetLang);
}

AbstractMetaFunctionList AbstractMetaClass::virtualOverrideFunctions() const
{
    return queryFunctions(NormalFunctions | NonEmptyFunctions | Visible | VirtualInCppFunctions | NotRemovedFromShell) +
           queryFunctions(Signals | NonEmptyFunctions | Visible | VirtualInCppFunctions | NotRemovedFromShell);
}

void AbstractMetaClass::sortFunctions()
{
    qSort(m_functions.begin(), m_functions.end(), function_sorter);
}

void AbstractMetaClass::setFunctions(const AbstractMetaFunctionList &functions)
{
    m_functions = functions;

    // Functions must be sorted by name before next loop
    sortFunctions();

    QString currentName;
    bool hasVirtuals = false;
    AbstractMetaFunctionList finalFunctions;
    for (AbstractMetaFunction *f : qAsConst(m_functions)) {
        f->setOwnerClass(this);

        m_hasVirtualSlots = m_hasVirtualSlots || f->isVirtualSlot();
        m_hasVirtuals = m_hasVirtuals || f->isVirtualSlot() || hasVirtualDestructor();
        m_isPolymorphic = m_isPolymorphic || m_hasVirtuals;
        m_hasNonpublic = m_hasNonpublic || !f->isPublic();

        // If we have non-virtual overloads of a virtual function, we have to implement
        // all the overloads in the shell class to override the hiding rule
        if (currentName == f->name()) {
            hasVirtuals = hasVirtuals || !f->isFinal();
            if (f->isFinal())
                finalFunctions += f;
        } else {
            if (hasVirtuals && finalFunctions.size() > 0) {
                for (AbstractMetaFunction *final_function : qAsConst(finalFunctions)) {
                    *final_function += AbstractMetaAttributes::ForceShellImplementation;

                    qCWarning(lcShiboken).noquote().nospace()
                        << QStringLiteral("hiding of function '%1' in class '%2'")
                                          .arg(final_function->name(), name());
                }
            }

            hasVirtuals = !f->isFinal();
            finalFunctions.clear();
            if (f->isFinal())
                finalFunctions += f;
            currentName = f->name();
        }
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

    m_hasVirtualSlots |= function->isVirtualSlot();
    m_hasVirtuals |= !function->isFinal() || function->isVirtualSlot() || hasVirtualDestructor();
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
    return lastNameSegment(m_typeEntry->targetLangName());
}

void AbstractMetaClass::setBaseClass(AbstractMetaClass *baseClass)
{
    m_baseClass = baseClass;
    if (baseClass)
        m_isPolymorphic |= baseClass->isPolymorphic();
}

QString AbstractMetaClass::package() const
{
    return m_typeEntry->targetLangPackage();
}

bool AbstractMetaClass::isInterface() const
{
    return m_typeEntry->isInterface();
}

bool AbstractMetaClass::isNamespace() const
{
    return m_typeEntry->isNamespace();
}

bool AbstractMetaClass::isQObject() const
{
    return m_typeEntry->isQObject();
}

QString AbstractMetaClass::qualifiedCppName() const
{
    return m_typeEntry->qualifiedCppName();
}

bool AbstractMetaClass::hasFunction(const QString &str) const
{
    return findFunction(str);
}

const AbstractMetaFunction* AbstractMetaClass::findFunction(const QString& functionName) const
{
    for (const AbstractMetaFunction *f : m_functions) {
        if (f->name() == functionName)
            return f;
    }
    return 0;
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

bool AbstractMetaClass::generateShellClass() const
{
    return m_forceShellClass ||
           (!isFinal()
            && (hasVirtualFunctions()
                || hasProtectedFunctions()
                || hasFieldAccessors()));
}

QPropertySpec *AbstractMetaClass::propertySpecForRead(const QString &name) const
{
    for (int i = 0; i < m_propertySpecs.size(); ++i)
        if (name == m_propertySpecs.at(i)->read())
            return m_propertySpecs.at(i);
    return 0;
}

QPropertySpec *AbstractMetaClass::propertySpecForWrite(const QString &name) const
{
    for (int i = 0; i < m_propertySpecs.size(); ++i)
        if (name == m_propertySpecs.at(i)->write())
            return m_propertySpecs.at(i);
    return 0;
}

QPropertySpec *AbstractMetaClass::propertySpecForReset(const QString &name) const
{
    for (int i = 0; i < m_propertySpecs.size(); ++i) {
        if (name == m_propertySpecs.at(i)->reset())
            return m_propertySpecs.at(i);
    }
    return 0;
}

typedef QHash<const AbstractMetaClass*, AbstractMetaTypeList> AbstractMetaClassBaseTemplateInstantiationsMap;
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

void AbstractMetaClass::setTemplateBaseClassInstantiations(AbstractMetaTypeList& instantiations)
{
    if (!templateBaseClass())
        return;
    metaClassBaseTemplateInstantiations()->insert(this, instantiations);
}

static bool functions_contains(const AbstractMetaFunctionList &l, const AbstractMetaFunction *func)
{
    for (const AbstractMetaFunction *f : l) {
        if ((f->compareTo(func) & AbstractMetaFunction::PrettySimilar) == AbstractMetaFunction::PrettySimilar)
            return true;
    }
    return false;
}

AbstractMetaField::AbstractMetaField() : m_getter(0), m_setter(0), m_class(0)
{
}

AbstractMetaField::~AbstractMetaField()
{
    delete m_setter;
    delete m_getter;
}

AbstractMetaField *AbstractMetaField::copy() const
{
    AbstractMetaField *returned = new AbstractMetaField;
    returned->setEnclosingClass(0);
    returned->setAttributes(attributes());
    returned->setName(name());
    returned->setType(type()->copy());
    returned->setOriginalAttributes(originalAttributes());

    return returned;
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
    AbstractMetaFunction *f = new AbstractMetaFunction;

    f->setName(name);
    f->setOriginalName(name);
    f->setOwnerClass(g->enclosingClass());
    f->setImplementingClass(g->enclosingClass());
    f->setDeclaringClass(g->enclosingClass());

    AbstractMetaAttributes::Attributes attr = AbstractMetaAttributes::Final | type;
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
        AbstractMetaArgument *argument = new AbstractMetaArgument;
        argument->setType(type()->copy());
        argument->setName(name());
        arguments.append(argument);
        m_setter->setArguments(arguments);
    }
    return m_setter;
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
        d << ae->fullName() << '[';
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
    return queryFunctions(Constructors).size();
}

bool AbstractMetaClass::hasCopyConstructor() const
{
    const AbstractMetaFunctionList &ctors = queryFunctions(Constructors);
    for (const AbstractMetaFunction* ctor : ctors) {
        if (ctor->functionType() == AbstractMetaFunction::CopyConstructorFunction)
            return true;
    }
    return false;
}

bool AbstractMetaClass::hasPrivateCopyConstructor() const
{
    const AbstractMetaFunctionList &ctors = queryFunctions(Constructors);
    for (const AbstractMetaFunction *ctor : ctors) {
        if (ctor->functionType() == AbstractMetaFunction::CopyConstructorFunction && ctor->isPrivate())
            return true;
    }
    return false;
}

void AbstractMetaClass::addDefaultConstructor()
{
    AbstractMetaFunction *f = new AbstractMetaFunction;
    f->setOriginalName(name());
    f->setName(name());
    f->setOwnerClass(this);
    f->setFunctionType(AbstractMetaFunction::ConstructorFunction);
    f->setArguments(AbstractMetaArgumentList());
    f->setDeclaringClass(this);

    f->setAttributes(AbstractMetaAttributes::Public | AbstractMetaAttributes::Final);
    f->setImplementingClass(this);
    f->setOriginalAttributes(f->attributes());

    addFunction(f);
    this->setHasNonPrivateConstructor(true);
}

void AbstractMetaClass::addDefaultCopyConstructor(bool isPrivate)
{
    AbstractMetaFunction* f = new AbstractMetaFunction;
    f->setOriginalName(name());
    f->setName(name());
    f->setOwnerClass(this);
    f->setFunctionType(AbstractMetaFunction::CopyConstructorFunction);
    f->setDeclaringClass(this);

    AbstractMetaType* argType = new AbstractMetaType;
    argType->setTypeEntry(typeEntry());
    argType->setReferenceType(LValueReference);
    argType->setConstant(true);
    argType->setTypeUsagePattern(AbstractMetaType::ValuePattern);

    AbstractMetaArgument* arg = new AbstractMetaArgument;
    arg->setType(argType);
    arg->setName(name());
    f->addArgument(arg);

    AbstractMetaAttributes::Attributes attr = AbstractMetaAttributes::Final;
    if (isPrivate)
        attr |= AbstractMetaAttributes::Private;
    else
        attr |= AbstractMetaAttributes::Public;
    f->setAttributes(attr);
    f->setImplementingClass(this);
    f->setOriginalAttributes(f->attributes());

    addFunction(f);
}

bool AbstractMetaClass::hasFunction(const AbstractMetaFunction *f) const
{
    return functions_contains(m_functions, f);
}

/* Goes through the list of functions and returns a list of all
   functions matching all of the criteria in \a query.
 */

AbstractMetaFunctionList AbstractMetaClass::queryFunctions(FunctionQueryOptions query) const
{
    AbstractMetaFunctionList functions;

    for (AbstractMetaFunction *f : m_functions) {

        if ((query & VirtualSlots) && !f->isVirtualSlot())
            continue;

        if ((query & NotRemovedFromTargetLang) && f->isRemovedFrom(f->implementingClass(), TypeSystem::TargetLangCode))
            continue;

        if ((query & NotRemovedFromTargetLang) && !f->isFinal() && f->isRemovedFrom(f->declaringClass(), TypeSystem::TargetLangCode))
            continue;

        if ((query & NotRemovedFromShell) && f->isRemovedFrom(f->implementingClass(), TypeSystem::ShellCode))
            continue;

        if ((query & NotRemovedFromShell) && !f->isFinal() && f->isRemovedFrom(f->declaringClass(), TypeSystem::ShellCode))
            continue;

        if ((query & Visible) && f->isPrivate())
            continue;

        if ((query & VirtualInTargetLangFunctions) && f->isFinalInTargetLang())
            continue;

        if ((query & Invisible) && !f->isPrivate())
            continue;

        if ((query & Empty) && !f->isEmptyFunction())
            continue;

        if ((query & WasPublic) && !f->wasPublic())
            continue;

        if ((query & WasVisible) && f->wasPrivate())
            continue;

        if ((query & WasProtected) && !f->wasProtected())
            continue;

        if ((query & ClassImplements) && f->ownerClass() != f->implementingClass())
            continue;

        if ((query & Inconsistent) && (f->isFinalInTargetLang() || !f->isFinalInCpp() || f->isStatic()))
            continue;

        if ((query & FinalInTargetLangFunctions) && !f->isFinalInTargetLang())
            continue;

        if ((query & FinalInCppFunctions) && !f->isFinalInCpp())
            continue;

        if ((query & VirtualInCppFunctions) && f->isFinalInCpp())
            continue;

        if ((query & Signals) && (!f->isSignal()))
            continue;

        if ((query & ForcedShellFunctions) &&
            (!f->isForcedShellImplementation() || !f->isFinal())) {
            continue;
        }

        if ((query & Constructors) && (!f->isConstructor() || f->ownerClass() != f->implementingClass()))
            continue;

        if (!(query & Constructors) && f->isConstructor())
            continue;

        // Destructors are never included in the functions of a class currently
        /*
           if ((query & Destructors) && (!f->isDestructor()
                                       || f->ownerClass() != f->implementingClass())
            || f->isDestructor() && (query & Destructors) == 0) {
            continue;
        }*/

        if ((query & VirtualFunctions) && (f->isFinal() || f->isSignal() || f->isStatic()))
            continue;

        if ((query & StaticFunctions) && (!f->isStatic() || f->isSignal()))
            continue;

        if ((query & NonStaticFunctions) && (f->isStatic()))
            continue;

        if ((query & NonEmptyFunctions) && (f->isEmptyFunction()))
            continue;

        if ((query & NormalFunctions) && (f->isSignal()))
            continue;

        if ((query & AbstractFunctions) && !f->isAbstract())
            continue;

        if ((query & OperatorOverloads) && !f->isOperatorOverload())
            continue;

        functions << f;
    }

    return functions;
}

bool AbstractMetaClass::hasSignals() const
{
    return cppSignalFunctions().size() > 0;
}


/**
 * Adds the specified interface to this class by adding all the
 * functions in the interface to this class.
 */
void AbstractMetaClass::addInterface(AbstractMetaClass *interface)
{
    Q_ASSERT(!m_interfaces.contains(interface));
    m_interfaces << interface;

    m_isPolymorphic |= interface->isPolymorphic();

    if (m_extractedInterface && m_extractedInterface != interface)
        m_extractedInterface->addInterface(interface);


#if 0
    const AbstractMetaFunctionList &funcs = interface->functions();
    for (AbstractMetaFunction *function : funcs)
    if (!hasFunction(function) && !function->isConstructor()) {
        AbstractMetaFunction *cpy = function->copy();
        cpy->setImplementingClass(this);

        // Setup that this function is an interface class.
        cpy->setInterfaceClass(interface);
        *cpy += AbstractMetaAttributes::InterfaceFunction;

        // Copy the modifications in interface into the implementing classes.
        const FunctionModificationList &mods = function->modifications(interface);
        for (const FunctionModification &mod : mods)
            m_typeEntry->addFunctionModification(mod);

        // It should be mostly safe to assume that when we implement an interface
        // we don't "pass on" pure virtual functions to our sublcasses...
//             *cpy -= AbstractMetaAttributes::Abstract;

        addFunction(cpy);
    }
#endif

}


void AbstractMetaClass::setInterfaces(const AbstractMetaClassList &interfaces)
{
    m_interfaces = interfaces;
    for (const AbstractMetaClass *interface : interfaces) {
        if (interface)
            m_isPolymorphic |= interface->isPolymorphic();
    }
}


AbstractMetaEnum *AbstractMetaClass::findEnum(const QString &enumName)
{
    for (AbstractMetaEnum *e : qAsConst(m_enums)) {
        if (e->name() == enumName)
            return e;
    }

    if (typeEntry()->designatedInterface())
        return extractInterface()->findEnum(enumName);

    return 0;
}




/*!  Recursivly searches for the enum value named \a enumValueName in
  this class and its superclasses and interfaces. Values belonging to
  \a meta_enum are excluded from the search.
*/
AbstractMetaEnumValue *AbstractMetaClass::findEnumValue(const QString &enumValueName, AbstractMetaEnum *meta_enum)
{
    for (AbstractMetaEnum *e : qAsConst(m_enums)) {
        if (e != meta_enum)
            continue;
        const AbstractMetaEnumValueList &values = e->values();
        for (AbstractMetaEnumValue *v : values) {
            if (v->name() == enumValueName)
                return v;
        }
    }

    if (typeEntry()->designatedInterface())
        return extractInterface()->findEnumValue(enumValueName, meta_enum);

    if (baseClass())
        return baseClass()->findEnumValue(enumValueName, meta_enum);

    return 0;
}


/*!
 * Searches through all of this class' enums for a value matching the
 * name \a enumValueName. The name is excluding the class/namespace
 * prefix. The function recursivly searches interfaces and baseclasses
 * of this class.
 */
AbstractMetaEnum *AbstractMetaClass::findEnumForValue(const QString &enumValueName)
{
    for (AbstractMetaEnum *e : qAsConst(m_enums)) {
        const AbstractMetaEnumValueList &values = e->values();
        for (AbstractMetaEnumValue *v : values) {
            if (v->name() == enumValueName)
                return e;
        }
    }

    if (typeEntry()->designatedInterface())
        return extractInterface()->findEnumForValue(enumValueName);

    if (baseClass())
        return baseClass()->findEnumForValue(enumValueName);

    return 0;
}


static void addExtraIncludeForType(AbstractMetaClass *metaClass, const AbstractMetaType *type)
{
    if (!type)
        return;

    Q_ASSERT(metaClass);
    const TypeEntry *entry = (type ? type->typeEntry() : 0);
    if (entry && entry->isComplex()) {
        const ComplexTypeEntry *centry = static_cast<const ComplexTypeEntry *>(entry);
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
    else
        m_functionsFixed = true;

    AbstractMetaClass *superClass = baseClass();
    AbstractMetaFunctionList funcs = functions();

    if (superClass)
        superClass->fixFunctions();
    int iface_idx = 0;
    while (superClass || iface_idx < interfaces().size()) {
        // Since we always traverse the complete hierarchy we are only
        // interrested in what each super class implements, not what
        // we may have propagated from their base classes again.
        AbstractMetaFunctionList superFuncs;
        if (superClass) {
            // Super classes can never be final
            if (superClass->isFinalInTargetLang()) {
                qCWarning(lcShiboken).noquote().nospace()
                    << "Final class '" << superClass->name() << "' set to non-final, as it is extended by other classes";
                *superClass -= AbstractMetaAttributes::FinalInTargetLang;
            }
            superFuncs = superClass->queryFunctions(AbstractMetaClass::ClassImplements);
            AbstractMetaFunctionList virtuals = superClass->queryFunctions(AbstractMetaClass::VirtualInCppFunctions);
            superFuncs += virtuals;
        } else {
            superFuncs = interfaces().at(iface_idx)->queryFunctions(AbstractMetaClass::NormalFunctions);
            AbstractMetaFunctionList virtuals = interfaces().at(iface_idx)->queryFunctions(AbstractMetaClass::VirtualInCppFunctions);
            superFuncs += virtuals;
        }

        QSet<AbstractMetaFunction *> funcsToAdd;
        for (int sfi = 0; sfi < superFuncs.size(); ++sfi) {
            AbstractMetaFunction *sf = superFuncs.at(sfi);

            if (sf->isRemovedFromAllLanguages(sf->implementingClass()))
                continue;

            // skip functions added in base classes
            if (sf->isUserAdded() && sf->declaringClass() != this)
                continue;

            // we generally don't care about private functions, but we have to get the ones that are
            // virtual in case they override abstract functions.
            bool add = (sf->isNormal() || sf->isSignal() || sf->isEmptyFunction());
            for (int fi = 0; fi < funcs.size(); ++fi) {
                AbstractMetaFunction *f = funcs.at(fi);
                if (f->isRemovedFromAllLanguages(f->implementingClass()))
                    continue;


                const AbstractMetaFunction::CompareResult cmp = f->compareTo(sf);

                if (cmp & AbstractMetaFunction::EqualModifiedName) {
                    add = false;
                    if (cmp & AbstractMetaFunction::EqualArguments) {
                        // Same function, propegate virtual...
                        if (!(cmp & AbstractMetaFunction::EqualAttributes)) {
                            if (!f->isEmptyFunction()) {
                                if (!sf->isFinalInCpp() && f->isFinalInCpp()) {
                                    *f -= AbstractMetaAttributes::FinalInCpp;
                                }
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
                                *f += AbstractMetaAttributes::FinalInCpp;
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
                                    } else if (mod.isPrivate()) {
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

        for (AbstractMetaFunction *f : qAsConst(funcsToAdd))
            funcs << f->copy();

        if (superClass)
            superClass = superClass->baseClass();
        else
            iface_idx++;
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
            (*this) -= AbstractMetaAttributes::Final;
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
        (*this) -= AbstractMetaAttributes::Final;
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

    if (!minimal && (m_indirections != 0 || m_referenceType != NoReference))
        result += QLatin1Char(' ');
    if (m_indirections)
        result += QString(m_indirections, QLatin1Char('*'));
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

bool AbstractMetaType::hasNativeId() const
{
    return (isQObject() || isValue() || isObject()) && typeEntry()->isNativeIdBased();
}

bool AbstractMetaType::isCppPrimitive() const
{
    return m_pattern == PrimitivePattern && m_typeEntry->isCppPrimitive();
}

bool AbstractMetaType::isTargetLangEnum() const
{
    return isEnum() && !static_cast<const EnumTypeEntry *>(typeEntry())->forceInteger();
}

bool AbstractMetaType::isTargetLangFlags() const
{
    return isFlags() && !static_cast<const FlagsTypeEntry *>(typeEntry())->forceInteger();
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
        return 0;
    }

    return metaClass->findEnum(enumName);
}

AbstractMetaEnumValue *AbstractMetaClass::findEnumValue(const AbstractMetaClassList &classes,
                                                        const QString &name)
{
    QStringList lst = name.split(QLatin1String("::"));

    if (lst.size() > 1) {
        QString prefixName = lst.at(0);
        QString enumName = lst.at(1);

        AbstractMetaClass* cl = findClass(classes, prefixName);
        if (cl)
            return cl->findEnumValue(enumName, 0);
    }

    for (AbstractMetaClass *metaClass : classes) {
        const AbstractMetaEnumList &enums = metaClass->enums();
        for (AbstractMetaEnum *metaEnum : enums) {
            AbstractMetaEnumValue* enumValue = metaClass->findEnumValue(name, metaEnum);
            if (enumValue)
                return enumValue;
        }
    }

    qCWarning(lcShiboken).noquote().nospace()
        << QStringLiteral("no matching enum '%1'").arg(name);
    return 0;
}

/*!
 * Searches the list after a class that mathces \a name; either as
 * C++, Target language base name or complete Target language package.class name.
 */

AbstractMetaClass *AbstractMetaClass::findClass(const AbstractMetaClassList &classes,
                                                const QString &name)
{
    if (name.isEmpty())
        return 0;

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

    return 0;
}

AbstractMetaClass *AbstractMetaClass::findClass(const AbstractMetaClassList &classes,
                                                const TypeEntry* typeEntry)
{
    for (AbstractMetaClass* c : classes) {
        if (c->typeEntry() == typeEntry)
            return c;
    }
    return 0;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaClass *ac)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaClass(";
    if (ac) {
        d << '"' << ac->fullName() << '"';
        if (ac->attributes() & AbstractMetaAttributes::FinalCppClass)
            d << " [final]";
        if (ac->m_baseClass)
            d << ", inherits \"" << ac->m_baseClass->name() << '"';
        const AbstractMetaEnumList &enums = ac->enums();
        if (!enums.isEmpty())
            d << ", enums[" << enums.size() << "]=" << enums;
        const AbstractMetaFunctionList &functions = ac->functions();
        if (!functions.isEmpty()) {
            const int count = functions.size();
            d << ", functions=[" << count << "](";
            for (int i = 0; i < count; ++i) {
                if (i)
                    d << ", ";
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
                if (d.verbosity() > 2)
                    d << functions.at(i);
                else
#endif
                    formatMetaFunctionBrief(d, functions.at(i));
            }
            d << ')';
        }
        const AbstractMetaFieldList &fields = ac->fields();
        if (!fields.isEmpty()) {
            const int count = fields.size();
            d << ", fields=[" << count << "](";
            for (int i = 0; i < count; ++i) {
                if (i)
                    d << ", ";
                formatMetaField(d, fields.at(i));
            }
            d << ')';
        }
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

QString AbstractMetaEnum::name() const
{
    return m_typeEntry->targetLangName();
}

QString AbstractMetaEnum::qualifier() const
{
    return m_typeEntry->targetLangQualifier();
}

QString AbstractMetaEnum::package() const
{
    return m_typeEntry->targetLangPackage();
}

bool AbstractMetaEnum::isAnonymous() const
{
    return m_typeEntry->isAnonymous();
}
