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
#include "modifications.h"
#include "messages.h"
#include "sourcelocation.h"

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>

static QString strings_Object = QLatin1String("Object");
static QString strings_String = QLatin1String("String");
static QString strings_char = QLatin1String("char");
static QString strings_jchar = QLatin1String("jchar");
static QString strings_jobject = QLatin1String("jobject");

static QString buildName(const QString &entryName, const TypeEntry *parent)
{
    return parent == nullptr || parent->type() == TypeEntry::TypeSystemType
        ? entryName : parent->name() + QLatin1String("::") + entryName;
}

// Access private class as 'd', cf macro Q_D()
#define S_D(Class) auto d = static_cast<Class##Private *>(d_func())

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

class TypeEntryPrivate
{
public:
    explicit TypeEntryPrivate(const QString &entryName, TypeEntry::Type t, const QVersionNumber &vr,
                              const TypeEntry *parent);
    virtual ~TypeEntryPrivate();

    QString shortName() const;

    const TypeEntry *m_parent;
    QString m_name; // C++ fully qualified
    mutable QString m_cachedShortName; // C++ excluding inline namespaces
    QString m_entryName;
    QString m_targetLangPackage;
    mutable QString m_cachedTargetLangName; // "Foo.Bar"
    mutable QString m_cachedTargetLangEntryName; // "Bar"
    CustomFunction m_customConstructor;
    CustomFunction m_customDestructor;
    CodeSnipList m_codeSnips;
    DocModificationList m_docModifications;
    IncludeList m_extraIncludes;
    Include m_include;
    QString m_conversionRule;
    QVersionNumber m_version;
    CustomConversion *m_customConversion = nullptr;
    SourceLocation m_sourceLocation; // XML file
    TypeEntry::CodeGeneration m_codeGeneration = TypeEntry::GenerateCode;
    TypeEntry *m_viewOn = nullptr;
    int m_revision = 0;
    int m_sbkIndex = 0;
    TypeEntry::Type m_type;
    bool m_stream = false;
};

TypeEntryPrivate::TypeEntryPrivate(const QString &entryName, TypeEntry::Type t, const QVersionNumber &vr,
                                   const TypeEntry *parent) :
    m_parent(parent),
    m_name(buildName(entryName, parent)),
    m_entryName(entryName),
    m_version(vr),
    m_type(t)
{
}

TypeEntryPrivate::~TypeEntryPrivate()
{
    delete m_customConversion;
}

TypeEntry::TypeEntry(const QString &entryName, TypeEntry::Type t, const QVersionNumber &vr,
                     const TypeEntry *parent) :
    TypeEntry(new TypeEntryPrivate(entryName, t, vr, parent))
{
}

TypeEntry::TypeEntry(TypeEntryPrivate *d) : m_d(d)
{
}

TypeEntry::~TypeEntry() = default;

CodeSnipList TypeEntry::codeSnips() const
{
    return m_d->m_codeSnips;
}

void TypeEntry::setCodeSnips(const CodeSnipList &codeSnips)
{
    m_d->m_codeSnips = codeSnips;
}

void TypeEntry::addCodeSnip(const CodeSnip &codeSnip)
{
    m_d->m_codeSnips << codeSnip;
}

void TypeEntry::setDocModification(const DocModificationList &docMods)
{
    m_d->m_docModifications << docMods;
}

DocModificationList TypeEntry::docModifications() const
{
    return m_d->m_docModifications;
}

const IncludeList &TypeEntry::extraIncludes() const
{
    return m_d->m_extraIncludes;
}

void TypeEntry::setExtraIncludes(const IncludeList &includes)
{
    m_d->m_extraIncludes = includes;
}

void TypeEntry::addExtraInclude(const Include &newInclude)
{
    if (!m_d->m_extraIncludes.contains(newInclude))
        m_d->m_extraIncludes.append(newInclude);
}

Include TypeEntry::include() const
{
    return m_d->m_include;
}

void TypeEntry::setInclude(const Include &inc)
{
    // This is a workaround for preventing double inclusion of the QSharedPointer implementation
    // header, which does not use header guards. In the previous parser this was not a problem
    // because the Q_QDOC define was set, and the implementation header was never included.
    if (inc.name().endsWith(QLatin1String("qsharedpointer_impl.h"))) {
        QString path = inc.name();
        path.remove(QLatin1String("_impl"));
        m_d->m_include = Include(inc.type(), path);
    } else {
        m_d->m_include = inc;
    }
}

void TypeEntry::setConversionRule(const QString &conversionRule)
{
    m_d->m_conversionRule = conversionRule;
}

QString TypeEntry::conversionRule() const
{
    //skip conversions flag
    return m_d->m_conversionRule.mid(1);
}

bool TypeEntry::hasConversionRule() const
{
    return !m_d->m_conversionRule.isEmpty();
}

QVersionNumber TypeEntry::version() const
{
    return m_d->m_version;
}

bool TypeEntry::hasNativeConversionRule() const
{
    return m_d->m_conversionRule.startsWith(QLatin1String(NATIVE_CONVERSION_RULE_FLAG));
}

bool TypeEntry::hasTargetConversionRule() const
{
    return m_d->m_conversionRule.startsWith(QLatin1String(TARGET_CONVERSION_RULE_FLAG));
}

bool TypeEntry::isCppPrimitive() const
{
    if (!isPrimitive())
        return false;

    if (m_d->m_type == VoidType)
        return true;

    const PrimitiveTypeEntry *referencedType =
        static_cast<const PrimitiveTypeEntry *>(this)->basicReferencedTypeEntry();
    const QString &typeName = referencedType ? referencedType->name() : m_d->m_name;
    return typeName.contains(QLatin1Char(' ')) || primitiveCppTypes().contains(typeName);
}

TypeEntry::Type TypeEntry::type() const
{
    return m_d->m_type;
}

const TypeEntry *TypeEntry::parent() const
{
    return m_d->m_parent;
}

void TypeEntry::setParent(const TypeEntry *p)
{
    m_d->m_parent = p;
}

