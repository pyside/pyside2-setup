/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt for Python project.
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

#ifndef TYPESYSTEM_H
#define TYPESYSTEM_H

#include "typesystem_enums.h"
#include "typesystem_typedefs.h"
#include "include.h"

#include <QtCore/QHash>
#include <QtCore/qobjectdefs.h>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QVersionNumber>

//Used to identify the conversion rule to avoid break API
#define TARGET_CONVERSION_RULE_FLAG "0"
#define NATIVE_CONVERSION_RULE_FLAG "1"

class Indentor;

class AbstractMetaType;
QT_BEGIN_NAMESPACE
class QDebug;
class QTextStream;
QT_END_NAMESPACE

class EnumTypeEntry;
class FlagsTypeEntry;

typedef QMap<int, QString> ArgumentMap;

class TemplateInstance;

struct ReferenceCount
{
    enum Action { // 0x01 - 0xff
        Invalid     = 0x00,
        Add         = 0x01,
        AddAll      = 0x02,
        Remove      = 0x04,
        Set         = 0x08,
        Ignore      = 0x10,

        ActionsMask = 0xff,

        Padding     = 0xffffffff
    };

    QString varName;
    Action action = Invalid;
};

struct ArgumentOwner
{
    enum Action {
        Invalid     = 0x00,
        Add         = 0x01,
        Remove      = 0x02
    };
    enum {
        InvalidIndex = -2,
        ThisIndex = -1,
        ReturnIndex = 0,
        FirstArgumentIndex = 1
    };

    Action action = Invalid;
    int index = InvalidIndex;
};

class CodeSnipFragment
{
public:
    CodeSnipFragment() = default;
    explicit CodeSnipFragment(const QString &code) : m_code(code) {}
    explicit CodeSnipFragment(TemplateInstance *instance) : m_instance(instance) {}

    QString code() const;

private:
    QString m_code;
    TemplateInstance *m_instance = nullptr;
};

class CodeSnipAbstract
{
public:
    QString code() const;

    void addCode(const QString &code) { codeList.append(CodeSnipFragment(code)); }
    void addCode(const QStringRef &code) { addCode(code.toString()); }

    void addTemplateInstance(TemplateInstance *ti)
    {
        codeList.append(CodeSnipFragment(ti));
    }

    QVector<CodeSnipFragment> codeList;
};

class CustomFunction : public CodeSnipAbstract
{
public:
    explicit CustomFunction(const QString &n = QString()) : name(n) {}

    QString name;
    QString paramName;
};

class TemplateEntry : public CodeSnipAbstract
{
public:
    explicit TemplateEntry(const QString &name) : m_name(name) {}

    QString name() const
    {
        return m_name;
    }

private:
    QString m_name;
};

class TemplateInstance
{
public:
    explicit TemplateInstance(const QString &name) : m_name(name) {}

    void addReplaceRule(const QString &name, const QString &value)
    {
        replaceRules[name] = value;
    }

    QString expandCode() const;

    QString name() const
    {
        return m_name;
    }

private:
    const QString m_name;
    QHash<QString, QString> replaceRules;
};


class CodeSnip : public CodeSnipAbstract
{
public:
    CodeSnip() = default;
    explicit CodeSnip(TypeSystem::Language lang) : language(lang) {}

    TypeSystem::Language language = TypeSystem::TargetLangCode;
    TypeSystem::CodeSnipPosition position = TypeSystem::CodeSnipPositionAny;
    ArgumentMap argumentMap;
};

struct ArgumentModification
{
    ArgumentModification() : removedDefaultExpression(false), removed(false),
        noNullPointers(false), array(false) {}
    explicit ArgumentModification(int idx) : index(idx), removedDefaultExpression(false), removed(false),
              noNullPointers(false), array(false) {}

    // Should the default expression be removed?


    // Reference count flags for this argument
    QVector<ReferenceCount> referenceCounts;

    // The text given for the new type of the argument
    QString modified_type;

    QString replace_value;

    // The code to be used to construct a return value when noNullPointers is true and
    // the returned value is null. If noNullPointers is true and this string is
    // empty, then the base class implementation will be used (or a default construction
    // if there is no implementation)
    QString nullPointerDefaultValue;

    // The text of the new default expression of the argument
    QString replacedDefaultExpression;

    // The new definition of ownership for a specific argument
    QHash<TypeSystem::Language, TypeSystem::Ownership> ownerships;

    // Different conversion rules
    CodeSnipList conversion_rules;

    //QObject parent(owner) of this argument
    ArgumentOwner owner;

    //New name
    QString renamed_to;

    // The index of this argument
    int index = -1;

    uint removedDefaultExpression : 1;
    uint removed : 1;
    uint noNullPointers : 1;
    uint resetAfterUse : 1;
    uint array : 1; // consider "int*" to be "int[]"
};

