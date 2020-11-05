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

#include "typesystem.h"
#include "typedatabase.h"
#include "messages.h"
#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>

static QString strings_Object = QLatin1String("Object");
static QString strings_String = QLatin1String("String");
static QString strings_char = QLatin1String("char");
static QString strings_jchar = QLatin1String("jchar");
static QString strings_jobject = QLatin1String("jobject");

static inline QString callOperator() { return QStringLiteral("operator()"); }

PrimitiveTypeEntry::PrimitiveTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                       const TypeEntry *parent) :
    TypeEntry(entryName, PrimitiveType, vr, parent),
    m_preferredTargetLangType(true)
{
}

QString PrimitiveTypeEntry::targetLangApiName() const
{
    return m_targetLangApiName;
}

PrimitiveTypeEntry *PrimitiveTypeEntry::basicReferencedTypeEntry() const
{
    if (!m_referencedTypeEntry)
        return nullptr;

    PrimitiveTypeEntry *baseReferencedTypeEntry = m_referencedTypeEntry->basicReferencedTypeEntry();
    return baseReferencedTypeEntry ? baseReferencedTypeEntry : m_referencedTypeEntry;
}

TypeEntry *PrimitiveTypeEntry::clone() const
{
    return new PrimitiveTypeEntry(*this);
}

PrimitiveTypeEntry::PrimitiveTypeEntry(const PrimitiveTypeEntry &) = default;

CodeSnipList TypeEntry::codeSnips() const
{
    return m_codeSnips;
}

void TypeEntry::addExtraInclude(const Include &newInclude)
{
    if (!m_extraIncludes.contains(newInclude))
        m_extraIncludes.append(newInclude);
}

FunctionModificationList ComplexTypeEntry::functionModifications(const QString &signature) const
{
    FunctionModificationList lst;
    for (int i = 0; i < m_functionMods.count(); ++i) {
        const FunctionModification &mod = m_functionMods.at(i);
        if (mod.matches(signature))
            lst << mod;
    }
    return lst;
}

FieldModification ComplexTypeEntry::fieldModification(const QString &name) const
{
    for (const auto &fieldMod : m_fieldMods) {
        if (fieldMod.name == name)
            return fieldMod;
    }
    FieldModification mod;
    mod.name = name;
    mod.modifiers = FieldModification::Readable | FieldModification::Writable;
    return mod;
}

void ComplexTypeEntry::setDefaultConstructor(const QString& defaultConstructor)
{
    m_defaultConstructor = defaultConstructor;
}
QString ComplexTypeEntry::defaultConstructor() const
{
    return m_defaultConstructor;
}
bool ComplexTypeEntry::hasDefaultConstructor() const
{
    return !m_defaultConstructor.isEmpty();
}

TypeEntry *ComplexTypeEntry::clone() const
{
    return new ComplexTypeEntry(*this);
}

// Take over parameters relevant for typedefs
void ComplexTypeEntry::useAsTypedef(const ComplexTypeEntry *source)
{
    TypeEntry::useAsTypedef(source);
    m_qualifiedCppName = source->m_qualifiedCppName;
    m_targetType = source->m_targetType;
}

ComplexTypeEntry::ComplexTypeEntry(const ComplexTypeEntry &) = default;

QString ContainerTypeEntry::qualifiedCppName() const
{
    if (m_containerKind == StringListContainer)
        return QLatin1String("QStringList");
    return ComplexTypeEntry::qualifiedCppName();
}

TypeEntry *ContainerTypeEntry::clone() const
{
    return new ContainerTypeEntry(*this);
}

ContainerTypeEntry::ContainerTypeEntry(const ContainerTypeEntry &) = default;

QString EnumTypeEntry::targetLangQualifier() const
{
    const QString q = qualifier();
    if (!q.isEmpty()) {
        if (auto te = TypeDatabase::instance()->findType(q))
            return te->targetLangName();
    }
    return q;
}