bool TypeEntry::isChildOf(const TypeEntry *p) const
{
    for (auto e = m_d->m_parent; e; e = e->parent()) {
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
    auto result = m_d->m_parent;
    while (result && result->type() != TypeEntry::TypeSystemType
           && !NamespaceTypeEntry::isVisibleScope(result)) {
        result = result->parent();
    }
    return result;
}

bool TypeEntry::isPrimitive() const
{
    return m_d->m_type == PrimitiveType;
}

bool TypeEntry::isEnum() const
{
    return m_d->m_type == EnumType;
}

bool TypeEntry::isFlags() const
{
    return m_d->m_type == FlagsType;
}

bool TypeEntry::isObject() const
{
    return m_d->m_type == ObjectType;
}

bool TypeEntry::isNamespace() const
{
    return m_d->m_type == NamespaceType;
}

bool TypeEntry::isContainer() const
{
    return m_d->m_type == ContainerType;
}

bool TypeEntry::isSmartPointer() const
{
    return m_d->m_type == SmartPointerType;
}

bool TypeEntry::isArray() const
{
    return m_d->m_type == ArrayType;
}

bool TypeEntry::isTemplateArgument() const
{
    return m_d->m_type == TemplateArgumentType;
}

bool TypeEntry::isVoid() const
{
    return m_d->m_type == VoidType;
}

bool TypeEntry::isVarargs() const
{
    return m_d->m_type == VarargsType;
}

bool TypeEntry::isCustom() const
{
    return m_d->m_type == CustomType;
}

bool TypeEntry::isTypeSystem() const
{
    return m_d->m_type == TypeSystemType;
}

bool TypeEntry::isFunction() const
{
    return m_d->m_type == FunctionType;
}

bool TypeEntry::isEnumValue() const
{
    return m_d->m_type == EnumValue;
}

bool TypeEntry::stream() const
{
    return m_d->m_stream;
}

void TypeEntry::setStream(bool b)
{
    m_d->m_stream = b;
}

QString TypeEntry::name() const
{
    return m_d->m_name;
}

// Build the C++ name excluding any inline namespaces
// ("std::__1::shared_ptr" -> "std::shared_ptr"
QString TypeEntryPrivate::shortName() const
{
    if (m_cachedShortName.isEmpty()) {
        QVarLengthArray<const TypeEntry *> parents;
        bool foundInlineNamespace = false;
        for (auto p = m_parent; p != nullptr && p->type() != TypeEntry::TypeSystemType; p = p->parent()) {
            if (p->type() == TypeEntry::NamespaceType
                && static_cast<const NamespaceTypeEntry *>(p)->isInlineNamespace()) {
                foundInlineNamespace = true;
            } else {
                parents.append(p);
            }
        }
        if (foundInlineNamespace) {
            m_cachedShortName.reserve(m_name.size());
            for (int i = parents.size() - 1; i >= 0; --i) {
                m_cachedShortName.append(parents.at(i)->entryName());
                m_cachedShortName.append(QLatin1String("::"));
            }
            m_cachedShortName.append(m_entryName);
        } else {
            m_cachedShortName = m_name;
        }
    }
    return m_cachedShortName;
}

QString TypeEntry::shortName() const
{
    return m_d->shortName();
}

QString TypeEntry::entryName() const
{
    return m_d->m_entryName;
}

TypeEntry::CodeGeneration TypeEntry::codeGeneration() const
{
    return m_d->m_codeGeneration;
}

void TypeEntry::setCodeGeneration(TypeEntry::CodeGeneration cg)
{
    m_d->m_codeGeneration = cg;
}

bool TypeEntry::generateCode() const
{
    return m_d->m_codeGeneration == GenerateCode;
}

int TypeEntry::revision() const
{
    return m_d->m_revision;
}

void TypeEntry::setSbkIndex(int i)
{
    m_d->m_sbkIndex = i;
}

QString TypeEntry::qualifiedCppName() const
{
    return m_d->m_name;
}

QString TypeEntry::targetLangApiName() const
{
    return m_d->m_name;
}

QString TypeEntry::targetLangName() const
{
    if (m_d->m_cachedTargetLangName.isEmpty())
        m_d->m_cachedTargetLangName = buildTargetLangName();
    return m_d->m_cachedTargetLangName;
}

void TypeEntry::setTargetLangName(const QString &n)
{
    m_d->m_cachedTargetLangName = n;
}

QString TypeEntry::buildTargetLangName() const
{
    QString result = m_d->m_entryName;
    for (auto p = parent(); p && p->type() != TypeEntry::TypeSystemType; p = p->parent()) {
        if (NamespaceTypeEntry::isVisibleScope(p)) {
            if (!result.isEmpty())
                result.prepend(QLatin1Char('.'));
            QString n = p->m_d->m_entryName;
            n.replace(QLatin1String("::"), QLatin1String(".")); // Primitive types may have "std::"
            result.prepend(n);
        }
    }
    return result;
}

bool TypeEntry::setRevisionHelper(int r)
{
    const bool changed = m_d->m_revision != r;
    m_d->m_revision = r;
    return changed;
}

int TypeEntry::sbkIndexHelper() const
{
    return m_d->m_sbkIndex;
}

SourceLocation TypeEntry::sourceLocation() const
{
    return m_d->m_sourceLocation;
}

void TypeEntry::setSourceLocation(const SourceLocation &sourceLocation)
{
    m_d->m_sourceLocation = sourceLocation;
}

bool TypeEntry::isUserPrimitive() const
{
    if (!isPrimitive())
        return false;
    const auto *trueType = static_cast<const PrimitiveTypeEntry *>(this);
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    return trueType->isPrimitive() && !trueType->isCppPrimitive()
        && trueType->qualifiedCppName() != u"std::string";
}

bool TypeEntry::isWrapperType() const
{
  return isObject() || isValue() || isSmartPointer();
}

bool TypeEntry::isCppIntegralPrimitive() const
{
    if (!isCppPrimitive())
        return false;
    const auto *trueType = static_cast<const PrimitiveTypeEntry *>(this);
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    QString typeName = trueType->qualifiedCppName();
    return !typeName.contains(u"double")
        && !typeName.contains(u"float")
        && !typeName.contains(u"wchar");
}

bool TypeEntry::isExtendedCppPrimitive() const
{
    if (isCppPrimitive())
        return true;
    if (!isPrimitive())
        return false;
    const auto *trueType = static_cast<const PrimitiveTypeEntry *>(this);
    if (trueType->basicReferencedTypeEntry())
        trueType = trueType->basicReferencedTypeEntry();
    return trueType->qualifiedCppName() == u"std::string";
}

const TypeEntryPrivate *TypeEntry::d_func() const
{
    return m_d.data();
}

TypeEntryPrivate *TypeEntry::d_func()
{
    return m_d.data();
}

QString TypeEntry::targetLangEntryName() const
{
    if (m_d->m_cachedTargetLangEntryName.isEmpty()) {
        m_d->m_cachedTargetLangEntryName = targetLangName();
        const int lastDot = m_d->m_cachedTargetLangEntryName.lastIndexOf(QLatin1Char('.'));
        if (lastDot != -1)
            m_d->m_cachedTargetLangEntryName.remove(0, lastDot + 1);
    }
    return m_d->m_cachedTargetLangEntryName;
}

QString TypeEntry::targetLangPackage() const
{
    return m_d->m_targetLangPackage;
}

void TypeEntry::setTargetLangPackage(const QString &p)
{
    m_d->m_targetLangPackage = p;
}

QString TypeEntry::qualifiedTargetLangName() const
{
    return targetLangPackage() + QLatin1Char('.') + targetLangName();
}

void TypeEntry::setCustomConstructor(const CustomFunction &func)
{
    m_d->m_customConstructor = func;
}

CustomFunction TypeEntry::customConstructor() const
{
    return m_d->m_customConstructor;
}

void TypeEntry::setCustomDestructor(const CustomFunction &func)
{
    m_d->m_customDestructor = func;
}

CustomFunction TypeEntry::customDestructor() const
{
    return m_d->m_customDestructor;
}

bool TypeEntry::isValue() const
{
    return false;
}

bool TypeEntry::isComplex() const
{
    return false;
}

bool TypeEntry::hasCustomConversion() const
{
    return m_d->m_customConversion != nullptr;
}

void TypeEntry::setCustomConversion(CustomConversion* customConversion)
{
    m_d->m_customConversion = customConversion;
}

CustomConversion* TypeEntry::customConversion() const
{
    return m_d->m_customConversion;
}

TypeEntry *TypeEntry::viewOn() const
{
    return m_d->m_viewOn;
}

void TypeEntry::setViewOn(TypeEntry *v)
{
    m_d->m_viewOn = v;
}

TypeEntry *TypeEntry::clone() const
{
    return new TypeEntry(new TypeEntryPrivate(*m_d.data()));
}

// Take over parameters relevant for typedefs
void TypeEntry::useAsTypedef(const TypeEntry *source)
{
    // XML Typedefs are in the global namespace for now.
    m_d->m_parent = source->typeSystemTypeEntry();
    m_d->m_entryName = source->m_d->m_entryName;
    m_d->m_name = source->m_d->m_name;
    m_d->m_targetLangPackage = source->m_d->m_targetLangPackage;
    m_d->m_codeGeneration = source->m_d->m_codeGeneration;
    m_d->m_version = source->m_d->m_version;
}

TypeSystemTypeEntry::TypeSystemTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                         const TypeEntry *parent) :
    TypeEntry(entryName, TypeSystemType, vr, parent)
{
}

