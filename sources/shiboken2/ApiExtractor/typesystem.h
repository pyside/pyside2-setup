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

#ifndef TYPESYSTEM_H
#define TYPESYSTEM_H

#include "typesystem_enums.h"
#include "typesystem_typedefs.h"
#include "include.h"
#include "sourcelocation.h"

#include <QtCore/QHash>
#include <QtCore/qobjectdefs.h>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QVersionNumber>

//Used to identify the conversion rule to avoid break API
extern const char *TARGET_CONVERSION_RULE_FLAG;
extern const char *NATIVE_CONVERSION_RULE_FLAG;

class AbstractMetaType;
QT_BEGIN_NAMESPACE
class QDebug;
class QTextStream;
QT_END_NAMESPACE

class EnumTypeEntry;
class FlagsTypeEntry;

using ArgumentMap = QMap<int, QString>;

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

    void addCode(const QString &code);
    void addCode(QStringView code) { addCode(code.toString()); }

    void addTemplateInstance(TemplateInstance *ti)
    {
        codeList.append(CodeSnipFragment(ti));
    }

    QVector<CodeSnipFragment> codeList;

    static QString fixSpaces(QString code);
    static QString dedent(const QString &code);
    static void prependCode(QString *code, QString firstLine);
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
        noNullPointers(false), resetAfterUse(false), array(false) {}
    explicit ArgumentModification(int idx) : index(idx), removedDefaultExpression(false), removed(false),
              noNullPointers(false), resetAfterUse(false), array(false) {}

    // Should the default expression be removed?


    // Reference count flags for this argument
    QVector<ReferenceCount> referenceCounts;

    // The text given for the new type of the argument
    QString modified_type;

    QString replace_value;

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
    enum Modifiers : uint {
        InvalidModifier =       0x0000,
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
        ReplaceExpression =     0x8000
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

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

    QString renamedToName;
    uint modifiers = 0;
    TypeSystem::Language removal = TypeSystem::NoLanguage;
};

struct FunctionModification: public Modification
{
    using AllowThread = TypeSystem::AllowThread;

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

    AllowThread allowThread() const { return m_allowThread; }
    void setAllowThread(AllowThread allow) { m_allowThread = allow; }

    bool matches(const QString &functionSignature) const
    {
        return m_signature.isEmpty()
            ? m_signaturePattern.match(functionSignature).hasMatch()
            : m_signature == functionSignature;
    }

    bool setSignature(const QString &s, QString *errorMessage =  nullptr);
    QString signature() const { return m_signature.isEmpty() ? m_signaturePattern.pattern() : m_signature; }

    void setOriginalSignature(const QString &s) { m_originalSignature = s; }
    QString originalSignature() const { return m_originalSignature; }

    TypeSystem::ExceptionHandling exceptionHandling() const { return m_exceptionHandling; }
    void setExceptionHandling(TypeSystem::ExceptionHandling e) { m_exceptionHandling = e; }

    int overloadNumber() const { return m_overloadNumber; }
    void setOverloadNumber(int overloadNumber) { m_overloadNumber = overloadNumber; }

    QString toString() const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

    QString association;
    CodeSnipList snips;

    QVector<ArgumentModification> argument_mods;

private:
    QString m_signature;
    QString m_originalSignature;
    QRegularExpression m_signaturePattern;
    int m_overloadNumber = TypeSystem::OverloadNumberUnset;
    bool m_thread = false;
    AllowThread m_allowThread = AllowThread::Unspecified;
    TypeSystem::ExceptionHandling m_exceptionHandling = TypeSystem::ExceptionHandling::Unspecified;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const ReferenceCount &);
QDebug operator<<(QDebug d, const ArgumentOwner &a);
QDebug operator<<(QDebug d, const ArgumentModification &a);
QDebug operator<<(QDebug d, const FunctionModification &fm);
#endif

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
        InvalidAccess = 0,
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

    struct Argument
    {
        QString name;
        TypeInfo typeInfo;
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
    const QVector<Argument> &arguments() const
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

    FunctionModificationList modifications;

private:
    QString m_name;
    QVector<Argument> m_arguments;
    TypeInfo m_returnType;
    Access m_access = Protected;
    bool m_isConst = false;
    bool m_isStatic = false;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AddedFunction::TypeInfo &ti);