struct Modification
{
    enum Modifiers {
        Private =               0x0001,
        Protected =             0x0002,
        Public =                0x0003,
        Friendly =              0x0004,
        AccessModifierMask =    0x000f,

        Final =                 0x0010,
        NonFinal =              0x0020,
        FinalMask =             Final | NonFinal,

        Readable =              0x0100,
        Writable =              0x0200,

        CodeInjection =         0x1000,
        Rename =                0x2000,
        Deprecated =            0x4000,
        ReplaceExpression =     0x8000,
        VirtualSlot =          0x10000 | NonFinal
    };

    bool isAccessModifier() const
    {
        return modifiers & AccessModifierMask;
    }
    Modifiers accessModifier() const
    {
        return Modifiers(modifiers & AccessModifierMask);
    }
    bool isPrivate() const
    {
        return accessModifier() == Private;
    }
    bool isProtected() const
    {
        return accessModifier() == Protected;
    }
    bool isPublic() const
    {
        return accessModifier() == Public;
    }
    bool isFriendly() const
    {
        return accessModifier() == Friendly;
    }
    bool isFinal() const
    {
        return modifiers & Final;
    }
    bool isNonFinal() const
    {
        return modifiers & NonFinal;
    }
    bool isVirtualSlot() const
    {
        return (modifiers & VirtualSlot) == VirtualSlot;
    }
    QString accessModifierString() const;

    bool isDeprecated() const
    {
        return modifiers & Deprecated;
    }

    void setRenamedTo(const QString &name)
    {
        renamedToName = name;
    }
    QString renamedTo() const
    {
        return renamedToName;
    }
    bool isRenameModifier() const
    {
        return modifiers & Rename;
    }

    bool isRemoveModifier() const
    {
        return removal != TypeSystem::NoLanguage;
    }

    QString renamedToName;
    uint modifiers = 0;
    TypeSystem::Language removal = TypeSystem::NoLanguage;
};

struct FunctionModification: public Modification
{
    bool isCodeInjection() const
    {
        return modifiers & CodeInjection;
    }
    void setIsThread(bool flag)
    {
        m_thread = flag;
    }
    bool isThread() const
    {
        return m_thread;
    }
    bool allowThread() const
    {
        return m_allowThread;
    }
    void setAllowThread(bool allow)
    {
        m_allowThread = allow;
    }

    bool matches(const QString &functionSignature) const
    {
        return m_signature.isEmpty()
            ? m_signaturePattern.match(functionSignature).hasMatch()
            : m_signature == functionSignature;
    }

    bool setSignature(const QString &s, QString *errorMessage =  nullptr);
    QString signature() const { return m_signature.isEmpty() ? m_signaturePattern.pattern() : m_signature; }

    QString toString() const;

    QString association;
    CodeSnipList snips;

    QVector<ArgumentModification> argument_mods;

private:
    QString m_signature;
    QRegularExpression m_signaturePattern;
    bool m_thread = false;
    bool m_allowThread = false;
};

struct FieldModification: public Modification
{
    bool isReadable() const
    {
        return modifiers & Readable;
    }
    bool isWritable() const
    {
        return modifiers & Writable;
    }

    QString name;
};

/**
*   \internal
*   Struct used to store information about functions added by the typesystem.
*   This info will be used later to create a fake AbstractMetaFunction which
*   will be inserted into the right AbstractMetaClass.
*/
struct AddedFunction
{
    /// Function access types.
    enum Access {
        Protected = 0x1,
        Public =    0x2
    };

    /**
    *   \internal
    *   Internal struct used to store information about arguments and return type of the
    *   functions added by the type system. This information is later used to create
    *   AbstractMetaType and AbstractMetaArgument for the AbstractMetaFunctions.
    */
    struct TypeInfo {
        TypeInfo() = default;
        static TypeInfo fromSignature(const QString& signature);

        QString name;
        QString defaultValue;
        int indirections = 0;
        bool isConstant = false;
        bool isReference = false;
    };

    /// Creates a new AddedFunction with a signature and a return type.
    explicit AddedFunction(QString signature, const QString &returnType);
    AddedFunction() = default;

    /// Returns the function name.
    QString name() const
    {
        return m_name;
    }

    /// Set the function access type.
    void setAccess(Access access)
    {
        m_access = access;
    }

    /// Returns the function access type.
    Access access() const
    {
        return m_access;
    }

    /// Returns the function return type.
    TypeInfo returnType() const
    {
        return m_returnType;
    }

    /// Returns a list of argument type infos.
    QVector<TypeInfo> arguments() const
    {
        return m_arguments;
    }

    /// Returns true if this is a constant method.
    bool isConstant() const
    {
        return m_isConst;
    }

    /// Set this method static.
    void setStatic(bool value)
    {
        m_isStatic = value;
    }