QString EnumTypeEntry::qualifier() const
{
    auto parentEntry = parent();
    return parentEntry && parentEntry->type() != TypeEntry::TypeSystemType ?
        parentEntry->name() : QString();
}

QString EnumTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

QString FlagsTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

TypeEntry *EnumTypeEntry::clone() const
{
    return new EnumTypeEntry(*this);
}

EnumTypeEntry::EnumTypeEntry(const EnumTypeEntry &) = default;

TypeEntry *FlagsTypeEntry::clone() const
{
    return new FlagsTypeEntry(*this);
}

FlagsTypeEntry::FlagsTypeEntry(const FlagsTypeEntry &) = default;

static QString buildName(const QString &entryName, const TypeEntry *parent)
{
    return parent == nullptr || parent->type() == TypeEntry::TypeSystemType
        ? entryName : parent->name() + QLatin1String("::") + entryName;
}

ComplexTypeEntry::ComplexTypeEntry(const QString &entryName, TypeEntry::Type t,
                                   const QVersionNumber &vr,
                                   const TypeEntry *parent) :
    TypeEntry(entryName, t, vr, parent),
    m_qualifiedCppName(buildName(entryName, parent)),
    m_polymorphicBase(false),
    m_genericClass(false),
    m_deleteInMainThread(false)
{
}

bool ComplexTypeEntry::isComplex() const
{
    return true;
}

QString ComplexTypeEntry::targetLangApiName() const
{
    return strings_jobject;
}

QString ContainerTypeEntry::typeName() const
{
    switch (m_containerKind) {
        case LinkedListContainer:
            return QLatin1String("linked-list");
        case ListContainer:
            return QLatin1String("list");
        case StringListContainer:
            return QLatin1String("string-list");
        case VectorContainer:
            return QLatin1String("vector");
        case StackContainer:
            return QLatin1String("stack");
        case QueueContainer:
            return QLatin1String("queue");
        case SetContainer:
            return QLatin1String("set");
        case MapContainer:
            return QLatin1String("map");
        case MultiMapContainer:
            return QLatin1String("multi-map");
        case HashContainer:
            return QLatin1String("hash");
        case MultiHashContainer:
            return QLatin1String("multi-hash");
        case PairContainer:
            return QLatin1String("pair");
        case NoContainer:
        default:
            return QLatin1String("?");
    }
}

static const QSet<QString> &primitiveCppTypes()
{
    static QSet<QString> result;
    if (result.isEmpty()) {
        static const char *cppTypes[] = {
            "bool", "char", "double", "float", "int",
            "long", "long long", "short",
            "wchar_t"
        };
        for (const char *cppType : cppTypes)
            result.insert(QLatin1String(cppType));
    }
    return result;
}

void TypeEntry::setInclude(const Include &inc)
{
    // This is a workaround for preventing double inclusion of the QSharedPointer implementation
    // header, which does not use header guards. In the previous parser this was not a problem
    // because the Q_QDOC define was set, and the implementation header was never included.
    if (inc.name().endsWith(QLatin1String("qsharedpointer_impl.h"))) {
        QString path = inc.name();
        path.remove(QLatin1String("_impl"));
        m_include = Include(inc.type(), path);
    } else {
        m_include = inc;
    }
}

bool TypeEntry::isCppPrimitive() const
{
    if (!isPrimitive())
        return false;

    if (m_type == VoidType)
        return true;

    const PrimitiveTypeEntry *referencedType =
        static_cast<const PrimitiveTypeEntry *>(this)->basicReferencedTypeEntry();
    const QString &typeName = referencedType ? referencedType->name() : m_name;
    return typeName.contains(QLatin1Char(' ')) || primitiveCppTypes().contains(typeName);
}

TypeEntry::TypeEntry(const QString &entryName, TypeEntry::Type t, const QVersionNumber &vr,
                     const TypeEntry *parent) :
    m_parent(parent),
    m_name(buildName(entryName, parent)),
    m_entryName(entryName),
    m_version(vr),
    m_type(t)
{
}

TypeEntry::~TypeEntry()
{
    delete m_customConversion;
}