QDebug operator<<(QDebug d, const AddedFunction::Argument &a);
QDebug operator<<(QDebug d, const AddedFunction &af);
#endif

class ObjectTypeEntry;

class DocModification
{
public:
    DocModification() = default;
    explicit DocModification(const QString& xpath, const QString& signature) :
        m_xpath(xpath), m_signature(signature) {}
    explicit DocModification(TypeSystem::DocModificationMode mode, const QString& signature) :
        m_signature(signature), m_mode(mode) {}

    void setCode(const QString& code);
    void setCode(QStringView code) { setCode(code.toString()); }

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
class TypeSystemTypeEntry;

struct TypeSystemProperty
{
    bool isValid() const { return !name.isEmpty() && !read.isEmpty() && !type.isEmpty(); }

    QString type;
    QString name;
    QString read;
    QString write;
    QString reset;
    QString designable;
};

class TypeEntry
{
    Q_GADGET
public:
    TypeEntry &operator=(const TypeEntry &) = delete;
    TypeEntry &operator=(TypeEntry &&) = delete;
    TypeEntry(TypeEntry &&) = delete;

    enum Type {
        PrimitiveType,
        VoidType,
        VarargsType,
        FlagsType,
        EnumType,
        EnumValue,
        ConstantValueType,
        TemplateArgumentType,
        BasicValueType,
        ContainerType,
        ObjectType,
        NamespaceType,
        ArrayType,
        TypeSystemType,
        CustomType,
        FunctionType,
        SmartPointerType,
        TypedefType
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

    explicit TypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                       const TypeEntry *parent);

    virtual ~TypeEntry();

    Type type() const
    {
        return m_type;
    }

    const TypeEntry *parent() const { return m_parent; }
    void setParent(const TypeEntry *p) { m_parent = p; }
    bool isChildOf(const TypeEntry *p) const;
    const TypeSystemTypeEntry *typeSystemTypeEntry() const;
    // cf AbstractMetaClass::targetLangEnclosingClass()
    const TypeEntry *targetLangEnclosingEntry() const;

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
    bool isObject() const
    {
        return m_type == ObjectType;
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
    bool isCustom() const
    {
        return m_type == CustomType;
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

    bool stream() const
    {
        return m_stream;
    }

    void setStream(bool b)
    {
        m_stream = b;
    }

    // The type's name in C++, fully qualified
    QString name() const { return m_name; }
    // C++ excluding inline namespaces
    QString shortName() const;
    // Name as specified in XML
    QString entryName() const { return m_entryName; }

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

    int revision() const { return m_revision; }
    void setRevision(int r); // see typedatabase.cpp
    int sbkIndex() const;
    void setSbkIndex(int i) { m_sbkIndex = i; }

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
    QString targetLangName() const; // "Foo.Bar"
    void setTargetLangName(const QString &n) { m_cachedTargetLangName = n; }
    QString targetLangEntryName() const; // "Bar"

    // The package
    QString targetLangPackage() const { return m_targetLangPackage; }
    void setTargetLangPackage(const QString &p) { m_targetLangPackage = p; }

    QString qualifiedTargetLangName() const;

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
    void addExtraInclude(const Include &newInclude);

    Include include() const
    {
        return m_include;
    }
    void setInclude(const Include &inc);

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

    virtual TypeEntry *clone() const;

    void useAsTypedef(const TypeEntry *source);

    SourceLocation sourceLocation() const;
    void setSourceLocation(const SourceLocation &sourceLocation);

#ifndef QT_NO_DEBUG_STREAM
    virtual void formatDebug(QDebug &d) const;
#endif

protected:
    TypeEntry(const TypeEntry &);

    virtual QString buildTargetLangName() const;

private:
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
    uint m_codeGeneration = GenerateAll;
    int m_revision = 0;
    int m_sbkIndex = 0;
    Type m_type;
    bool m_stream = false;
};

class TypeSystemTypeEntry : public TypeEntry
{
public:
    explicit TypeSystemTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                 const TypeEntry *parent);

    TypeEntry *clone() const override;

protected:
    TypeSystemTypeEntry(const TypeSystemTypeEntry &);
};

class VoidTypeEntry : public TypeEntry
{
public:
    VoidTypeEntry();