TypeSystemTypeEntry::TypeSystemTypeEntry(TypeEntryPrivate *d) :
    TypeEntry(d)
{
}

TypeEntry *TypeSystemTypeEntry::clone() const
{
    return new TypeSystemTypeEntry(new TypeEntryPrivate(*d_func()));
}

// ----------------- VoidTypeEntry
VoidTypeEntry::VoidTypeEntry() :
    TypeEntry(QLatin1String("void"), VoidType, QVersionNumber(0, 0), nullptr)
{
}

VoidTypeEntry::VoidTypeEntry(TypeEntryPrivate *d) :
    TypeEntry(d)
{
}

TypeEntry *VoidTypeEntry::clone() const
{
    return new VoidTypeEntry(new TypeEntryPrivate(*d_func()));
}

VarargsTypeEntry::VarargsTypeEntry() :
    TypeEntry(QLatin1String("..."), VarargsType, QVersionNumber(0, 0), nullptr)
{
}

// ----------------- VarargsTypeEntry
TypeEntry *VarargsTypeEntry::clone() const
{
    return new VarargsTypeEntry(new TypeEntryPrivate(*d_func()));
}

VarargsTypeEntry::VarargsTypeEntry(TypeEntryPrivate *d) :
    TypeEntry(d)
{
}

// ----------------- TemplateArgumentEntry
class TemplateArgumentEntryPrivate : public TypeEntryPrivate
{
public:
    using TypeEntryPrivate::TypeEntryPrivate;

    int m_ordinal = 0;
};

TemplateArgumentEntry::TemplateArgumentEntry(const QString &entryName, const QVersionNumber &vr,
                                             const TypeEntry *parent) :
    TypeEntry(new TemplateArgumentEntryPrivate(entryName, TemplateArgumentType, vr, parent))
{
}

int TemplateArgumentEntry::ordinal() const
{
    S_D(const TemplateArgumentEntry);
    return d->m_ordinal;
}

void TemplateArgumentEntry::setOrdinal(int o)
{
    S_D(TemplateArgumentEntry);
    d->m_ordinal = o;
}

TypeEntry *TemplateArgumentEntry::clone() const
{
    S_D(const TemplateArgumentEntry);
    return new TemplateArgumentEntry(new TemplateArgumentEntryPrivate(*d));
}

TemplateArgumentEntry::TemplateArgumentEntry(TemplateArgumentEntryPrivate *d) :
    TypeEntry(d)
{
}

// ----------------- ArrayTypeEntry
class ArrayTypeEntryPrivate : public TypeEntryPrivate
{
public:
    explicit ArrayTypeEntryPrivate(const TypeEntry *nested_type, const QVersionNumber &vr,
                                   const TypeEntry *parent) :
        TypeEntryPrivate(QLatin1String("Array"), TypeEntry::ArrayType, vr, parent),
        m_nestedType(nested_type)
    {
    }

    const TypeEntry *m_nestedType;
};

ArrayTypeEntry::ArrayTypeEntry(const TypeEntry *nested_type, const QVersionNumber &vr,
                               const TypeEntry *parent) :
    TypeEntry(new ArrayTypeEntryPrivate(nested_type, vr, parent))
{
    Q_ASSERT(nested_type);
}

void ArrayTypeEntry::setNestedTypeEntry(TypeEntry *nested)
{
    S_D(ArrayTypeEntry);
    d->m_nestedType = nested;
}

const TypeEntry *ArrayTypeEntry::nestedTypeEntry() const
{
    S_D(const ArrayTypeEntry);
    return d->m_nestedType;
}

QString ArrayTypeEntry::buildTargetLangName() const
{
    S_D(const ArrayTypeEntry);
    return d->m_nestedType->targetLangName() + QLatin1String("[]");
}

QString ArrayTypeEntry::targetLangApiName() const
{
    S_D(const ArrayTypeEntry);
    return d->m_nestedType->isPrimitive()
        ? d->m_nestedType->targetLangApiName() + QLatin1String("Array")
        : QLatin1String("jobjectArray");
}

TypeEntry *ArrayTypeEntry::clone() const
{
    S_D(const ArrayTypeEntry);
    return new ArrayTypeEntry(new ArrayTypeEntryPrivate(*d));
}

ArrayTypeEntry::ArrayTypeEntry(ArrayTypeEntryPrivate *d) :
    TypeEntry(d)
{
}

// ----------------- PrimitiveTypeEntry
class PrimitiveTypeEntryPrivate : public TypeEntryPrivate
{
public:
    PrimitiveTypeEntryPrivate(const QString &entryName, const QVersionNumber &vr,
                              const TypeEntry *parent) :
        TypeEntryPrivate(entryName, TypeEntry::PrimitiveType, vr, parent),
        m_preferredTargetLangType(true)
    {
    }

    QString m_targetLangApiName;
    QString m_defaultConstructor;
    uint m_preferredTargetLangType : 1;
    PrimitiveTypeEntry* m_referencedTypeEntry = nullptr;
};

PrimitiveTypeEntry::PrimitiveTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                       const TypeEntry *parent) :
    TypeEntry(new PrimitiveTypeEntryPrivate(entryName, vr, parent))
{
}

QString PrimitiveTypeEntry::targetLangApiName() const
{
    S_D(const PrimitiveTypeEntry);
    return d->m_targetLangApiName;
}

void PrimitiveTypeEntry::setTargetLangApiName(const QString &targetLangApiName)
{
    S_D(PrimitiveTypeEntry);
    d->m_targetLangApiName = targetLangApiName;
}

QString PrimitiveTypeEntry::defaultConstructor() const
{
    S_D(const PrimitiveTypeEntry);
    return d->m_defaultConstructor;
}