    /// Returns true if this is a static method.
    bool isStatic() const
    {
        return m_isStatic;
    }

private:
    QString m_name;
    QVector<TypeInfo> m_arguments;
    TypeInfo m_returnType;
    Access m_access = Protected;
    bool m_isConst = false;
    bool m_isStatic = false;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AddedFunction::TypeInfo &ti);
QDebug operator<<(QDebug d, const AddedFunction &af);
#endif

class InterfaceTypeEntry;
class ObjectTypeEntry;

class DocModification
{
public:
    DocModification() = default;
    explicit DocModification(const QString& xpath, const QString& signature) :
        m_xpath(xpath), m_signature(signature) {}
    explicit DocModification(TypeSystem::DocModificationMode mode, const QString& signature) :
        m_signature(signature), m_mode(mode) {}

    void setCode(const QString& code) { m_code = code; }
    void setCode(const QStringRef& code) { m_code = code.toString(); }

    QString code() const
    {
        return m_code;
    }
    QString xpath() const
    {
        return m_xpath;
    }
    QString signature() const
    {
        return m_signature;
    }
    TypeSystem::DocModificationMode mode() const
    {
        return m_mode;
    }

    TypeSystem::Language  format() const { return m_format; }
    void setFormat(TypeSystem::Language f) { m_format = f; }

private:
    QString m_code;
    QString m_xpath;
    QString m_signature;
    TypeSystem::DocModificationMode m_mode = TypeSystem::DocModificationXPathReplace;
    TypeSystem::Language m_format = TypeSystem::NativeCode;
};

class CustomConversion;

class TypeEntry
{
    Q_DISABLE_COPY(TypeEntry)
    Q_GADGET
public:
    enum Type {
        PrimitiveType,
        VoidType,
        VarargsType,
        FlagsType,
        EnumType,
        EnumValue,
        TemplateArgumentType,
        ThreadType,
        BasicValueType,
        StringType,
        ContainerType,
        InterfaceType,
        ObjectType,
        NamespaceType,
        VariantType,
        JObjectWrapperType,
        CharType,
        ArrayType,
        TypeSystemType,
        CustomType,
        TargetLangType,
        FunctionType,
        SmartPointerType
    };
    Q_ENUM(Type)

    enum CodeGeneration {
        GenerateTargetLang      = 0x0001,
        GenerateCpp             = 0x0002,
        GenerateForSubclass     = 0x0004,

        GenerateNothing         = 0,
        GenerateAll             = 0xffff,
        GenerateCode            = GenerateTargetLang | GenerateCpp
    };
    Q_ENUM(CodeGeneration)

    explicit TypeEntry(const QString &name, Type t, const QVersionNumber &vr);

    virtual ~TypeEntry();

    Type type() const
    {
        return m_type;
    }
    bool isPrimitive() const
    {
        return m_type == PrimitiveType;
    }
    bool isEnum() const
    {
        return m_type == EnumType;
    }
    bool isFlags() const
    {
        return m_type == FlagsType;
    }
    bool isInterface() const
    {
        return m_type == InterfaceType;
    }
    bool isObject() const
    {
        return m_type == ObjectType;
    }
    bool isString() const
    {
        return m_type == StringType;
    }
    bool isChar() const
    {
        return m_type == CharType;
    }
    bool isNamespace() const
    {
        return m_type == NamespaceType;
    }
    bool isContainer() const
    {
        return m_type == ContainerType;
    }
    bool isSmartPointer() const
    {
        return m_type == SmartPointerType;
    }
    bool isVariant() const
    {
        return m_type == VariantType;
    }
    bool isJObjectWrapper() const
    {
        return m_type == JObjectWrapperType;
    }
    bool isArray() const
    {
        return m_type == ArrayType;
    }
    bool isTemplateArgument() const
    {
        return m_type == TemplateArgumentType;
    }
    bool isVoid() const
    {
        return m_type == VoidType;
    }
    bool isVarargs() const
    {
        return m_type == VarargsType;
    }
    bool isThread() const
    {
        return m_type == ThreadType;
    }
    bool isCustom() const
    {
        return m_type == CustomType;
    }
    bool isBasicValue() const
    {
        return m_type == BasicValueType;
    }
    bool isTypeSystem() const
    {
        return m_type == TypeSystemType;
    }
    bool isFunction() const
    {
        return m_type == FunctionType;
    }
    bool isEnumValue() const
    {
        return m_type == EnumValue;
    }

    virtual bool preferredConversion() const
    {
        return m_preferredConversion;
    }
    virtual void setPreferredConversion(bool b)
    {
        m_preferredConversion = b;
    }

    bool stream() const
    {
        return m_stream;
    }

    void setStream(bool b)
    {
        m_stream = b;
    }

    // The type's name in C++, fully qualified
    QString name() const
    {
        return m_name;
    }

    uint codeGeneration() const
    {
        return m_codeGeneration;
    }
    void setCodeGeneration(uint cg)
    {
        m_codeGeneration = cg;
    }