bool TypeEntry::isChildOf(const TypeEntry *p) const
{
    for (auto e = m_parent; e; e = e->parent()) {
        if (e == p)
            return true;
    }
    return false;
}

const TypeSystemTypeEntry *TypeEntry::typeSystemTypeEntry() const
{
    for (auto e = this; e; e = e->parent()) {
        if (e->type() == TypeEntry::TypeSystemType)
            return static_cast<const TypeSystemTypeEntry *>(e);
    }
    return nullptr;
}

const TypeEntry *TypeEntry::targetLangEnclosingEntry() const
{
    auto result = m_parent;
    while (result && result->type() != TypeEntry::TypeSystemType
           && !NamespaceTypeEntry::isVisibleScope(result)) {
        result = result->parent();
    }
    return result;
}

QString TypeEntry::targetLangName() const
{
    if (m_cachedTargetLangName.isEmpty())
        m_cachedTargetLangName = buildTargetLangName();
    return m_cachedTargetLangName;
}

QString TypeEntry::buildTargetLangName() const
{
    QString result = m_entryName;
    for (auto p = parent(); p && p->type() != TypeEntry::TypeSystemType; p = p->parent()) {
        if (NamespaceTypeEntry::isVisibleScope(p)) {
            if (!result.isEmpty())
                result.prepend(QLatin1Char('.'));
            QString n = p->m_entryName;
            n.replace(QLatin1String("::"), QLatin1String(".")); // Primitive types may have "std::"
            result.prepend(n);
        }
    }
    return result;
}

SourceLocation TypeEntry::sourceLocation() const
{
    return m_sourceLocation;
}

void TypeEntry::setSourceLocation(const SourceLocation &sourceLocation)
{
    m_sourceLocation = sourceLocation;
}

QString TypeEntry::targetLangEntryName() const
{
    if (m_cachedTargetLangEntryName.isEmpty()) {
        m_cachedTargetLangEntryName = targetLangName();
        const int lastDot = m_cachedTargetLangEntryName.lastIndexOf(QLatin1Char('.'));
        if (lastDot != -1)
            m_cachedTargetLangEntryName.remove(0, lastDot + 1);
    }
    return m_cachedTargetLangEntryName;
}

QString TypeEntry::qualifiedTargetLangName() const
{
    return targetLangPackage() + QLatin1Char('.') + targetLangName();
}

bool TypeEntry::hasCustomConversion() const
{
    return m_customConversion != nullptr;
}

void TypeEntry::setCustomConversion(CustomConversion* customConversion)
{
    m_customConversion = customConversion;
}

CustomConversion* TypeEntry::customConversion() const
{
    return m_customConversion;
}

TypeEntry *TypeEntry::clone() const
{
    return new TypeEntry(*this);
}

// Take over parameters relevant for typedefs
void TypeEntry::useAsTypedef(const TypeEntry *source)
{
    // XML Typedefs are in the global namespace for now.
    m_parent = source->typeSystemTypeEntry();
    m_entryName = source->m_entryName;
    m_name = source->m_name;
    m_targetLangPackage = source->m_targetLangPackage;
    m_codeGeneration = source->m_codeGeneration;
    m_version = source->m_version;
}

TypeEntry::TypeEntry(const TypeEntry &) = default;

TypeSystemTypeEntry::TypeSystemTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                         const TypeEntry *parent) :
    TypeEntry(entryName, TypeSystemType, vr, parent)
{
}

TypeEntry *TypeSystemTypeEntry::clone() const
{
    return new TypeSystemTypeEntry(*this);
}

TypeSystemTypeEntry::TypeSystemTypeEntry(const TypeSystemTypeEntry &) = default;

VoidTypeEntry::VoidTypeEntry() :
    TypeEntry(QLatin1String("void"), VoidType, QVersionNumber(0, 0), nullptr)
{
}

TypeEntry *VoidTypeEntry::clone() const
{
    return new VoidTypeEntry(*this);
}

VoidTypeEntry::VoidTypeEntry(const VoidTypeEntry &) = default;

