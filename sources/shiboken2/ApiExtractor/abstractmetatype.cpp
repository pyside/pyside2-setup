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

#include "abstractmetatype.h"
#include "typedatabase.h"
#include "typesystem.h"
#include "parser/codemodel.h"

#ifndef QT_NO_DEBUG_STREAM
#  include <QtCore/QDebug>
#endif

#include <QtCore/QSharedData>
#include <QtCore/QSharedPointer>
#include <QtCore/QStack>

using AbstractMetaTypeCPtr = QSharedPointer<const AbstractMetaType>;

class AbstractMetaTypeData : public QSharedData
{
public:
    AbstractMetaTypeData(const TypeEntry *t);

    int actualIndirections() const;
    bool passByConstRef() const;
    bool passByValue() const;
    AbstractMetaType::TypeUsagePattern determineUsagePattern() const;
    bool hasTemplateChildren() const;
    QString formatSignature(bool minimal) const;
    QString formatPythonSignature() const;
    bool equals(const AbstractMetaTypeData &rhs) const;

    const TypeEntry *m_typeEntry;
    AbstractMetaTypeList m_instantiations;
    mutable QString m_cachedCppSignature;
    mutable QString m_cachedPythonSignature;
    QString m_originalTypeDescription;

    int m_arrayElementCount = -1;
    AbstractMetaTypeCPtr m_arrayElementType;
    AbstractMetaTypeCPtr m_originalTemplateType;
    AbstractMetaTypeCPtr m_viewOn;
    AbstractMetaType::Indirections m_indirections;

    AbstractMetaType::TypeUsagePattern m_pattern = AbstractMetaType::InvalidPattern;
    uint m_constant : 1;
    uint m_volatile : 1;
    uint m_signaturesDirty : 1;
    uint m_reserved : 29; // unused

    ReferenceType m_referenceType = NoReference;
    AbstractMetaTypeList m_children;
};

AbstractMetaTypeData::AbstractMetaTypeData(const TypeEntry *t) :
    m_typeEntry(t),
    m_constant(false),
    m_volatile(false),
    m_signaturesDirty(true),
    m_reserved(0)
{
}

AbstractMetaType::AbstractMetaType(const TypeEntry *t) : d(new AbstractMetaTypeData(t))
{
}

AbstractMetaType::AbstractMetaType() : d(new AbstractMetaTypeData(nullptr))
{
}

AbstractMetaType &AbstractMetaType::operator=(const AbstractMetaType &) = default;

AbstractMetaType::AbstractMetaType(const AbstractMetaType &rhs) = default;

AbstractMetaType::AbstractMetaType(AbstractMetaType &&) = default;

AbstractMetaType &AbstractMetaType::operator=(AbstractMetaType &&) = default;

AbstractMetaType::~AbstractMetaType() = default;

bool AbstractMetaType::isValid() const
{
    return d->m_pattern != AbstractMetaType::InvalidPattern;
}

QString AbstractMetaType::package() const
{
    return d->m_typeEntry->targetLangPackage();
}

QString AbstractMetaType::name() const
{
    return d->m_typeEntry->targetLangEntryName();
}

QString AbstractMetaType::fullName() const
{
    return d->m_typeEntry->qualifiedTargetLangName();
}

void AbstractMetaType::setTypeUsagePattern(AbstractMetaType::TypeUsagePattern pattern)
{
    if (d->m_pattern != pattern)
        d->m_pattern = pattern;
}

AbstractMetaType::TypeUsagePattern AbstractMetaType::typeUsagePattern() const
{
    return d->m_pattern;
}

bool AbstractMetaType::hasInstantiations() const
{
    return !d->m_instantiations.isEmpty();
}

void AbstractMetaType::addInstantiation(const AbstractMetaType &inst)
{
    d->m_instantiations << inst;
}

void AbstractMetaType::setInstantiations(const AbstractMetaTypeList &insts)
{
    if (insts != d->m_instantiations)
        d->m_instantiations = insts;
}

const AbstractMetaTypeList &AbstractMetaType::instantiations() const
{
    return d->m_instantiations;
}