    // Returns true if code must be generated for this entry,
    // it will return false in case of types coming from typesystems
    // included for reference only.
    // NOTE: 'GenerateForSubclass' means 'generate="no"'
    //       on 'load-typesystem' tag
    inline bool generateCode() const
    {
        return m_codeGeneration != TypeEntry::GenerateForSubclass
               && m_codeGeneration != TypeEntry::GenerateNothing;
    }

    virtual QString qualifiedCppName() const
    {
        return m_name;
    }

    /**
     *   Its type's name in target language API
     *   The target language API name represents how this type is
     *   referred on low level code for the target language.
     *   Examples: for Java this would be a JNI name, for Python
     *   it should represent the CPython type name.
     *   /return string representing the target language API name
     *   for this type entry
     */
    virtual QString targetLangApiName() const
    {
        return m_name;
    }

    // The type's name in TargetLang
    virtual QString targetLangName() const
    {
        return m_name;
    }

    // The type to lookup when converting to TargetLang
    virtual QString lookupName() const
    {
        return targetLangName();
    }

    // The package
    virtual QString targetLangPackage() const
    {
        return QString();
    }

    virtual QString qualifiedTargetLangName() const
    {
        QString pkg = targetLangPackage();
        if (pkg.isEmpty())
            return targetLangName();
        return pkg + QLatin1Char('.') + targetLangName();
    }

    virtual InterfaceTypeEntry *designatedInterface() const
    {
        return 0;
    }

    void setCustomConstructor(const CustomFunction &func)
    {
        m_customConstructor = func;
    }
    CustomFunction customConstructor() const
    {
        return m_customConstructor;
    }

    void setCustomDestructor(const CustomFunction &func)
    {
        m_customDestructor = func;
    }
    CustomFunction customDestructor() const
    {
        return m_customDestructor;
    }

    virtual bool isValue() const
    {
        return false;
    }
    virtual bool isComplex() const
    {
        return false;
    }

    virtual bool isNativeIdBased() const
    {
        return false;
    }

    CodeSnipList codeSnips() const;
    void setCodeSnips(const CodeSnipList &codeSnips)
    {
        m_codeSnips = codeSnips;
    }
    void addCodeSnip(const CodeSnip &codeSnip)
    {
        m_codeSnips << codeSnip;
    }

    void setDocModification(const DocModificationList& docMods)
    {
        m_docModifications << docMods;
    }
    DocModificationList docModifications() const
    {
        return m_docModifications;
    }

    IncludeList extraIncludes() const
    {
        return m_extraIncludes;
    }
    void setExtraIncludes(const IncludeList &includes)
    {
        m_extraIncludes = includes;
    }
    void addExtraInclude(const Include &include)
    {
        if (!m_includesUsed.value(include.name(), false)) {
            m_extraIncludes << include;
            m_includesUsed[include.name()] = true;
        }
    }

    Include include() const
    {
        return m_include;
    }
    void setInclude(const Include &inc)
    {
        // This is a workaround for preventing double inclusion of the QSharedPointer implementation
        // header, which does not use header guards. In the previous parser this was not a problem
        // because the Q_QDOC define was set, and the implementation header was never included.
        if (inc.name() == QLatin1String("qsharedpointer_impl.h"))
            m_include = Include(inc.type(), QLatin1String("qsharedpointer.h"));
        else
            m_include = inc;
    }

    // Replace conversionRule arg to CodeSnip in future version
    /// Set the type convertion rule
    void setConversionRule(const QString& conversionRule)
    {
        m_conversionRule = conversionRule;
    }

    /// Returns the type convertion rule
    QString conversionRule() const
    {
        //skip conversions flag
        return m_conversionRule.mid(1);
    }

    /// Returns true if there are any conversiton rule for this type, false otherwise.
    bool hasConversionRule() const
    {
        return !m_conversionRule.isEmpty();
    }

    QVersionNumber version() const
    {
        return m_version;
    }

    /// TODO-CONVERTER: mark as deprecated
    bool hasNativeConversionRule() const
    {
        return m_conversionRule.startsWith(QLatin1String(NATIVE_CONVERSION_RULE_FLAG));
    }

    /// TODO-CONVERTER: mark as deprecated
    bool hasTargetConversionRule() const
    {
        return m_conversionRule.startsWith(QLatin1String(TARGET_CONVERSION_RULE_FLAG));
    }

    bool isCppPrimitive() const;

    bool hasCustomConversion() const;
    void setCustomConversion(CustomConversion* customConversion);
    CustomConversion* customConversion() const;
private:
    QString m_name;
    Type m_type;
    uint m_codeGeneration = GenerateAll;
    CustomFunction m_customConstructor;
    CustomFunction m_customDestructor;
    bool m_preferredConversion = true;
    CodeSnipList m_codeSnips;
    DocModificationList m_docModifications;
    IncludeList m_extraIncludes;
    Include m_include;
    QHash<QString, bool> m_includesUsed;
    QString m_conversionRule;
    bool m_stream = false;
    QVersionNumber m_version;
};