    TypeEntry *clone() const override;

protected:
    VoidTypeEntry(const VoidTypeEntry &);
};

class VarargsTypeEntry : public TypeEntry
{
public:
    VarargsTypeEntry();

    TypeEntry *clone() const override;

protected:
    VarargsTypeEntry(const VarargsTypeEntry &);
};

class TemplateArgumentEntry : public TypeEntry
{
public:
    explicit TemplateArgumentEntry(const QString &entryName, const QVersionNumber &vr,
                                   const TypeEntry *parent);

    int ordinal() const
    {
        return m_ordinal;
    }
    void setOrdinal(int o)
    {
        m_ordinal = o;
    }

    TypeEntry *clone() const override;

protected:
    TemplateArgumentEntry(const TemplateArgumentEntry &);

private:
    int m_ordinal = 0;
};

class ArrayTypeEntry : public TypeEntry
{
public:
    explicit ArrayTypeEntry(const TypeEntry *nested_type, const QVersionNumber &vr,
                            const TypeEntry *parent);

    void setNestedTypeEntry(TypeEntry *nested)
    {
        m_nestedType = nested;
    }
    const TypeEntry *nestedTypeEntry() const
    {
        return m_nestedType;
    }

    QString targetLangApiName() const override;

    TypeEntry *clone() const override;

protected:
    ArrayTypeEntry(const ArrayTypeEntry &);

    QString buildTargetLangName() const override;

private:
    const TypeEntry *m_nestedType;
};


class PrimitiveTypeEntry : public TypeEntry
{
public:
    explicit PrimitiveTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                const TypeEntry *parent);

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

    bool preferredTargetLangType() const
    {
        return m_preferredTargetLangType;
    }
    void setPreferredTargetLangType(bool b)
    {
        m_preferredTargetLangType = b;
    }

    TypeEntry *clone() const override;

protected:
    PrimitiveTypeEntry(const PrimitiveTypeEntry &);

private:
    QString m_targetLangApiName;
    QString m_defaultConstructor;
    uint m_preferredTargetLangType : 1;
    PrimitiveTypeEntry* m_referencedTypeEntry = nullptr;
};

class EnumValueTypeEntry;

class EnumTypeEntry : public TypeEntry
{
public:
    explicit EnumTypeEntry(const QString &entryName,
                           const QVersionNumber &vr,
                           const TypeEntry *parent);

    QString targetLangQualifier() const;

    QString targetLangApiName() const override;

    QString qualifier() const;

    const EnumValueTypeEntry *nullValue() const { return m_nullValue; }
    void setNullValue(const EnumValueTypeEntry *n) { m_nullValue = n; }

    void setFlags(FlagsTypeEntry *flags)
    {
        m_flags = flags;
    }
    FlagsTypeEntry *flags() const
    {
        return m_flags;
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

    TypeEntry *clone() const override;
#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    EnumTypeEntry(const EnumTypeEntry &);

private:
    const EnumValueTypeEntry *m_nullValue = nullptr;

    QStringList m_rejectedEnums;

    FlagsTypeEntry *m_flags = nullptr;
};

// EnumValueTypeEntry is used for resolving integer type templates
// like array<EnumValue>. Note: Dummy entries for integer values will
// be created for non-type template parameters, where m_enclosingEnum==nullptr.
class EnumValueTypeEntry : public TypeEntry
{
public:
    explicit EnumValueTypeEntry(const QString& name, const QString& value,
                                const EnumTypeEntry* enclosingEnum,
                                bool isScopedEnum, const QVersionNumber &vr);

    QString value() const { return m_value; }
    const EnumTypeEntry* enclosingEnum() const { return m_enclosingEnum; }

    TypeEntry *clone() const override;

protected:
    EnumValueTypeEntry(const EnumValueTypeEntry &);

private:
    QString m_value;
    const EnumTypeEntry* m_enclosingEnum;
};

class FlagsTypeEntry : public TypeEntry
{
public:
    explicit FlagsTypeEntry(const QString &entryName, const QVersionNumber &vr,
                            const TypeEntry *parent);

    QString targetLangApiName() const override;

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
        return m_flagsName;
    }
    void setFlagsName(const QString &name)
    {
        m_flagsName = name;
    }

    EnumTypeEntry *originator() const
    {
        return m_enum;
    }
    void setOriginator(EnumTypeEntry *e)
    {
        m_enum = e;
    }