VarargsTypeEntry::VarargsTypeEntry() :
    TypeEntry(QLatin1String("..."), VarargsType, QVersionNumber(0, 0), nullptr)
{
}

TypeEntry *VarargsTypeEntry::clone() const
{
    return new VarargsTypeEntry(*this);
}

VarargsTypeEntry::VarargsTypeEntry(const VarargsTypeEntry &) = default;

TemplateArgumentEntry::TemplateArgumentEntry(const QString &entryName, const QVersionNumber &vr,
                                             const TypeEntry *parent) :
    TypeEntry(entryName, TemplateArgumentType, vr, parent)
{
}

TypeEntry *TemplateArgumentEntry::clone() const
{
    return new TemplateArgumentEntry(*this);
}

TemplateArgumentEntry::TemplateArgumentEntry(const TemplateArgumentEntry &) = default;

ArrayTypeEntry::ArrayTypeEntry(const TypeEntry *nested_type, const QVersionNumber &vr,
                               const TypeEntry *parent) :
    TypeEntry(QLatin1String("Array"), ArrayType, vr, parent),
    m_nestedType(nested_type)
{
    Q_ASSERT(m_nestedType);
}

QString ArrayTypeEntry::buildTargetLangName() const
{
    return m_nestedType->targetLangName() + QLatin1String("[]");
}

QString ArrayTypeEntry::targetLangApiName() const
{
    return m_nestedType->isPrimitive()
        ? m_nestedType->targetLangApiName() + QLatin1String("Array")
        : QLatin1String("jobjectArray");
}

TypeEntry *ArrayTypeEntry::clone() const
{
    return new ArrayTypeEntry(*this);
}

ArrayTypeEntry::ArrayTypeEntry(const ArrayTypeEntry &) = default;

EnumTypeEntry::EnumTypeEntry(const QString &entryName,
                             const QVersionNumber &vr,
                             const TypeEntry *parent) :
    TypeEntry(entryName, EnumType, vr, parent)
{
}

EnumValueTypeEntry::EnumValueTypeEntry(const QString &name, const QString &value,
                                       const EnumTypeEntry *enclosingEnum,
                                       bool isScopedEnum,
                                       const QVersionNumber &vr) :
    TypeEntry(name, TypeEntry::EnumValue, vr,
              isScopedEnum ? enclosingEnum : enclosingEnum->parent()),
    m_value(value),
    m_enclosingEnum(enclosingEnum)
{
}

TypeEntry *EnumValueTypeEntry::clone() const
{
    return new EnumValueTypeEntry(*this);
}

EnumValueTypeEntry::EnumValueTypeEntry(const EnumValueTypeEntry &) = default;

FlagsTypeEntry::FlagsTypeEntry(const QString &entryName, const QVersionNumber &vr,
                               const TypeEntry *parent) :
    TypeEntry(entryName, FlagsType, vr, parent)
{
}

QString FlagsTypeEntry::buildTargetLangName() const
{
    QString on = m_originalName;
    on.replace(QLatin1String("::"), QLatin1String("."));
    return on;
}

ConstantValueTypeEntry::ConstantValueTypeEntry(const QString& name,
                                               const TypeEntry *parent) :
    TypeEntry(name, ConstantValueType, QVersionNumber(0, 0), parent)
{
}

TypeEntry *ConstantValueTypeEntry::clone() const
{
    return new ConstantValueTypeEntry(*this);
}

ConstantValueTypeEntry::ConstantValueTypeEntry(const ConstantValueTypeEntry &) = default;

/* A typedef entry allows for specifying template specializations in the
 * typesystem XML file. */
TypedefEntry::TypedefEntry(const QString &entryName, const QString &sourceType,
                           const QVersionNumber &vr, const TypeEntry *parent) :
    ComplexTypeEntry(entryName, TypedefType, vr, parent),
    m_sourceType(sourceType)
{
}

TypeEntry *TypedefEntry::clone() const
{
    return new TypedefEntry(*this);
}