class TypeSystemTypeEntry : public TypeEntry
{
public:
    explicit TypeSystemTypeEntry(const QString &name, const QVersionNumber &vr);
};

class VoidTypeEntry : public TypeEntry
{
public:
    VoidTypeEntry();
};

class VarargsTypeEntry : public TypeEntry
{
public:
    VarargsTypeEntry();
};

class TemplateArgumentEntry : public TypeEntry
{
public:
    explicit TemplateArgumentEntry(const QString &name, const QVersionNumber &vr);

    int ordinal() const
    {
        return m_ordinal;
    }
    void setOrdinal(int o)
    {
        m_ordinal = o;
    }

private:
    int m_ordinal = 0;
};

class ArrayTypeEntry : public TypeEntry
{
public:
    explicit ArrayTypeEntry(const TypeEntry *nested_type, const QVersionNumber &vr);

    void setNestedTypeEntry(TypeEntry *nested)
    {
        m_nestedType = nested;
    }
    const TypeEntry *nestedTypeEntry() const
    {
        return m_nestedType;
    }

    QString targetLangName() const override;
    QString targetLangApiName() const override;

private:
    const TypeEntry *m_nestedType;
};


class PrimitiveTypeEntry : public TypeEntry
{
public:
    explicit PrimitiveTypeEntry(const QString &name, const QVersionNumber &vr);

    QString targetLangName() const override;
    void setTargetLangName(const QString &targetLangName)
    {
        m_targetLangName  = targetLangName;
    }

    QString targetLangApiName() const override;
    void setTargetLangApiName(const QString &targetLangApiName)
    {
        m_targetLangApiName = targetLangApiName;
    }

    QString defaultConstructor() const
    {
        return m_defaultConstructor;
    }
    void setDefaultConstructor(const QString& defaultConstructor)
    {
        m_defaultConstructor = defaultConstructor;
    }
    bool hasDefaultConstructor() const
    {
        return !m_defaultConstructor.isEmpty();
    }

    /**
     *   The PrimitiveTypeEntry pointed by this type entry if it
     *   represents a typedef).
     *   /return the type referenced by the typedef, or a null pointer
     *   if the current object is not an typedef
     */
    PrimitiveTypeEntry* referencedTypeEntry() const { return m_referencedTypeEntry; }

    /**
     *   Defines type referenced by this entry.
     *   /param referencedTypeEntry type referenced by this entry
     */
    void setReferencedTypeEntry(PrimitiveTypeEntry* referencedTypeEntry)
    {
        m_referencedTypeEntry = referencedTypeEntry;
    }

    /**
     *   Finds the most basic primitive type that the typedef represents,
     *   i.e. a type that is not an typedef'ed.
     *   /return the most basic non-typedef'ed primitive type represented
     *   by this typedef
     */
    PrimitiveTypeEntry* basicReferencedTypeEntry() const;

    bool preferredConversion() const override;
    void setPreferredConversion(bool b) override;

    bool preferredTargetLangType() const
    {
        return m_preferredTargetLangType;
    }
    void setPreferredTargetLangType(bool b)
    {
        m_preferredTargetLangType = b;
    }

    void setTargetLangPackage(const QString& package);
    QString targetLangPackage() const override;
private:
    QString m_targetLangName;
    QString m_targetLangApiName;
    QString m_defaultConstructor;
    uint m_preferredConversion : 1;
    uint m_preferredTargetLangType : 1;
    PrimitiveTypeEntry* m_referencedTypeEntry = nullptr;
};

class EnumTypeEntry : public TypeEntry
{
public:
    explicit EnumTypeEntry(const QString &nspace, const QString &enumName,
                           const QVersionNumber &vr);

    QString targetLangPackage() const override;
    void setTargetLangPackage(const QString &package);

    QString targetLangName() const override;
    QString targetLangQualifier() const;
    QString qualifiedTargetLangName() const override;

    QString targetLangApiName() const override;

    QString qualifier() const
    {
        return m_qualifier;
    }
    void setQualifier(const QString &q)
    {
        m_qualifier = q;
    }

    bool preferredConversion() const override;

    bool isBoundsChecked() const
    {
        return m_lowerBound.isEmpty() && m_upperBound.isEmpty();
    }

    QString upperBound() const
    {
        return m_upperBound;
    }
    void setUpperBound(const QString &bound)
    {
        m_upperBound = bound;
    }

    QString lowerBound() const
    {
        return m_lowerBound;
    }
    void setLowerBound(const QString &bound)
    {
        m_lowerBound = bound;
    }

    void setFlags(FlagsTypeEntry *flags)
    {
        m_flags = flags;
    }
    FlagsTypeEntry *flags() const
    {
        return m_flags;
    }

    bool isExtensible() const
    {
        return m_extensible;
    }
    void setExtensible(bool is)
    {
        m_extensible = is;
    }