void PrimitiveTypeEntry::setDefaultConstructor(const QString &defaultConstructor)
{
    S_D(PrimitiveTypeEntry);
    d->m_defaultConstructor = defaultConstructor;
}

bool PrimitiveTypeEntry::hasDefaultConstructor() const
{
    S_D(const PrimitiveTypeEntry);
    return !d->m_defaultConstructor.isEmpty();
}

PrimitiveTypeEntry *PrimitiveTypeEntry::referencedTypeEntry() const
{
    S_D(const PrimitiveTypeEntry);
    return d->m_referencedTypeEntry;
}

void PrimitiveTypeEntry::setReferencedTypeEntry(PrimitiveTypeEntry *referencedTypeEntry)
{
    S_D(PrimitiveTypeEntry);
    d->m_referencedTypeEntry = referencedTypeEntry;
}

PrimitiveTypeEntry *PrimitiveTypeEntry::basicReferencedTypeEntry() const
{
    S_D(const PrimitiveTypeEntry);
    if (!d->m_referencedTypeEntry)
        return nullptr;

    PrimitiveTypeEntry *baseReferencedTypeEntry = d->m_referencedTypeEntry->basicReferencedTypeEntry();
    return baseReferencedTypeEntry ? baseReferencedTypeEntry : d->m_referencedTypeEntry;
}

bool PrimitiveTypeEntry::preferredTargetLangType() const
{
    S_D(const PrimitiveTypeEntry);
    return d->m_preferredTargetLangType;
}

void PrimitiveTypeEntry::setPreferredTargetLangType(bool b)
{
    S_D(PrimitiveTypeEntry);
    d->m_preferredTargetLangType = b;
}

TypeEntry *PrimitiveTypeEntry::clone() const
{
    S_D(const PrimitiveTypeEntry);
    return new PrimitiveTypeEntry(new PrimitiveTypeEntryPrivate(*d));
}

PrimitiveTypeEntry::PrimitiveTypeEntry(PrimitiveTypeEntryPrivate *d)
    : TypeEntry(d)
{
}

// ----------------- EnumTypeEntry
class EnumTypeEntryPrivate : public TypeEntryPrivate
{
public:
    using TypeEntryPrivate::TypeEntryPrivate;

    const EnumValueTypeEntry *m_nullValue = nullptr;
    QStringList m_rejectedEnums;
    FlagsTypeEntry *m_flags = nullptr;
};

EnumTypeEntry::EnumTypeEntry(const QString &entryName,
                             const QVersionNumber &vr,
                             const TypeEntry *parent) :
    TypeEntry(new EnumTypeEntryPrivate(entryName, EnumType, vr, parent))
{
}

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

const EnumValueTypeEntry *EnumTypeEntry::nullValue() const
{
    S_D(const EnumTypeEntry);
    return d->m_nullValue;
}

void EnumTypeEntry::setNullValue(const EnumValueTypeEntry *n)
{
    S_D(EnumTypeEntry);
    d->m_nullValue = n;
}

void EnumTypeEntry::setFlags(FlagsTypeEntry *flags)
{
    S_D(EnumTypeEntry);
    d->m_flags = flags;
}

FlagsTypeEntry *EnumTypeEntry::flags() const
{
    S_D(const EnumTypeEntry);
    return d->m_flags;
}

bool EnumTypeEntry::isEnumValueRejected(const QString &name) const
{
    S_D(const EnumTypeEntry);
    return d->m_rejectedEnums.contains(name);
}

void EnumTypeEntry::addEnumValueRejection(const QString &name)
{
    S_D(EnumTypeEntry);
    d->m_rejectedEnums << name;
}

QStringList EnumTypeEntry::enumValueRejections() const
{
    S_D(const EnumTypeEntry);
    return d->m_rejectedEnums;
}

QString EnumTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

TypeEntry *EnumTypeEntry::clone() const
{
    S_D(const EnumTypeEntry);
    return new EnumTypeEntry(new EnumTypeEntryPrivate(*d));
}

EnumTypeEntry::EnumTypeEntry(EnumTypeEntryPrivate *d) :
    TypeEntry(d)
{
}

// ----------------- EnumValueTypeEntryPrivate
class EnumValueTypeEntryPrivate : public TypeEntryPrivate
{
public:
    EnumValueTypeEntryPrivate(const QString &name, const QString &value,
                              const EnumTypeEntry *enclosingEnum,
                              bool isScopedEnum,
                              const QVersionNumber &vr) :
        TypeEntryPrivate(name, TypeEntry::EnumValue, vr,
                         isScopedEnum ? enclosingEnum : enclosingEnum->parent()),
               m_value(value),
               m_enclosingEnum(enclosingEnum)
    {
    }

    QString m_value;
    const EnumTypeEntry *m_enclosingEnum;
};

EnumValueTypeEntry::EnumValueTypeEntry(const QString &name, const QString &value,
                                       const EnumTypeEntry *enclosingEnum,
                                       bool isScopedEnum,
                                       const QVersionNumber &vr) :
    TypeEntry(new EnumValueTypeEntryPrivate(name, value, enclosingEnum, isScopedEnum, vr))
{
}

QString EnumValueTypeEntry::value() const
{
    S_D(const EnumValueTypeEntry);
    return d->m_value;
}

const EnumTypeEntry *EnumValueTypeEntry::enclosingEnum() const
{
    S_D(const EnumValueTypeEntry);
    return d->m_enclosingEnum;
}

TypeEntry *EnumValueTypeEntry::clone() const
{
    S_D(const EnumValueTypeEntry);
    return new EnumValueTypeEntry(new EnumValueTypeEntryPrivate(*d));
}

EnumValueTypeEntry::EnumValueTypeEntry(EnumValueTypeEntryPrivate *d) :
    TypeEntry(d)
{
}

// ----------------- FlagsTypeEntry
class FlagsTypeEntryPrivate : public TypeEntryPrivate
{
public:
    using TypeEntryPrivate::TypeEntryPrivate;

    QString m_originalName;
    QString m_flagsName;
    EnumTypeEntry *m_enum = nullptr;
};

FlagsTypeEntry::FlagsTypeEntry(const QString &entryName, const QVersionNumber &vr,
                               const TypeEntry *parent) :
    TypeEntry(new FlagsTypeEntryPrivate(entryName, FlagsType, vr, parent))
{
}

QString FlagsTypeEntry::buildTargetLangName() const
{
    S_D(const FlagsTypeEntry);
    QString on = d->m_originalName;
    on.replace(QLatin1String("::"), QLatin1String("."));
    return on;
}

FlagsTypeEntry::FlagsTypeEntry(FlagsTypeEntryPrivate *d) :
    TypeEntry(d)
{
}

QString FlagsTypeEntry::targetLangApiName() const
{
    return QLatin1String("jint");
}

QString FlagsTypeEntry::originalName() const
{
    S_D(const FlagsTypeEntry);
    return d->m_originalName;
}

void FlagsTypeEntry::setOriginalName(const QString &s)
{
    S_D(FlagsTypeEntry);
    d->m_originalName = s;
}

QString FlagsTypeEntry::flagsName() const
{
    S_D(const FlagsTypeEntry);
    return d->m_flagsName;
}