TypedefEntry::TypedefEntry(const TypedefEntry &) = default;

ContainerTypeEntry::ContainerTypeEntry(const QString &entryName, ContainerKind containerKind,
                                       const QVersionNumber &vr,
                                       const TypeEntry *parent) :
    ComplexTypeEntry(entryName, ContainerType, vr, parent),
    m_containerKind(containerKind)
{
    setCodeGeneration(GenerateForSubclass);
}

SmartPointerTypeEntry::SmartPointerTypeEntry(const QString &entryName,
                                             const QString &getterName,
                                             const QString &smartPointerType,
                                             const QString &refCountMethodName,
                                             const QVersionNumber &vr, const TypeEntry *parent) :
    ComplexTypeEntry(entryName, SmartPointerType, vr, parent),
    m_getterName(getterName),
    m_smartPointerType(smartPointerType),
    m_refCountMethodName(refCountMethodName)
{
}

TypeEntry *SmartPointerTypeEntry::clone() const
{
    return new SmartPointerTypeEntry(*this);
}

SmartPointerTypeEntry::SmartPointerTypeEntry(const SmartPointerTypeEntry &) = default;

bool SmartPointerTypeEntry::matchesInstantiation(const TypeEntry *e) const
{
    return m_instantiations.isEmpty() || m_instantiations.contains(e);
}

NamespaceTypeEntry::NamespaceTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                       const TypeEntry *parent) :
    ComplexTypeEntry(entryName, NamespaceType, vr, parent)
{
}

TypeEntry *NamespaceTypeEntry::clone() const
{
    return new NamespaceTypeEntry(*this);
}

void NamespaceTypeEntry::setFilePattern(const QRegularExpression &r)
{
    m_filePattern = r;
    m_hasPattern = !m_filePattern.pattern().isEmpty();
    if (m_hasPattern)
        m_filePattern.optimize();
}

NamespaceTypeEntry::NamespaceTypeEntry(const NamespaceTypeEntry &) = default;

bool NamespaceTypeEntry::matchesFile(const QString &needle) const
{
    return m_filePattern.match(needle).hasMatch();
}

bool NamespaceTypeEntry::isVisible() const
{
    return m_visibility == TypeSystem::Visibility::Visible
        || (m_visibility == TypeSystem::Visibility::Auto && !m_inlineNamespace);
}

bool NamespaceTypeEntry::isVisibleScope(const TypeEntry *e)
{
    return e->type() != TypeEntry::NamespaceType
        || static_cast<const NamespaceTypeEntry *>(e)->isVisible();
}

ValueTypeEntry::ValueTypeEntry(const QString &entryName, const QVersionNumber &vr,
                               const TypeEntry *parent) :
    ComplexTypeEntry(entryName, BasicValueType, vr, parent)
{
}

bool ValueTypeEntry::isValue() const
{
    return true;
}

TypeEntry *ValueTypeEntry::clone() const
{
    return new ValueTypeEntry(*this);
}

ValueTypeEntry::ValueTypeEntry(const ValueTypeEntry &) = default;

ValueTypeEntry::ValueTypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                               const TypeEntry *parent) :
    ComplexTypeEntry(entryName, t, vr, parent)
{
}

struct CustomConversion::CustomConversionPrivate
{
    CustomConversionPrivate(const TypeEntry* ownerType)
        : ownerType(ownerType), replaceOriginalTargetToNativeConversions(false)
    {
    }
    const TypeEntry* ownerType;
    QString nativeToTargetConversion;
    bool replaceOriginalTargetToNativeConversions;
    TargetToNativeConversions targetToNativeConversions;
};

struct CustomConversion::TargetToNativeConversion::TargetToNativeConversionPrivate
{
    TargetToNativeConversionPrivate()
        : sourceType(nullptr)
    {
    }
    const TypeEntry* sourceType;
    QString sourceTypeName;
    QString sourceTypeCheck;
    QString conversion;
};