    bool isEnumValueRejected(const QString &name) const
    {
        return m_rejectedEnums.contains(name);
    }
    void addEnumValueRejection(const QString &name)
    {
        m_rejectedEnums << name;
    }
    QStringList enumValueRejections() const
    {
        return m_rejectedEnums;
    }

    bool forceInteger() const
    {
        return m_forceInteger;
    }
    void setForceInteger(bool force)
    {
        m_forceInteger = force;
    }

private:
    QString m_packageName;
    QString m_qualifier;
    QString m_targetLangName;

    QString m_lowerBound;
    QString m_upperBound;

    QStringList m_rejectedEnums;

    FlagsTypeEntry *m_flags = nullptr;

    bool m_extensible = false;
    bool m_forceInteger = false;
};

// EnumValueTypeEntry is used for resolving integer type templates
// like array<EnumValue>.
class EnumValueTypeEntry : public TypeEntry
{
public:
    explicit EnumValueTypeEntry(const QString& name, const QString& value, const EnumTypeEntry* enclosingEnum, const QVersionNumber &vr);

    QString value() const { return m_value; }
    const EnumTypeEntry* enclosingEnum() const { return m_enclosingEnum; }
private:
    QString m_value;
    const EnumTypeEntry* m_enclosingEnum;
};

class FlagsTypeEntry : public TypeEntry
{
public:
    explicit FlagsTypeEntry(const QString &name, const QVersionNumber &vr);

    QString qualifiedTargetLangName() const override;
    QString targetLangName() const override;
    QString targetLangApiName() const override;
    bool preferredConversion() const override;

    QString originalName() const
    {
        return m_originalName;
    }
    void setOriginalName(const QString &s)
    {
        m_originalName = s;
    }

    QString flagsName() const
    {
        return m_targetLangName;
    }
    void setFlagsName(const QString &name)
    {
        m_targetLangName = name;
    }

    bool forceInteger() const
    {
        return m_enum->forceInteger();
    }

    EnumTypeEntry *originator() const
    {
        return m_enum;
    }
    void setOriginator(EnumTypeEntry *e)
    {
        m_enum = e;
    }

    QString targetLangPackage() const override;

private:
    QString m_originalName;
    QString m_targetLangName;
    EnumTypeEntry *m_enum = nullptr;
};


class ComplexTypeEntry : public TypeEntry
{
public:
    enum TypeFlag {
        ForceAbstract      = 0x1,
        DeleteInMainThread = 0x2,
        Deprecated         = 0x4
    };
    typedef QFlags<TypeFlag> TypeFlags;

    enum CopyableFlag {
        CopyableSet,
        NonCopyableSet,
        Unknown
    };

    explicit ComplexTypeEntry(const QString &name, Type t, const QVersionNumber &vr);

    bool isComplex() const override;

    void setLookupName(const QString &name)
    {
        m_lookupName = name;
    }

    QString lookupName() const override;

    QString targetLangApiName() const override;

    void setTypeFlags(TypeFlags flags)
    {
        m_typeFlags = flags;
    }

    TypeFlags typeFlags() const
    {
        return m_typeFlags;
    }

    FunctionModificationList functionModifications() const
    {
        return m_functionMods;
    }
    void setFunctionModifications(const FunctionModificationList &functionModifications)
    {
        m_functionMods = functionModifications;
    }
    void addFunctionModification(const FunctionModification &functionModification)
    {
        m_functionMods << functionModification;
    }
    FunctionModificationList functionModifications(const QString &signature) const;

    AddedFunctionList addedFunctions() const
    {
        return m_addedFunctions;
    }
    void setAddedFunctions(const AddedFunctionList &addedFunctions)
    {
        m_addedFunctions = addedFunctions;
    }
    void addNewFunction(const AddedFunction &addedFunction)
    {
        m_addedFunctions << addedFunction;
    }

    FieldModification fieldModification(const QString &name) const;
    void setFieldModifications(const FieldModificationList &mods)
    {
        m_fieldMods = mods;
    }
    FieldModificationList fieldModifications() const
    {
        return m_fieldMods;
    }

    QString targetLangPackage() const override;
    void setTargetLangPackage(const QString &package)
    {
        m_package = package;
    }

    bool isQObject() const
    {
        return m_qobject;
    }
    void setQObject(bool qobject)
    {
        m_qobject = qobject;
    }

    QString defaultSuperclass() const
    {
        return m_defaultSuperclass;
    }
    void setDefaultSuperclass(const QString &sc)
    {
        m_defaultSuperclass = sc;
    }

    QString qualifiedCppName() const override
    {
        return m_qualifiedCppName;
    }


    void setIsPolymorphicBase(bool on)
    {
        m_polymorphicBase = on;
    }
    bool isPolymorphicBase() const
    {
        return m_polymorphicBase;
    }