// For applying the <array> function argument modification: change into a type
// where "int *" becomes "int[]".
bool AbstractMetaType::applyArrayModification(QString *errorMessage)
{

    if (d->m_pattern == AbstractMetaType::NativePointerAsArrayPattern) {
        *errorMessage = QLatin1String("<array> modification already applied.");
        return false;
    }
    if (!d->m_arrayElementType.isNull())  {
        QTextStream(errorMessage) << "The type \"" << cppSignature()
            << "\" is an array of " << d->m_arrayElementType->name() << '.';
        return false;
    }
    if (d->m_indirections.isEmpty()) {
        QTextStream(errorMessage) << "The type \"" << cppSignature()
            << "\" does not have indirections.";
        return false;
    }
    // Element type to be used for ArrayHandle<>, strip constness.

    auto elementType = new AbstractMetaType(*this);
    auto indir = indirectionsV();
    indir.pop_front();
    elementType->setIndirectionsV(indir);
    elementType->setConstant(false);
    elementType->setVolatile(false);
    elementType->decideUsagePattern();
    d->m_arrayElementType.reset(elementType);
    d->m_pattern = AbstractMetaType::NativePointerAsArrayPattern;
    return true;
}

const TypeEntry *AbstractMetaType::typeEntry() const
{
    return d->m_typeEntry;
}

void AbstractMetaType::setTypeEntry(const TypeEntry *type)
{
    if (d->m_typeEntry != type)
        d->m_typeEntry = type;
}

void AbstractMetaType::setOriginalTypeDescription(const QString &otd)
{
    if (d->m_originalTypeDescription != otd)
        d->m_originalTypeDescription = otd;
}

QString AbstractMetaType::originalTypeDescription() const
{
    return d->m_originalTypeDescription;
}

void AbstractMetaType::setOriginalTemplateType(const AbstractMetaType &type)
{
    if (d->m_originalTemplateType.isNull() || *d->m_originalTemplateType != type)
        d->m_originalTemplateType.reset(new AbstractMetaType(type));
}

const AbstractMetaType *AbstractMetaType::originalTemplateType() const
{
    return d->m_originalTemplateType.data();
}

AbstractMetaType AbstractMetaType::getSmartPointerInnerType() const
{
    Q_ASSERT(isSmartPointer());
    Q_ASSERT(!d->m_instantiations.isEmpty());
    AbstractMetaType innerType = d->m_instantiations.at(0);
    return innerType;
}

QString AbstractMetaType::getSmartPointerInnerTypeName() const
{
    Q_ASSERT(isSmartPointer());
    return getSmartPointerInnerType().name();
}

AbstractMetaTypeList AbstractMetaType::nestedArrayTypes() const
{
    AbstractMetaTypeList result;
    switch (d->m_pattern) {
    case ArrayPattern:
        for (AbstractMetaType t = *this; t.typeUsagePattern() == ArrayPattern; ) {
            const AbstractMetaType *elt = t.arrayElementType();
            result.append(*elt);
            t = *elt;
        }
        break;
    case NativePointerAsArrayPattern:
        result.append(*d->m_arrayElementType.data());
        break;
    default:
        break;
    }
    return result;
}

bool AbstractMetaTypeData::passByConstRef() const
{
    return m_constant && m_referenceType == LValueReference && m_indirections.isEmpty();
}

bool AbstractMetaType::passByConstRef() const
{
    return d->passByConstRef();
}

bool AbstractMetaTypeData::passByValue() const
{
    return m_referenceType == NoReference && m_indirections.isEmpty();
}

bool AbstractMetaType::passByValue() const
{
    return d->passByValue();
}

ReferenceType AbstractMetaType::referenceType() const
{
    return d->m_referenceType;
}

void AbstractMetaType::setReferenceType(ReferenceType ref)
{
    if (d->m_referenceType != ref) {
        d->m_referenceType = ref;
        d->m_signaturesDirty = true;
    }
}

int AbstractMetaTypeData::actualIndirections() const
{
    return m_indirections.size() + (m_referenceType == LValueReference ? 1 : 0);
}

int AbstractMetaType::actualIndirections() const
{
    return d->actualIndirections();
}

AbstractMetaType::Indirections AbstractMetaType::indirectionsV() const
{
    return d->m_indirections;
}

void AbstractMetaType::setIndirectionsV(const AbstractMetaType::Indirections &i)
{
    if (d->m_indirections != i) {
        d->m_indirections = i;
        d->m_signaturesDirty = true;
    }
}

void AbstractMetaType::clearIndirections()
{
    if (!d->m_indirections.isEmpty()) {
        d->m_indirections.clear();
        d->m_signaturesDirty = true;
    }
}