void FlagsTypeEntry::setFlagsName(const QString &name)
{
    S_D(FlagsTypeEntry);
    d->m_flagsName = name;
}

EnumTypeEntry *FlagsTypeEntry::originator() const
{
    S_D(const FlagsTypeEntry);
    return d->m_enum;
}

void FlagsTypeEntry::setOriginator(EnumTypeEntry *e)
{
    S_D(FlagsTypeEntry);
    d->m_enum = e;
}

TypeEntry *FlagsTypeEntry::clone() const
{
    S_D(const FlagsTypeEntry);
    return new FlagsTypeEntry(new FlagsTypeEntryPrivate(*d));
}

// ----------------- ConstantValueTypeEntry
ConstantValueTypeEntry::ConstantValueTypeEntry(const QString& name,
                                               const TypeEntry *parent) :
    TypeEntry(name, ConstantValueType, QVersionNumber(0, 0), parent)
{
}

ConstantValueTypeEntry::ConstantValueTypeEntry(TypeEntryPrivate *d) :
    TypeEntry(d)
{
}

// ----------------- ComplexTypeEntry
class ComplexTypeEntryPrivate : public TypeEntryPrivate
{
public:
    ComplexTypeEntryPrivate(const QString &entryName, TypeEntry::Type t,
                            const QVersionNumber &vr,
                            const TypeEntry *parent) :
        TypeEntryPrivate(entryName, t, vr, parent),
        m_qualifiedCppName(buildName(entryName, parent)),
        m_polymorphicBase(false),
        m_genericClass(false),
        m_deleteInMainThread(false)
    {
    }

    AddedFunctionList m_addedFunctions;
    FunctionModificationList m_functionMods;
    FieldModificationList m_fieldMods;
    QList<TypeSystemProperty> m_properties;
    QString m_defaultConstructor;
    QString m_defaultSuperclass;
    QString m_qualifiedCppName;

    uint m_polymorphicBase : 1;
    uint m_genericClass : 1;
    uint m_deleteInMainThread : 1;

    QString m_polymorphicIdValue;
    QString m_targetType;
    ComplexTypeEntry::TypeFlags m_typeFlags;
    ComplexTypeEntry::CopyableFlag m_copyableFlag = ComplexTypeEntry::Unknown;
    QString m_hashFunction;

    const ComplexTypeEntry* m_baseContainerType = nullptr;
    // For class functions
    TypeSystem::ExceptionHandling m_exceptionHandling = TypeSystem::ExceptionHandling::Unspecified;
    TypeSystem::AllowThread m_allowThread = TypeSystem::AllowThread::Unspecified;
};

ComplexTypeEntry::ComplexTypeEntry(const QString &entryName, TypeEntry::Type t,
                                   const QVersionNumber &vr,
                                   const TypeEntry *parent) :
    TypeEntry(new ComplexTypeEntryPrivate(entryName, t, vr, parent))
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

void ComplexTypeEntry::setTypeFlags(TypeFlags flags)
{
    S_D(ComplexTypeEntry);
    d->m_typeFlags = flags;
}

ComplexTypeEntry::TypeFlags ComplexTypeEntry::typeFlags() const
{
    S_D(const ComplexTypeEntry);
    return d->m_typeFlags;
}

FunctionModificationList ComplexTypeEntry::functionModifications() const
{
    S_D(const ComplexTypeEntry);
    return d->m_functionMods;
}

void ComplexTypeEntry::setFunctionModifications(const FunctionModificationList &functionModifications)
{
    S_D(ComplexTypeEntry);
    d->m_functionMods = functionModifications;
}

void ComplexTypeEntry::addFunctionModification(const FunctionModification &functionModification)
{
    S_D(ComplexTypeEntry);
    d->m_functionMods << functionModification;
}

FunctionModificationList ComplexTypeEntry::functionModifications(const QString &signature) const
{
    S_D(const ComplexTypeEntry);
    FunctionModificationList lst;
    for (int i = 0; i < d->m_functionMods.count(); ++i) {
        const FunctionModification &mod = d->m_functionMods.at(i);
        if (mod.matches(signature))
            lst << mod;
    }
    return lst;
}

AddedFunctionList ComplexTypeEntry::addedFunctions() const
{
    S_D(const ComplexTypeEntry);
    return d->m_addedFunctions;
}

void ComplexTypeEntry::setAddedFunctions(const AddedFunctionList &addedFunctions)
{
    S_D(ComplexTypeEntry);
    d->m_addedFunctions = addedFunctions;
}

void ComplexTypeEntry::addNewFunction(const AddedFunctionPtr &addedFunction)
{
    S_D(ComplexTypeEntry);
    d->m_addedFunctions << addedFunction;
}

void ComplexTypeEntry::setFieldModifications(const FieldModificationList &mods)
{
    S_D(ComplexTypeEntry);
    d->m_fieldMods = mods;
}

FieldModificationList ComplexTypeEntry::fieldModifications() const
{
    S_D(const ComplexTypeEntry);
    return d->m_fieldMods;
}

const QList<TypeSystemProperty> &ComplexTypeEntry::properties() const
{
    S_D(const ComplexTypeEntry);
    return d->m_properties;
}

void ComplexTypeEntry::addProperty(const TypeSystemProperty &p)
{
    S_D(ComplexTypeEntry);
    d->m_properties.append(p);
}

QString ComplexTypeEntry::defaultSuperclass() const
{
    S_D(const ComplexTypeEntry);
    return d->m_defaultSuperclass;
}

void ComplexTypeEntry::setDefaultSuperclass(const QString &sc)
{
    S_D(ComplexTypeEntry);
    d->m_defaultSuperclass = sc;
}

QString ComplexTypeEntry::qualifiedCppName() const
{
    S_D(const ComplexTypeEntry);
    return d->m_qualifiedCppName;
}

void ComplexTypeEntry::setIsPolymorphicBase(bool on)
{
    S_D(ComplexTypeEntry);
    d->m_polymorphicBase = on;
}

bool ComplexTypeEntry::isPolymorphicBase() const
{
    S_D(const ComplexTypeEntry);
    return d->m_polymorphicBase;
}

void ComplexTypeEntry::setPolymorphicIdValue(const QString &value)
{
    S_D(ComplexTypeEntry);
    d->m_polymorphicIdValue = value;
}

QString ComplexTypeEntry::polymorphicIdValue() const
{
    S_D(const ComplexTypeEntry);
    return d->m_polymorphicIdValue;
}

QString ComplexTypeEntry::targetType() const
{
    S_D(const ComplexTypeEntry);
    return d->m_targetType;
}

void ComplexTypeEntry::setTargetType(const QString &code)
{
    S_D(ComplexTypeEntry);
    d->m_targetType = code;
}

bool ComplexTypeEntry::isGenericClass() const
{
    S_D(const ComplexTypeEntry);
    return d->m_genericClass;
}

void ComplexTypeEntry::setGenericClass(bool isGeneric)
{
    S_D(ComplexTypeEntry);
    d->m_genericClass = isGeneric;
}