    void setPolymorphicIdValue(const QString &value)
    {
        m_polymorphicIdValue = value;
    }
    QString polymorphicIdValue() const
    {
        return m_polymorphicIdValue;
    }

    void setHeldType(const QString &value)
    {
        m_heldTypeValue = value;
    }
    QString heldTypeValue() const
    {
        return m_heldTypeValue;
    }

    QString targetType() const
    {
        return m_targetType;
    }
    void setTargetType(const QString &code)
    {
        m_targetType = code;
    }

    QString targetLangName() const override;
    void setTargetLangName(const QString &name)
    {
        m_targetLangName = name;
    }

    bool isGenericClass() const
    {
        return m_genericClass;
    }
    void setGenericClass(bool isGeneric)
    {
        m_genericClass = isGeneric;
    }

    CopyableFlag copyable() const
    {
        return m_copyableFlag;
    }
    void setCopyable(CopyableFlag flag)
    {
        m_copyableFlag = flag;
    }

    QString hashFunction() const
    {
        return m_hashFunction;
    }
    void setHashFunction(QString hashFunction)
    {
        m_hashFunction = hashFunction;
    }

    void setBaseContainerType(const ComplexTypeEntry *baseContainer)
    {
        m_baseContainerType = baseContainer;
    }

    const ComplexTypeEntry* baseContainerType() const
    {
        return m_baseContainerType;
    }

    QString defaultConstructor() const;
    void setDefaultConstructor(const QString& defaultConstructor);
    bool hasDefaultConstructor() const;

private:
    AddedFunctionList m_addedFunctions;
    FunctionModificationList m_functionMods;
    FieldModificationList m_fieldMods;
    QString m_package;
    QString m_defaultSuperclass;
    QString m_qualifiedCppName;
    QString m_targetLangName;

    uint m_qobject : 1;
    uint m_polymorphicBase : 1;
    uint m_genericClass : 1;

    QString m_polymorphicIdValue;
    QString m_heldTypeValue;
    QString m_lookupName;
    QString m_targetType;
    TypeFlags m_typeFlags;
    CopyableFlag m_copyableFlag = Unknown;
    QString m_hashFunction;

    const ComplexTypeEntry* m_baseContainerType = nullptr;
};

class ContainerTypeEntry : public ComplexTypeEntry
{
    Q_GADGET
public:
    enum Type {
        NoContainer,
        ListContainer,
        StringListContainer,
        LinkedListContainer,
        VectorContainer,
        StackContainer,
        QueueContainer,
        SetContainer,
        MapContainer,
        MultiMapContainer,
        HashContainer,
        MultiHashContainer,
        PairContainer,
    };
    Q_ENUM(Type)

    explicit ContainerTypeEntry(const QString &name, Type type, const QVersionNumber &vr);

    Type type() const
    {
        return m_type;
    }

    QString typeName() const;
    QString targetLangName() const override;
    QString targetLangPackage() const override;
    QString qualifiedCppName() const override;

    static Type containerTypeFromString(QString typeName)
    {
        static QHash<QString, Type> m_stringToContainerType;
        if (m_stringToContainerType.isEmpty()) {
            m_stringToContainerType.insert(QLatin1String("list"), ListContainer);
            m_stringToContainerType.insert(QLatin1String("string-list"), StringListContainer);
            m_stringToContainerType.insert(QLatin1String("linked-list"), LinkedListContainer);
            m_stringToContainerType.insert(QLatin1String("vector"), VectorContainer);
            m_stringToContainerType.insert(QLatin1String("stack"), StackContainer);
            m_stringToContainerType.insert(QLatin1String("queue"), QueueContainer);
            m_stringToContainerType.insert(QLatin1String("set"), SetContainer);
            m_stringToContainerType.insert(QLatin1String("map"), MapContainer);
            m_stringToContainerType.insert(QLatin1String("multi-map"), MultiMapContainer);
            m_stringToContainerType.insert(QLatin1String("hash"), HashContainer);
            m_stringToContainerType.insert(QLatin1String("multi-hash"), MultiHashContainer);
            m_stringToContainerType.insert(QLatin1String("pair"), PairContainer);
        }
        return m_stringToContainerType.value(typeName, NoContainer);
    }

private:
    Type m_type;
};

class SmartPointerTypeEntry : public ComplexTypeEntry
{
public:
    explicit SmartPointerTypeEntry(const QString &name,
                                   const QString &getterName,
                                   const QString &smartPointerType,
                                   const QString &refCountMethodName,
                                   const QVersionNumber &vr);

    QString getter() const
    {
        return m_getterName;
    }

    QString refCountMethodName() const
    {
        return m_refCountMethodName;
    }

private:
    QString m_getterName;
    QString m_smartPointerType;
    QString m_refCountMethodName;
};

class NamespaceTypeEntry : public ComplexTypeEntry
{
public:
    explicit NamespaceTypeEntry(const QString &name, const QVersionNumber &vr);
};