    TypeEntry *clone() const override;

protected:
    FlagsTypeEntry(const FlagsTypeEntry &);

    QString buildTargetLangName() const override;

private:
    QString m_originalName;
    QString m_flagsName;
    EnumTypeEntry *m_enum = nullptr;
};

// For primitive values, typically to provide a dummy type for
// example the '2' in non-type template 'Array<2>'.
class ConstantValueTypeEntry : public TypeEntry
{
public:
    explicit  ConstantValueTypeEntry(const QString& name,
                                     const TypeEntry *parent);

    TypeEntry *clone() const override;

protected:
    ConstantValueTypeEntry(const ConstantValueTypeEntry &);
};

class ComplexTypeEntry : public TypeEntry
{
public:
    enum TypeFlag {
        Deprecated         = 0x4
    };
    Q_DECLARE_FLAGS(TypeFlags, TypeFlag)

    enum CopyableFlag {
        CopyableSet,
        NonCopyableSet,
        Unknown
    };

    explicit ComplexTypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                              const TypeEntry *parent);

    bool isComplex() const override;

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
    void addNewFunction(const AddedFunctionPtr &addedFunction)
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

    const QList<TypeSystemProperty> &properties() const { return m_properties; }
    void addProperty(const TypeSystemProperty &p) { m_properties.append(p); }

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

    QString targetType() const
    {
        return m_targetType;
    }
    void setTargetType(const QString &code)
    {
        m_targetType = code;
    }

    bool isGenericClass() const
    {
        return m_genericClass;
    }
    void setGenericClass(bool isGeneric)
    {
        m_genericClass = isGeneric;
    }

    bool deleteInMainThread() const { return m_deleteInMainThread; }
    void setDeleteInMainThread(bool d) { m_deleteInMainThread = d; }

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
    void setHashFunction(const QString &hashFunction)
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

    TypeSystem::ExceptionHandling exceptionHandling() const { return m_exceptionHandling; }
    void setExceptionHandling(TypeSystem::ExceptionHandling e) { m_exceptionHandling = e; }

    TypeSystem::AllowThread allowThread() const { return m_allowThread; }
    void setAllowThread(TypeSystem::AllowThread allowThread) { m_allowThread = allowThread; }

    QString defaultConstructor() const;
    void setDefaultConstructor(const QString& defaultConstructor);
    bool hasDefaultConstructor() const;

    TypeEntry *clone() const override;

    void useAsTypedef(const ComplexTypeEntry *source);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    ComplexTypeEntry(const ComplexTypeEntry &);

private:
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
    TypeFlags m_typeFlags;
    CopyableFlag m_copyableFlag = Unknown;
    QString m_hashFunction;

    const ComplexTypeEntry* m_baseContainerType = nullptr;
    // For class functions
    TypeSystem::ExceptionHandling m_exceptionHandling = TypeSystem::ExceptionHandling::Unspecified;
    TypeSystem::AllowThread m_allowThread = TypeSystem::AllowThread::Unspecified;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ComplexTypeEntry::TypeFlags)

class TypedefEntry : public ComplexTypeEntry
{
public:
    explicit TypedefEntry(const QString &entryName,
                          const QString &sourceType,
                          const QVersionNumber &vr,
                          const TypeEntry *parent);

    QString sourceType() const { return m_sourceType; }
    void setSourceType(const QString &s) { m_sourceType =s; }

    TypeEntry *clone() const override;

    ComplexTypeEntry *source() const { return m_source; }
    void setSource(ComplexTypeEntry *source) { m_source = source; }

    ComplexTypeEntry *target() const { return m_target; }
    void setTarget(ComplexTypeEntry *target) { m_target = target; }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    TypedefEntry(const TypedefEntry &);

private:
    QString m_sourceType;
    ComplexTypeEntry *m_source = nullptr;
    ComplexTypeEntry *m_target = nullptr;
};

class ContainerTypeEntry : public ComplexTypeEntry
{
    Q_GADGET
public:
    enum ContainerKind {
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
    Q_ENUM(ContainerKind)

    explicit ContainerTypeEntry(const QString &entryName, ContainerKind containerKind,
                                const QVersionNumber &vr, const TypeEntry *parent);