CustomConversion::CustomConversion(TypeEntry* ownerType)
{
    m_d = new CustomConversionPrivate(ownerType);
    if (ownerType)
        ownerType->setCustomConversion(this);
}

CustomConversion::~CustomConversion()
{
    qDeleteAll(m_d->targetToNativeConversions);
    delete m_d;
}

const TypeEntry* CustomConversion::ownerType() const
{
    return m_d->ownerType;
}

QString CustomConversion::nativeToTargetConversion() const
{
    return m_d->nativeToTargetConversion;
}

void CustomConversion::setNativeToTargetConversion(const QString& nativeToTargetConversion)
{
    m_d->nativeToTargetConversion = nativeToTargetConversion;
}

bool CustomConversion::replaceOriginalTargetToNativeConversions() const
{
    return m_d->replaceOriginalTargetToNativeConversions;
}

void CustomConversion::setReplaceOriginalTargetToNativeConversions(bool replaceOriginalTargetToNativeConversions)
{
    m_d->replaceOriginalTargetToNativeConversions = replaceOriginalTargetToNativeConversions;
}

bool CustomConversion::hasTargetToNativeConversions() const
{
    return !(m_d->targetToNativeConversions.isEmpty());
}

CustomConversion::TargetToNativeConversions& CustomConversion::targetToNativeConversions()
{
    return m_d->targetToNativeConversions;
}

const CustomConversion::TargetToNativeConversions& CustomConversion::targetToNativeConversions() const
{
    return m_d->targetToNativeConversions;
}

void CustomConversion::addTargetToNativeConversion(const QString& sourceTypeName,
                                                   const QString& sourceTypeCheck,
                                                   const QString& conversion)
{
    m_d->targetToNativeConversions.append(new TargetToNativeConversion(sourceTypeName, sourceTypeCheck, conversion));
}

CustomConversion::TargetToNativeConversion::TargetToNativeConversion(const QString& sourceTypeName,
                                                                     const QString& sourceTypeCheck,
                                                                     const QString& conversion)
{
    m_d = new TargetToNativeConversionPrivate;
    m_d->sourceTypeName = sourceTypeName;
    m_d->sourceTypeCheck = sourceTypeCheck;
    m_d->conversion = conversion;
}

CustomConversion::TargetToNativeConversion::~TargetToNativeConversion()
{
    delete m_d;
}

const TypeEntry* CustomConversion::TargetToNativeConversion::sourceType() const
{
    return m_d->sourceType;
}

void CustomConversion::TargetToNativeConversion::setSourceType(const TypeEntry* sourceType)
{
    m_d->sourceType = sourceType;
}

bool CustomConversion::TargetToNativeConversion::isCustomType() const
{
    return !(m_d->sourceType);
}

QString CustomConversion::TargetToNativeConversion::sourceTypeName() const
{
    return m_d->sourceTypeName;
}

QString CustomConversion::TargetToNativeConversion::sourceTypeCheck() const
{
    return m_d->sourceTypeCheck;
}

QString CustomConversion::TargetToNativeConversion::conversion() const
{
    return m_d->conversion;
}

void CustomConversion::TargetToNativeConversion::setConversion(const QString& conversion)
{
    m_d->conversion = conversion;
}

FunctionTypeEntry::FunctionTypeEntry(const QString &entryName, const QString &signature,
                                     const QVersionNumber &vr,
                                     const TypeEntry *parent) :
    TypeEntry(entryName, FunctionType, vr, parent)
{
    addSignature(signature);
}

TypeEntry *FunctionTypeEntry::clone() const
{
    return new FunctionTypeEntry(*this);
}

FunctionTypeEntry::FunctionTypeEntry(const FunctionTypeEntry &) = default;

ObjectTypeEntry::ObjectTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                 const TypeEntry *parent)
    : ComplexTypeEntry(entryName, ObjectType, vr, parent)
{
}

TypeEntry *ObjectTypeEntry::clone() const
{
    return new ObjectTypeEntry(*this);
}

ObjectTypeEntry::ObjectTypeEntry(const ObjectTypeEntry &) = default;