bool ComplexTypeEntry::deleteInMainThread() const
{
    S_D(const ComplexTypeEntry);
    return d->m_deleteInMainThread;
}

void ComplexTypeEntry::setDeleteInMainThread(bool dmt)
{
    S_D(ComplexTypeEntry);
    d->m_deleteInMainThread = dmt;
}

ComplexTypeEntry::CopyableFlag ComplexTypeEntry::copyable() const
{
    S_D(const ComplexTypeEntry);
    return d->m_copyableFlag;
}

void ComplexTypeEntry::setCopyable(ComplexTypeEntry::CopyableFlag flag)
{
    S_D(ComplexTypeEntry);
    d->m_copyableFlag = flag;
}

QString ComplexTypeEntry::hashFunction() const
{
    S_D(const ComplexTypeEntry);
    return d->m_hashFunction;
}

void ComplexTypeEntry::setHashFunction(const QString &hashFunction)
{
    S_D(ComplexTypeEntry);
    d->m_hashFunction = hashFunction;
}

void ComplexTypeEntry::setBaseContainerType(const ComplexTypeEntry *baseContainer)
{
    S_D(ComplexTypeEntry);
    d->m_baseContainerType = baseContainer;
}

const ComplexTypeEntry *ComplexTypeEntry::baseContainerType() const
{
    S_D(const ComplexTypeEntry);
    return d->m_baseContainerType;
}

TypeSystem::ExceptionHandling ComplexTypeEntry::exceptionHandling() const
{
    S_D(const ComplexTypeEntry);
    return d->m_exceptionHandling;
}

void ComplexTypeEntry::setExceptionHandling(TypeSystem::ExceptionHandling e)
{
    S_D(ComplexTypeEntry);
    d->m_exceptionHandling = e;
}

TypeSystem::AllowThread ComplexTypeEntry::allowThread() const
{
    S_D(const ComplexTypeEntry);
    return d->m_allowThread;
}

void ComplexTypeEntry::setAllowThread(TypeSystem::AllowThread allowThread)
{
    S_D(ComplexTypeEntry);
    d->m_allowThread = allowThread;
}

void ComplexTypeEntry::setDefaultConstructor(const QString& defaultConstructor)
{
    S_D(ComplexTypeEntry);
    d->m_defaultConstructor = defaultConstructor;
}

QString ComplexTypeEntry::defaultConstructor() const
{
    S_D(const ComplexTypeEntry);
    return d->m_defaultConstructor;
}

bool ComplexTypeEntry::hasDefaultConstructor() const
{
    S_D(const ComplexTypeEntry);
    return !d->m_defaultConstructor.isEmpty();
}

TypeEntry *ComplexTypeEntry::clone() const
{
    S_D(const ComplexTypeEntry);
    return new ComplexTypeEntry(new ComplexTypeEntryPrivate(*d));
}

// Take over parameters relevant for typedefs
void ComplexTypeEntry::useAsTypedef(const ComplexTypeEntry *source)
{
    S_D(ComplexTypeEntry);
    TypeEntry::useAsTypedef(source);
    d->m_qualifiedCppName = source->qualifiedCppName();
    d->m_targetType = source->targetType();
}

ComplexTypeEntry::ComplexTypeEntry(ComplexTypeEntryPrivate *d) :
    TypeEntry(d)
{
}

TypeEntry *ConstantValueTypeEntry::clone() const
{
    return new ConstantValueTypeEntry(new TypeEntryPrivate(*d_func()));
}


// ----------------- TypedefEntry
/* A typedef entry allows for specifying template specializations in the
 * typesystem XML file. */
class TypedefEntryPrivate : public ComplexTypeEntryPrivate
{
public:
    TypedefEntryPrivate(const QString &entryName,
                        const QString &sourceType,
                        const QVersionNumber &vr,
                        const TypeEntry *parent) :
        ComplexTypeEntryPrivate(entryName, TypeEntry::TypedefType,
                                vr, parent),
        m_sourceType(sourceType)
    {
    }

    QString m_sourceType;
    ComplexTypeEntry *m_source = nullptr;
    ComplexTypeEntry *m_target = nullptr;
};

TypedefEntry::TypedefEntry(const QString &entryName, const QString &sourceType,
                           const QVersionNumber &vr, const TypeEntry *parent) :
    ComplexTypeEntry(new TypedefEntryPrivate(entryName, sourceType, vr, parent))
{
}

QString TypedefEntry::sourceType() const
{
    S_D(const TypedefEntry);
    return d->m_sourceType;
}

void TypedefEntry::setSourceType(const QString &s)
{
    S_D(TypedefEntry);
    d->m_sourceType =s;
}

TypeEntry *TypedefEntry::clone() const
{
    S_D(const TypedefEntry);
    return new TypedefEntry(new TypedefEntryPrivate(*d));
}

ComplexTypeEntry *TypedefEntry::source() const
{
    S_D(const TypedefEntry);
    return d->m_source;
}

void TypedefEntry::setSource(ComplexTypeEntry *source)
{
    S_D(TypedefEntry);
    d->m_source = source;
}

ComplexTypeEntry *TypedefEntry::target() const
{
    S_D(const TypedefEntry);
    return d->m_target;
}

void TypedefEntry::setTarget(ComplexTypeEntry *target)
{
    S_D(TypedefEntry);
    d->m_target = target;
}

TypedefEntry::TypedefEntry(TypedefEntryPrivate *d) :
    ComplexTypeEntry(d)
{
}

// ----------------- ContainerTypeEntry
class ContainerTypeEntryPrivate : public ComplexTypeEntryPrivate
{
public:
    ContainerTypeEntryPrivate(const QString &entryName,
                              ContainerTypeEntry::ContainerKind containerKind,
                              const QVersionNumber &vr,
                              const TypeEntry *parent) :
        ComplexTypeEntryPrivate(entryName, TypeEntry::ContainerType, vr, parent),
        m_containerKind(containerKind)
    {
    }

    ContainerTypeEntry::ContainerKind m_containerKind;
};

ContainerTypeEntry::ContainerTypeEntry(const QString &entryName, ContainerKind containerKind,
                                       const QVersionNumber &vr,
                                       const TypeEntry *parent) :
    ComplexTypeEntry(new ContainerTypeEntryPrivate(entryName, containerKind, vr, parent))
{
    setCodeGeneration(GenerateForSubclass);
}

ContainerTypeEntry::ContainerKind ContainerTypeEntry::containerKind() const
{
    S_D(const ContainerTypeEntry);
    return d->m_containerKind;
}

QString ContainerTypeEntry::qualifiedCppName() const
{
    S_D(const ContainerTypeEntry);
    if (d->m_containerKind == StringListContainer)
        return QLatin1String("QStringList");
    return ComplexTypeEntry::qualifiedCppName();
}

TypeEntry *ContainerTypeEntry::clone() const
{
    S_D(const ContainerTypeEntry);
    return new ContainerTypeEntry(new ContainerTypeEntryPrivate(*d));
}

ContainerTypeEntry::ContainerTypeEntry(ContainerTypeEntryPrivate *d) :
    ComplexTypeEntry(d)
{
}