int AbstractMetaType::indirections() const
{
    return d->m_indirections.size();
}

void AbstractMetaType::setIndirections(int indirections)
{
    const Indirections newValue(indirections, Indirection::Pointer);
    if (d->m_indirections != newValue) {
        d->m_indirections = newValue;
        d->m_signaturesDirty = true;
    }
}

void AbstractMetaType::addIndirection(Indirection i)
{
    d->m_indirections.append(i);
}

void AbstractMetaType::setArrayElementCount(int n)
{
    if (d->m_arrayElementCount != n) {
        d->m_arrayElementCount = n;
        d->m_signaturesDirty = true;
    }
}

int AbstractMetaType::arrayElementCount() const
{
    return d->m_arrayElementCount;
}

const AbstractMetaType *AbstractMetaType::arrayElementType() const
{
    return d->m_arrayElementType.data();
}

void AbstractMetaType::setArrayElementType(const AbstractMetaType &t)
{
    if (d->m_arrayElementType.isNull() || *d->m_arrayElementType != t) {
        d->m_arrayElementType.reset(new AbstractMetaType(t));
        d->m_signaturesDirty = true;
    }
}

QString AbstractMetaType::cppSignature() const
{
    const AbstractMetaTypeData *cd = d.constData();
    if (cd->m_cachedCppSignature.isEmpty() || cd->m_signaturesDirty)
        cd->m_cachedCppSignature = formatSignature(false);
    return cd->m_cachedCppSignature;
}

QString AbstractMetaType::pythonSignature() const
{
    // PYSIDE-921: Handle container returntypes correctly.
    // This is now a clean reimplementation.
    const AbstractMetaTypeData *cd = d.constData();
    if (cd->m_cachedPythonSignature.isEmpty() || cd->m_signaturesDirty)
        cd->m_cachedPythonSignature = formatPythonSignature();
    return cd->m_cachedPythonSignature;
}

AbstractMetaType::TypeUsagePattern AbstractMetaTypeData::determineUsagePattern() const
{
    if (m_typeEntry->isTemplateArgument())
        return AbstractMetaType::TemplateArgument;

    if (m_typeEntry->type() == TypeEntry::ConstantValueType)
        return AbstractMetaType::NonTypeTemplateArgument;

    if (m_typeEntry->isPrimitive() && (actualIndirections() == 0 || passByConstRef()))
        return AbstractMetaType::PrimitivePattern;

    if (m_typeEntry->isVoid()) {
        return m_arrayElementCount < 0 && m_referenceType == NoReference
            && m_indirections.isEmpty() && m_constant == 0 && m_volatile == 0
            ? AbstractMetaType::VoidPattern : AbstractMetaType::NativePointerPattern;
    }

    if (m_typeEntry->isVarargs())
        return AbstractMetaType::VarargsPattern;

    if (m_typeEntry->isEnum() && (actualIndirections() == 0 || passByConstRef()))
        return AbstractMetaType::EnumPattern;

    if (m_typeEntry->isObject()) {
        if (m_indirections.isEmpty() && m_referenceType == NoReference)
            return AbstractMetaType::ValuePattern;
        return AbstractMetaType::ObjectPattern;
    }

    if (m_typeEntry->isContainer() && m_indirections.isEmpty())
        return AbstractMetaType::ContainerPattern;

    if (m_typeEntry->isSmartPointer() && m_indirections.isEmpty())
        return AbstractMetaType::SmartPointerPattern;

    if (m_typeEntry->isFlags() && (actualIndirections() == 0 || passByConstRef()))
        return AbstractMetaType::FlagsPattern;

    if (m_typeEntry->isArray())
        return AbstractMetaType::ArrayPattern;

    if (m_typeEntry->isValue())
        return m_indirections.size() == 1 ? AbstractMetaType::ValuePointerPattern : AbstractMetaType::ValuePattern;

    return AbstractMetaType::NativePointerPattern;
}

AbstractMetaType::TypeUsagePattern AbstractMetaType::determineUsagePattern() const
{
    return d->determineUsagePattern();
}

void AbstractMetaType::decideUsagePattern()
{
    TypeUsagePattern pattern = determineUsagePattern();
    if (d->m_typeEntry->isObject() && indirections() == 1
        && d->m_referenceType == LValueReference && isConstant()) {
        // const-references to pointers can be passed as pointers
        setReferenceType(NoReference);
        setConstant(false);
        pattern = ObjectPattern;
    }
    setTypeUsagePattern(pattern);
}