class ValueTypeEntry : public ComplexTypeEntry
{
public:
    explicit ValueTypeEntry(const QString &name, const QVersionNumber &vr);

    bool isValue() const override;

    bool isNativeIdBased() const override;

protected:
    explicit ValueTypeEntry(const QString &name, Type t, const QVersionNumber &vr);
};


class StringTypeEntry : public ValueTypeEntry
{
public:
    explicit StringTypeEntry(const QString &name, const QVersionNumber &vr);

    QString targetLangApiName() const override;
    QString targetLangName() const override;
    QString targetLangPackage() const override;

    bool isNativeIdBased() const override;
};

class CharTypeEntry : public ValueTypeEntry
{
public:
    explicit CharTypeEntry(const QString &name, const QVersionNumber &vr);

    QString targetLangApiName() const override;
    QString targetLangName() const override;
    QString targetLangPackage() const override;

    bool isNativeIdBased() const override;
};

class VariantTypeEntry: public ValueTypeEntry
{
public:
    explicit VariantTypeEntry(const QString &name, const QVersionNumber &vr);

    QString targetLangApiName() const override;
    QString targetLangName() const override;
    QString targetLangPackage() const override;

    bool isNativeIdBased() const override;
};


class InterfaceTypeEntry : public ComplexTypeEntry
{
public:
    explicit InterfaceTypeEntry(const QString &name, const QVersionNumber &vr);

    static QString interfaceName(const QString &name)
    {
        return name + QLatin1String("Interface");
    }

    ObjectTypeEntry *origin() const
    {
        return m_origin;
    }
    void setOrigin(ObjectTypeEntry *origin)
    {
        m_origin = origin;
    }

    bool isNativeIdBased() const override;
    QString qualifiedCppName() const override;

private:
    ObjectTypeEntry *m_origin;
};


class FunctionTypeEntry : public TypeEntry
{
public:
    explicit FunctionTypeEntry(const QString& name, const QString& signature,
                               const QVersionNumber &vr);
    void addSignature(const QString& signature)
    {
        m_signatures << signature;
    }

    QStringList signatures() const
    {
        return m_signatures;
    }

    bool hasSignature(const QString& signature) const
    {
        return m_signatures.contains(signature);
    }
private:
    QStringList m_signatures;
};

class ObjectTypeEntry : public ComplexTypeEntry
{
public:
    explicit ObjectTypeEntry(const QString &name, const QVersionNumber &vr);

    InterfaceTypeEntry *designatedInterface() const override;
    void setDesignatedInterface(InterfaceTypeEntry *entry)
    {
        m_interface = entry;
    }

    bool isNativeIdBased() const override;

private:
    InterfaceTypeEntry *m_interface = nullptr;
};

struct TypeRejection
{
    enum MatchType
    {
        ExcludeClass,                // Match className only
        Function,                    // Match className and function name
        Field,                       // Match className and field name
        Enum,                        // Match className and enum name
        ArgumentType,                // Match className and argument type
        ReturnType                   // Match className and return type
    };

    QRegularExpression className;
    QRegularExpression pattern;
    MatchType matchType;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeRejection &r);
#endif

QString fixCppTypeName(const QString &name);

class CustomConversion
{
public:
    CustomConversion(TypeEntry* ownerType);
    ~CustomConversion();

    const TypeEntry* ownerType() const;
    QString nativeToTargetConversion() const;
    void setNativeToTargetConversion(const QString& nativeToTargetConversion);

    class TargetToNativeConversion
    {
    public:
        TargetToNativeConversion(const QString& sourceTypeName,
                                 const QString& sourceTypeCheck,
                                 const QString& conversion = QString());
        ~TargetToNativeConversion();

        const TypeEntry* sourceType() const;
        void setSourceType(const TypeEntry* sourceType);
        bool isCustomType() const;
        QString sourceTypeName() const;
        QString sourceTypeCheck() const;
        QString conversion() const;
        void setConversion(const QString& conversion);
    private:
        struct TargetToNativeConversionPrivate;
        TargetToNativeConversionPrivate* m_d;
    };

    /**
     *  Returns true if the target to C++ custom conversions should
     *  replace the original existing ones, and false if the custom
     *  conversions should be added to the original.
     */
    bool replaceOriginalTargetToNativeConversions() const;
    void setReplaceOriginalTargetToNativeConversions(bool replaceOriginalTargetToNativeConversions);

    typedef QVector<TargetToNativeConversion*> TargetToNativeConversions;
    bool hasTargetToNativeConversions() const;
    TargetToNativeConversions& targetToNativeConversions();
    const TargetToNativeConversions& targetToNativeConversions() const;
    void addTargetToNativeConversion(const QString& sourceTypeName,
                                     const QString& sourceTypeCheck,
                                     const QString& conversion = QString());
private:
    struct CustomConversionPrivate;
    CustomConversionPrivate* m_d;
};

#endif // TYPESYSTEM_H