QString ContainerTypeEntry::typeName() const
{
    S_D(const ContainerTypeEntry);
    switch (d->m_containerKind) {
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
        break;
    }
    return QLatin1String("?");
}

// ----------------- SmartPointerTypeEntry
class SmartPointerTypeEntryPrivate : public ComplexTypeEntryPrivate
{
public:
    SmartPointerTypeEntryPrivate(const QString &entryName,
                                 const QString &getterName,
                                 const QString &smartPointerType,
                                 const QString &refCountMethodName,
                                 const QVersionNumber &vr, const TypeEntry *parent) :
        ComplexTypeEntryPrivate(entryName, TypeEntry::SmartPointerType, vr, parent),
        m_getterName(getterName),
        m_smartPointerType(smartPointerType),
        m_refCountMethodName(refCountMethodName)
    {
    }

    QString m_getterName;
    QString m_smartPointerType;
    QString m_refCountMethodName;
    SmartPointerTypeEntry::Instantiations m_instantiations;
};

SmartPointerTypeEntry::SmartPointerTypeEntry(const QString &entryName,
                                             const QString &getterName,
                                             const QString &smartPointerType,
                                             const QString &refCountMethodName,
                                             const QVersionNumber &vr, const TypeEntry *parent) :
    ComplexTypeEntry(new SmartPointerTypeEntryPrivate(entryName, getterName, smartPointerType,
                                                      refCountMethodName, vr, parent))
{
}

QString SmartPointerTypeEntry::getter() const
{
    S_D(const SmartPointerTypeEntry);
    return d->m_getterName;
}

QString SmartPointerTypeEntry::refCountMethodName() const
{
    S_D(const SmartPointerTypeEntry);
    return d->m_refCountMethodName;
}

TypeEntry *SmartPointerTypeEntry::clone() const
{
    S_D(const SmartPointerTypeEntry);
    return new SmartPointerTypeEntry(new SmartPointerTypeEntryPrivate(*d));
}

SmartPointerTypeEntry::Instantiations SmartPointerTypeEntry::instantiations() const
{
    S_D(const SmartPointerTypeEntry);
    return d->m_instantiations;
}

void SmartPointerTypeEntry::setInstantiations(const Instantiations &i)
{
    S_D(SmartPointerTypeEntry);
    d->m_instantiations = i;
}

SmartPointerTypeEntry::SmartPointerTypeEntry(SmartPointerTypeEntryPrivate *d) :
    ComplexTypeEntry(d)
{
}

bool SmartPointerTypeEntry::matchesInstantiation(const TypeEntry *e) const
{
    S_D(const SmartPointerTypeEntry);
    return d->m_instantiations.isEmpty() || d->m_instantiations.contains(e);
}

// ----------------- NamespaceTypeEntry
class NamespaceTypeEntryPrivate : public ComplexTypeEntryPrivate
{
public:
    using ComplexTypeEntryPrivate::ComplexTypeEntryPrivate;

    QRegularExpression m_filePattern;
    const NamespaceTypeEntry *m_extends = nullptr;
    TypeSystem::Visibility m_visibility = TypeSystem::Visibility::Auto;
    bool m_hasPattern = false;
    bool m_inlineNamespace = false;
    bool m_generateUsing = true; // Whether to generate "using namespace" into wrapper
};

NamespaceTypeEntry::NamespaceTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                       const TypeEntry *parent) :
    ComplexTypeEntry(new NamespaceTypeEntryPrivate(entryName, NamespaceType, vr, parent))
{
}

TypeEntry *NamespaceTypeEntry::clone() const
{
    S_D(const NamespaceTypeEntry);
    return new NamespaceTypeEntry(new NamespaceTypeEntryPrivate(*d));
}

const NamespaceTypeEntry *NamespaceTypeEntry::extends() const
{
    S_D(const NamespaceTypeEntry);
    return d->m_extends;
}

void NamespaceTypeEntry::setExtends(const NamespaceTypeEntry *e)
{
    S_D(NamespaceTypeEntry);
    d->m_extends = e;
}

const QRegularExpression &NamespaceTypeEntry::filePattern() const
{
    S_D(const NamespaceTypeEntry);
    return d->m_filePattern;
}

void NamespaceTypeEntry::setFilePattern(const QRegularExpression &r)
{
    S_D(NamespaceTypeEntry);
    d->m_filePattern = r;
    d->m_hasPattern = !d->m_filePattern.pattern().isEmpty();
    if (d->m_hasPattern)
        d->m_filePattern.optimize();
}

bool NamespaceTypeEntry::hasPattern() const
{
    S_D(const NamespaceTypeEntry);
    return d->m_hasPattern;
}

NamespaceTypeEntry::NamespaceTypeEntry(NamespaceTypeEntryPrivate *d) :
    ComplexTypeEntry(d)
{
}

bool NamespaceTypeEntry::matchesFile(const QString &needle) const
{
    S_D(const NamespaceTypeEntry);
    return d->m_filePattern.match(needle).hasMatch();
}

bool NamespaceTypeEntry::isVisible() const
{
    S_D(const NamespaceTypeEntry);
    return d->m_visibility == TypeSystem::Visibility::Visible
        || (d->m_visibility == TypeSystem::Visibility::Auto && !d->m_inlineNamespace);
}

void NamespaceTypeEntry::setVisibility(TypeSystem::Visibility v)
{
    S_D(NamespaceTypeEntry);
    d->m_visibility = v;
}

bool NamespaceTypeEntry::isInlineNamespace() const
{
    S_D(const NamespaceTypeEntry);
    return d->m_inlineNamespace;
}

void NamespaceTypeEntry::setInlineNamespace(bool i)
{
    S_D(NamespaceTypeEntry);
    d->m_inlineNamespace = i;
}

bool NamespaceTypeEntry::isVisibleScope(const TypeEntry *e)
{
    return e->type() != TypeEntry::NamespaceType
        || static_cast<const NamespaceTypeEntry *>(e)->isVisible();
}

bool NamespaceTypeEntry::generateUsing() const
{
    S_D(const NamespaceTypeEntry);
    return d->m_generateUsing;
}

void NamespaceTypeEntry::setGenerateUsing(bool generateUsing)
{
    S_D(NamespaceTypeEntry);
    d->m_generateUsing = generateUsing;
}

// ----------------- ValueTypeEntry
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
    S_D(const ComplexTypeEntry);
    return new ValueTypeEntry(new ComplexTypeEntryPrivate(*d));
}

ValueTypeEntry::ValueTypeEntry(ComplexTypeEntryPrivate *d) :
    ComplexTypeEntry(d)
{
}

ValueTypeEntry::ValueTypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                               const TypeEntry *parent) :
    ComplexTypeEntry(entryName, t, vr, parent)
{
}

// ----------------- CustomConversion
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

// ----------------- FunctionTypeEntry
class FunctionTypeEntryPrivate : public TypeEntryPrivate
{
public:
    FunctionTypeEntryPrivate(const QString &entryName, const QString &signature,
                             const QVersionNumber &vr,
                             const TypeEntry *parent) :
         TypeEntryPrivate(entryName,  TypeEntry::FunctionType, vr, parent),
         m_signatures(signature)
    {
    }