bool AbstractMetaTypeData::hasTemplateChildren() const
{
    QStack<AbstractMetaType> children;
    children << m_instantiations;

    // Recursively iterate over the children / descendants of the type, to check if any of them
    // corresponds to a template argument type.
    while (!children.isEmpty()) {
        AbstractMetaType child = children.pop();
        if (child.typeEntry()->isTemplateArgument())
            return true;
        children << child.instantiations();
    }
    return false;
}

bool AbstractMetaType::hasTemplateChildren() const
{
    return d->hasTemplateChildren();
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

QString AbstractMetaTypeData::formatSignature(bool minimal) const
{
    QString result;
    if (m_constant)
        result += QLatin1String("const ");
    if (m_volatile)
        result += QLatin1String("volatile ");
    if (m_pattern == AbstractMetaType::ArrayPattern) {
        // Build nested array dimensions a[2][3] in correct order
        result += m_arrayElementType->minimalSignature();
        const int arrayPos = result.indexOf(QLatin1Char('['));
        if (arrayPos != -1)
            result.insert(arrayPos, formatArraySize(m_arrayElementCount));
        else
            result.append(formatArraySize(m_arrayElementCount));
    } else {
        result += m_typeEntry->qualifiedCppName();
    }
    if (!m_instantiations.isEmpty()) {
        result += QLatin1Char('<');
        if (minimal)
            result += QLatin1Char(' ');
        for (int i = 0, size = m_instantiations.size(); i < size; ++i) {
            if (i > 0)
                result += QLatin1Char(',');
            result += m_instantiations.at(i).minimalSignature();
        }
        result += QLatin1String(" >");
    }

    if (!minimal && (!m_indirections.isEmpty() || m_referenceType != NoReference))
        result += QLatin1Char(' ');
    for (Indirection i : m_indirections)
        result += TypeInfo::indirectionKeyword(i);
    switch (m_referenceType) {
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

QString AbstractMetaType::formatSignature(bool minimal) const
{
    return d->formatSignature(minimal);
}

QString AbstractMetaTypeData::formatPythonSignature() const
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
     * Smart pointer instantiations: Drop the package
     */
    QString result;
    if (m_pattern == AbstractMetaType::NativePointerAsArrayPattern)
        result += QLatin1String("array ");
    // We no longer use the "const" qualifier for heuristics. Instead,
    // NativePointerAsArrayPattern indicates when we have <array> in XML.
    // if (m_typeEntry->isPrimitive() && isConstant())
    //     result += QLatin1String("const ");
    if (!m_typeEntry->isPrimitive() && !m_typeEntry->isSmartPointer()) {
        const QString package = m_typeEntry->targetLangPackage();
        if (!package.isEmpty())
            result += package + QLatin1Char('.');
    }
    if (m_pattern == AbstractMetaType::ArrayPattern) {
        // Build nested array dimensions a[2][3] in correct order
        result += m_arrayElementType->formatPythonSignature();
        const int arrayPos = result.indexOf(QLatin1Char('['));
        if (arrayPos != -1)
            result.insert(arrayPos, formatArraySize(m_arrayElementCount));
        else
            result.append(formatArraySize(m_arrayElementCount));
    } else {
        result += m_typeEntry->targetLangName();
    }
    if (!m_instantiations.isEmpty()) {
        result += QLatin1Char('[');
        for (int i = 0, size = m_instantiations.size(); i < size; ++i) {
            if (i > 0)
                result += QLatin1String(", ");
            result += m_instantiations.at(i).formatPythonSignature();
        }
        result += QLatin1Char(']');
    }
    if (m_typeEntry->isPrimitive())
        for (Indirection i : m_indirections)
            result += TypeInfo::indirectionKeyword(i);
    // If it is a flags type, we replace it with the full name:
    // "PySide2.QtCore.Qt.ItemFlags" instead of "PySide2.QtCore.QFlags<Qt.ItemFlag>"
    if (m_typeEntry->isFlags())
        result = m_typeEntry->qualifiedTargetLangName();
    result.replace(QLatin1String("::"), QLatin1String("."));
    return result;
}

QString AbstractMetaType::formatPythonSignature() const
{
    return d->formatPythonSignature();
}

bool AbstractMetaType::isCppPrimitive() const
{
    return d->m_pattern == PrimitivePattern && d->m_typeEntry->isCppPrimitive();
}

bool AbstractMetaType::isConstant() const
{
    return d->m_constant;
}

void AbstractMetaType::setConstant(bool constant)
{
    if (d->m_constant != constant) {
        d->m_constant = constant;
        d->m_signaturesDirty = true;
    }
}

bool AbstractMetaType::isVolatile() const
{
    return d->m_volatile;
}

void AbstractMetaType::setVolatile(bool v)
{
    if (d->m_volatile != v) {
        d->m_volatile = v;
        d->m_signaturesDirty = true;\
    }
}

static bool equalsCPtr(const AbstractMetaTypeCPtr &t1, const AbstractMetaTypeCPtr &t2)
{
    if (t1.isNull() != t2.isNull())
        return false;
    return t1.isNull() || *t1 == *t2;
}

bool AbstractMetaTypeData::equals(const AbstractMetaTypeData &rhs) const
{
    if (m_typeEntry != rhs.m_typeEntry
        || m_indirections != rhs.m_indirections
        || m_arrayElementCount != rhs.m_arrayElementCount) {
        return false;
    }

    if (m_constant != rhs.m_constant || m_volatile != rhs.m_volatile
        || m_referenceType != rhs.m_referenceType) {
        return false;
    }

    if (!equalsCPtr(m_arrayElementType, rhs.m_arrayElementType))
        return false;

    if (!equalsCPtr(m_viewOn, rhs.m_viewOn))
        return false;

    if (m_instantiations != rhs.m_instantiations)
        return false;
    return true;
}

bool AbstractMetaType::equals(const AbstractMetaType &rhs) const
{
    return d->equals(*rhs.d);
}

const AbstractMetaType *AbstractMetaType::viewOn() const
{
    return d->m_viewOn.data();
}

void AbstractMetaType::setViewOn(const AbstractMetaType &v)
{
    if (d->m_viewOn.isNull() || *d->m_viewOn != v)
        d->m_viewOn.reset(new AbstractMetaType(v));
}

AbstractMetaType AbstractMetaType::createVoid()
{
    static QScopedPointer<AbstractMetaType> metaType;
    if (metaType.isNull()) {
        static const TypeEntry *voidTypeEntry = TypeDatabase::instance()->findType(QLatin1String("void"));
        Q_ASSERT(voidTypeEntry);
        metaType.reset(new AbstractMetaType(voidTypeEntry));
        metaType->decideUsagePattern();
    }
    return *metaType.data();
}

#ifndef QT_NO_DEBUG_STREAM
void AbstractMetaType::formatDebug(QDebug &debug) const
{
    if (!isValid()) {
        debug << "Invalid";
        return;
    }
    debug << '"' << name() << '"';
    if (debug.verbosity() > 2) {
        auto te = typeEntry();
        debug << ", typeEntry=";
        if (debug.verbosity() > 3)
            debug << te;
        else
            debug << "(\"" << te->qualifiedCppName() << "\", " << te->type() << ')';
        debug << ", signature=\"" << cppSignature() << "\", pattern="
            << typeUsagePattern();
        const auto indirections = indirectionsV();
        if (!indirections.isEmpty()) {
            debug << ", indirections=";
            for (auto i : indirections)
                debug << ' ' << TypeInfo::indirectionKeyword(i);
        }
        if (referenceType())
            debug << ", reftype=" << referenceType();
        if (isConstant())
            debug << ", [const]";
        if (isVolatile())
            debug << ", [volatile]";
        if (isArray()) {
            debug << ", array of \"" << arrayElementType()->cppSignature()
                << "\", arrayElementCount="  << arrayElementCount();
        }
        const auto &instantiations = this->instantiations();
        if (const int instantiationsSize = instantiations.size()) {
            debug << ", instantiations[" << instantiationsSize << "]=<";
            for (int i = 0; i < instantiationsSize; ++i) {
                if (i)
                    debug << ", ";
                instantiations.at(i).formatDebug(debug);
            }
        }
        debug << '>';
        if (viewOn())
            debug << ", views " << viewOn()->name();
    }
}

QDebug operator<<(QDebug d, const AbstractMetaType &at)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaType(";
    at.formatDebug(d);
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const AbstractMetaType *at)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    if (at)
        d << *at;
    else
        d << "AbstractMetaType(0)";
    return d;
}

#endif // !QT_NO_DEBUG_STREAM