    ContainerKind containerKind() const
    {
        return m_containerKind;
    }

    QString typeName() const;
    QString qualifiedCppName() const override;

    TypeEntry *clone() const override;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    ContainerTypeEntry(const ContainerTypeEntry &);

private:
    ContainerKind m_containerKind;
};

class SmartPointerTypeEntry : public ComplexTypeEntry
{
public:
    using Instantiations = QVector<const TypeEntry *>;

    explicit SmartPointerTypeEntry(const QString &entryName,
                                   const QString &getterName,
                                   const QString &smartPointerType,
                                   const QString &refCountMethodName,
                                   const QVersionNumber &vr,
                                   const TypeEntry *parent);

    QString getter() const
    {
        return m_getterName;
    }

    QString refCountMethodName() const
    {
        return m_refCountMethodName;
    }

    TypeEntry *clone() const override;

    Instantiations instantiations() const { return m_instantiations; }
    void setInstantiations(const Instantiations &i) { m_instantiations = i; }
    bool matchesInstantiation(const TypeEntry *e) const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif
protected:
    SmartPointerTypeEntry(const SmartPointerTypeEntry &);

private:
    QString m_getterName;
    QString m_smartPointerType;
    QString m_refCountMethodName;
    Instantiations m_instantiations;
};

class NamespaceTypeEntry : public ComplexTypeEntry
{
public:
    explicit NamespaceTypeEntry(const QString &entryName, const QVersionNumber &vr,
                                const TypeEntry *parent);

    TypeEntry *clone() const override;

    const NamespaceTypeEntry *extends() const { return m_extends; }
    void setExtends(const NamespaceTypeEntry *e)  { m_extends = e; }

    const QRegularExpression &filePattern() const { return m_filePattern; } // restrict files
    void setFilePattern(const QRegularExpression &r);

    bool hasPattern() const { return m_hasPattern; }

    bool matchesFile(const QString &needle) const;

    bool isVisible() const;
    void setVisibility(TypeSystem::Visibility v) { m_visibility = v; }

    // C++ 11 inline namespace, from code model
    bool isInlineNamespace() const { return m_inlineNamespace; }
    void setInlineNamespace(bool i) { m_inlineNamespace = i; }

    static bool isVisibleScope(const TypeEntry *e);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

    bool generateUsing() const { return m_generateUsing; }
    void setGenerateUsing(bool generateUsing) { m_generateUsing = generateUsing; }

protected:
    NamespaceTypeEntry(const NamespaceTypeEntry &);

private:
    QRegularExpression m_filePattern;
    const NamespaceTypeEntry *m_extends = nullptr;
    TypeSystem::Visibility m_visibility = TypeSystem::Visibility::Auto;
    bool m_hasPattern = false;
    bool m_inlineNamespace = false;
    bool m_generateUsing = true; // Whether to generate "using namespace" into wrapper
};

class ValueTypeEntry : public ComplexTypeEntry
{
public:
    explicit ValueTypeEntry(const QString &entryName, const QVersionNumber &vr,
                            const TypeEntry *parent);

    bool isValue() const override;

    TypeEntry *clone() const override;

protected:
    explicit ValueTypeEntry(const QString &entryName, Type t, const QVersionNumber &vr,
                            const TypeEntry *parent);
    ValueTypeEntry(const ValueTypeEntry &);
};

class FunctionTypeEntry : public TypeEntry
{
public:
    explicit FunctionTypeEntry(const QString& name, const QString& signature,
                               const QVersionNumber &vr,
                               const TypeEntry *parent);
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

    TypeEntry *clone() const override;

protected:
    FunctionTypeEntry(const FunctionTypeEntry &);

private:
    QStringList m_signatures;
};

class ObjectTypeEntry : public ComplexTypeEntry
{
public:
    explicit ObjectTypeEntry(const QString &entryName, const QVersionNumber &vr,
                             const TypeEntry *parent);

    TypeEntry *clone() const override;

protected:
    ObjectTypeEntry(const ObjectTypeEntry &);
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
        ReturnType,                  // Match className and return type
        Invalid
    };

    QRegularExpression className;
    QRegularExpression pattern;
    MatchType matchType = Invalid;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeRejection &r);
#endif

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

    using TargetToNativeConversions = QVector<TargetToNativeConversion *>;
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