    QStringList m_signatures;
};

FunctionTypeEntry::FunctionTypeEntry(const QString &entryName, const QString &signature,
                                     const QVersionNumber &vr,
                                     const TypeEntry *parent) :
    TypeEntry(new FunctionTypeEntryPrivate(entryName, signature, vr, parent))
{
}

void FunctionTypeEntry::addSignature(const QString &signature)
{
    S_D(FunctionTypeEntry);
    d->m_signatures << signature;
}

const QStringList &FunctionTypeEntry::signatures() const
{
     S_D(const FunctionTypeEntry);
     return d->m_signatures;
}

bool FunctionTypeEntry::hasSignature(const QString &signature) const
{
    S_D(const FunctionTypeEntry);
    return d->m_signatures.contains(signature);
}

TypeEntry *FunctionTypeEntry::clone() const
{
    S_D(const FunctionTypeEntry);
    return new FunctionTypeEntry(new FunctionTypeEntryPrivate(*d));
}

FunctionTypeEntry::FunctionTypeEntry(FunctionTypeEntryPrivate *d) :
    TypeEntry(d)
{
}

// ----------------- ObjectTypeEntry
ObjectTypeEntry::ObjectTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                 const TypeEntry *parent)
    : ComplexTypeEntry(entryName, ObjectType, vr, parent)
{
}

TypeEntry *ObjectTypeEntry::clone() const
{
    S_D(const ComplexTypeEntry);
    return new ObjectTypeEntry(new ComplexTypeEntryPrivate(*d));
}

ObjectTypeEntry::ObjectTypeEntry(ComplexTypeEntryPrivate *d) :
    ComplexTypeEntry(d)
{
}

#ifndef QT_NO_DEBUG_STREAM

#define FORMAT_BOOL(name, var) \
    if (var) \
         debug << ", [" << name << ']';

#define FORMAT_NONEMPTY_STRING(name, var) \
    if (!var.isEmpty()) \
         debug << ", " << name << "=\"" << var << '"';

#define FORMAT_LIST_SIZE(name, var) \
    if (!var.isEmpty()) \
         debug << ", " << var.size() << ' '  << name;

template <class Container, class Separator>
static void formatList(QDebug &d, const char *name, const Container &c, Separator sep)
{
    if (const int size = c.size()) {
        d << ", " << name << '[' << size << "]=(";
        for (int i = 0; i < size; ++i) {
            if (i)
                d << sep;
             d << c.at(i);
        }
        d << ')';
    }
}

void TypeEntry::formatDebug(QDebug &debug) const
{
    const QString cppName = qualifiedCppName();
    debug << '"' << m_d->m_name << '"';
    if (m_d->m_name != cppName)
        debug << "\", cppName=\"" << cppName << '"';
    debug << ", type=" << m_d->m_type << ", codeGeneration="
        << m_d->m_codeGeneration << ", target=\"" << targetLangName() << '"';
    FORMAT_NONEMPTY_STRING("package", m_d->m_targetLangPackage)
    FORMAT_BOOL("stream", m_d->m_stream)
    FORMAT_LIST_SIZE("codeSnips", m_d->m_codeSnips)
    FORMAT_NONEMPTY_STRING("conversionRule", m_d->m_conversionRule)
    if (m_d->m_viewOn)
       debug << ", views=" << m_d->m_viewOn->name();
    if (!m_d->m_version.isNull() && m_d->m_version > QVersionNumber(0, 0))
        debug << ", version=" << m_d->m_version;
    if (m_d->m_revision)
        debug << ", revision=" << m_d->m_revision;
    if (m_d->m_sbkIndex)
        debug << ", sbkIndex=" << m_d->m_sbkIndex;
    if (m_d->m_include.isValid())
        debug << ", include=" << m_d->m_include;
    formatList(debug, "extraIncludes", m_d->m_extraIncludes, ", ");
}

void ComplexTypeEntry::formatDebug(QDebug &debug) const
{
    S_D(const ComplexTypeEntry);

    TypeEntry::formatDebug(debug);
    FORMAT_BOOL("polymorphicBase", d->m_polymorphicBase)
    FORMAT_BOOL("genericClass", d->m_genericClass)
    FORMAT_BOOL("deleteInMainThread", d->m_deleteInMainThread)
    if (d->m_typeFlags != 0)
        debug << ", typeFlags=" << d->m_typeFlags;
    debug << ", copyableFlag=" << d->m_copyableFlag
        << ", except=" << int(d->m_exceptionHandling);
    FORMAT_NONEMPTY_STRING("defaultSuperclass", d->m_defaultSuperclass)
    FORMAT_NONEMPTY_STRING("polymorphicIdValue", d->m_polymorphicIdValue)
    FORMAT_NONEMPTY_STRING("targetType", d->m_targetType)
    FORMAT_NONEMPTY_STRING("hash", d->m_hashFunction)
    FORMAT_LIST_SIZE("addedFunctions", d->m_addedFunctions)
    formatList(debug, "functionMods", d->m_functionMods, ", ");
    FORMAT_LIST_SIZE("fieldMods", d->m_fieldMods)
}

void TypedefEntry::formatDebug(QDebug &debug) const
{
    S_D(const TypedefEntry);

    ComplexTypeEntry::formatDebug(debug);
    debug << ", sourceType=\"" << d->m_sourceType << '"'
        << ", source=" << d->m_source << ", target=" << d->m_target;
}

void EnumTypeEntry::formatDebug(QDebug &debug) const
{
    S_D(const EnumTypeEntry);

    TypeEntry::formatDebug(debug);
    if (d->m_flags)
        debug << ", flags=(" << d->m_flags << ')';
}

void NamespaceTypeEntry::formatDebug(QDebug &debug) const
{
    S_D(const NamespaceTypeEntry);

    ComplexTypeEntry::formatDebug(debug);
    auto pattern = d->m_filePattern.pattern();
    FORMAT_NONEMPTY_STRING("pattern", pattern)
    debug << ",visibility=" << d->m_visibility;
    if (d->m_inlineNamespace)
        debug << "[inline]";
}

void ContainerTypeEntry::formatDebug(QDebug &debug) const
{
    S_D(const ContainerTypeEntry);

    ComplexTypeEntry::formatDebug(debug);
    debug << ", type=" << d->m_containerKind << ",\"" << typeName() << '"';
}

void SmartPointerTypeEntry::formatDebug(QDebug &debug) const
{
    S_D(const SmartPointerTypeEntry);

    ComplexTypeEntry::formatDebug(debug);
    if (!d->m_instantiations.isEmpty()) {
        debug << ", instantiations[" << d->m_instantiations.size() << "]=(";
        for (auto i : d->m_instantiations)
            debug << i->name() << ',';
        debug << ')';
    }
}

QDebug operator<<(QDebug d, const TypeEntry *te)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "TypeEntry(";
    if (te)
        te->formatDebug(d);
    else
        d << '0';
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const TemplateEntry *te)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "TemplateEntry(";
    if (te) {
        d << '"' << te->name() << '"';
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // QT_NO_DEBUG_STREAM
