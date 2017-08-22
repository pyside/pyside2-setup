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
    ReferenceCount()  {}
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

    Action action;
    QString varName;
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
    ArgumentOwner() : action(ArgumentOwner::Invalid), index(ArgumentOwner::InvalidIndex) {}

    Action action;
    int index;
};

class CodeSnipFragment
{
private:
    QString m_code;
    TemplateInstance *m_instance;

public:
    CodeSnipFragment() : m_instance(0) {}
    CodeSnipFragment(const QString &code)
        : m_code(code),
          m_instance(0) {}

    CodeSnipFragment(TemplateInstance *instance)
        : m_instance(instance) {}

    QString code() const;
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
    CustomFunction(const QString &n = QString()) : name(n) { }

    QString name;
    QString paramName;
};

class TemplateEntry : public CodeSnipAbstract
{
public:
    TemplateEntry(const QString &name, double vr)
            : m_name(name), m_version(vr)
    {
    };

    QString name() const
    {
        return m_name;
    };

    double version() const
    {
        return m_version;
    }

private:
    QString m_name;
    double m_version;
};

class TemplateInstance
{
public:
    TemplateInstance(const QString &name, double vr)
            : m_name(name), m_version(vr) {}

    void addReplaceRule(const QString &name, const QString &value)
    {
        replaceRules[name] = value;
    }

    QString expandCode() const;

    QString name() const
    {
        return m_name;
    }

    double version() const
    {
        return m_version;
    }

private:
    const QString m_name;
    double m_version;
    QHash<QString, QString> replaceRules;
};


class CodeSnip : public CodeSnipAbstract
{
public:
    CodeSnip() : language(TypeSystem::TargetLangCode), version(0) {}
    CodeSnip(double vr) : language(TypeSystem::TargetLangCode), version(vr) { }
    CodeSnip(double vr, TypeSystem::Language lang) : language(lang), version(vr) { }

    TypeSystem::Language language;
    TypeSystem::CodeSnipPosition position;
    ArgumentMap argumentMap;
    double version;
};

struct ArgumentModification
{
    ArgumentModification() : removedDefaultExpression(false), removed(false),
        noNullPointers(false), array(false), index(-1), version(0) {}
    ArgumentModification(int idx, double vr)
            : removedDefaultExpression(false), removed(false),
              noNullPointers(false), array(false), index(idx), version(vr) {}

    // Should the default expression be removed?
    uint removedDefaultExpression : 1;
    uint removed : 1;
    uint noNullPointers : 1;
    uint resetAfterUse : 1;
    uint array : 1; // consider "int*" to be "int[]"

    // The index of this argument
    int index;

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

    //Api version
    double version;

    //New name
    QString renamed_to;
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

    Modification() : modifiers(0), removal(TypeSystem::NoLanguage) { }

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

    uint modifiers;
    QString renamedToName;
    TypeSystem::Language removal;
};

struct FunctionModification: public Modification
{
    FunctionModification() : m_thread(false), m_allowThread(false), m_version(0) {}
    FunctionModification(double vr) : m_thread(false), m_allowThread(false), m_version(vr) {}

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
    double version() const
    {
        return m_version;
    }

    bool operator!=(const FunctionModification& other) const;
    bool operator==(const FunctionModification& other) const;


    QString toString() const;

    QString signature;
    QString association;
    CodeSnipList snips;

    QVector<ArgumentModification> argument_mods;

private:
    bool m_thread;
    bool m_allowThread;
    double m_version;


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
        TypeInfo() : isConstant(false), indirections(0), isReference(false) {}
        static TypeInfo fromSignature(const QString& signature);

        QString name;
        bool isConstant;
        int indirections;
        bool isReference;
        QString defaultValue;
    };

    /// Creates a new AddedFunction with a signature and a return type.
    AddedFunction(QString signature, QString returnType, double vr);
    AddedFunction() : m_access(Protected), m_isConst(false), m_isStatic(false), m_version(0) {}

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

    double version() const
    {
        return m_version;
    }
private:
    QString m_name;
    Access m_access;
    QVector<TypeInfo> m_arguments;
    TypeInfo m_returnType;
    bool m_isConst;
    bool m_isStatic;
    double m_version;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AddedFunction::TypeInfo &ti);
QDebug operator<<(QDebug d, const AddedFunction &af);
#endif

struct ExpensePolicy
{
    ExpensePolicy() : limit(-1) {}
    int limit;
    QString cost;
    bool isValid() const
    {
        return limit >= 0;
    }
};

class InterfaceTypeEntry;
class ObjectTypeEntry;

class DocModification
{
public:
    DocModification() : format(TypeSystem::NativeCode), m_mode(TypeSystem::DocModificationXPathReplace), m_version(0) {}
    DocModification(const QString& xpath, const QString& signature, double vr)
            : format(TypeSystem::NativeCode), m_mode(TypeSystem::DocModificationXPathReplace),
              m_xpath(xpath), m_signature(signature), m_version(vr) {}
    DocModification(TypeSystem::DocModificationMode mode, const QString& signature, double vr)
            : m_mode(mode), m_signature(signature), m_version(vr) {}

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
    double version() const
    {
        return m_version;
    }

    TypeSystem::Language format;

private:
    TypeSystem::DocModificationMode m_mode;
    QString m_code;
    QString m_xpath;
    QString m_signature;
    double m_version;
};

class CustomConversion;

class TypeEntry
{
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

    TypeEntry(const QString &name, Type t, double vr)
            : m_name(name),
              m_type(t),
              m_codeGeneration(GenerateAll),
              m_preferredConversion(true),
              m_stream(false),
              m_version(vr)
    {
    };

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

    double version() const
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
    uint m_codeGeneration;
    CustomFunction m_customConstructor;
    CustomFunction m_customDestructor;
    bool m_preferredConversion;
    CodeSnipList m_codeSnips;
    DocModificationList m_docModifications;
    IncludeList m_extraIncludes;
    Include m_include;
    QHash<QString, bool> m_includesUsed;
    QString m_conversionRule;
    bool m_stream;
    double m_version;
};

class TypeSystemTypeEntry : public TypeEntry
{
public:
    TypeSystemTypeEntry(const QString &name, double vr)
            : TypeEntry(name, TypeSystemType, vr)
    {
    };
};

class VoidTypeEntry : public TypeEntry
{
public:
    VoidTypeEntry() : TypeEntry(QLatin1String("void"), VoidType, 0) { }
};

class VarargsTypeEntry : public TypeEntry
{
public:
    VarargsTypeEntry() : TypeEntry(QLatin1String("..."), VarargsType, 0) { }
};

class TemplateArgumentEntry : public TypeEntry
{
public:
    TemplateArgumentEntry(const QString &name, double vr)
            : TypeEntry(name, TemplateArgumentType, vr), m_ordinal(0)
    {
    }

    int ordinal() const
    {
        return m_ordinal;
    }
    void setOrdinal(int o)
    {
        m_ordinal = o;
    }

private:
    int m_ordinal;
};

class ArrayTypeEntry : public TypeEntry
{
public:
    ArrayTypeEntry(const TypeEntry *nested_type, double vr)
            : TypeEntry(QLatin1String("Array"), ArrayType, vr), m_nestedType(nested_type)
    {
        Q_ASSERT(m_nestedType);
    }

    void setNestedTypeEntry(TypeEntry *nested)
    {
        m_nestedType = nested;
    }
    const TypeEntry *nestedTypeEntry() const
    {
        return m_nestedType;
    }

    QString targetLangName() const override
    {
        return m_nestedType->targetLangName() + QLatin1String("[]");
    }
    QString targetLangApiName() const override
    {
        if (m_nestedType->isPrimitive())
            return m_nestedType->targetLangApiName() + QLatin1String("Array");
        else
            return QLatin1String("jobjectArray");
    }

private:
    const TypeEntry *m_nestedType;
};


class PrimitiveTypeEntry : public TypeEntry
{
public:
    PrimitiveTypeEntry(const QString &name, double vr)
            : TypeEntry(name, PrimitiveType, vr),
              m_preferredConversion(true),
              m_preferredTargetLangType(true),
              m_referencedTypeEntry(0)
    {
    }

    QString targetLangName() const override
    {
        return m_targetLangName;
    }
    void setTargetLangName(const QString &targetLangName)
    {
        m_targetLangName  = targetLangName;
    }

    QString targetLangApiName() const override
    {
        return m_targetLangApiName;
    }
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

    bool preferredConversion() const override
    {
        return m_preferredConversion;
    }
    void setPreferredConversion(bool b) override
    {
        m_preferredConversion = b;
    }

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
    PrimitiveTypeEntry* m_referencedTypeEntry;
};

struct EnumValueRedirection
{
    EnumValueRedirection() {}
    EnumValueRedirection(const QString &rej, const QString &us)
            : rejected(rej),
            used(us)
    {
    }
    QString rejected;
    QString used;
};

class EnumTypeEntry : public TypeEntry
{
public:
    EnumTypeEntry(const QString &nspace, const QString &enumName, double vr)
            : TypeEntry(nspace.isEmpty() ? enumName : nspace + QLatin1String("::") + enumName,
                        EnumType, vr),
            m_qualifier(nspace),
            m_targetLangName(enumName),
            m_flags(0),
            m_extensible(false),
            m_forceInteger(false),
            m_anonymous(false)
    {
    }

    QString targetLangPackage() const
    {
        return m_packageName;
    }
    void setTargetLangPackage(const QString &package)
    {
        m_packageName = package;
    }

    QString targetLangName() const
    {
        return m_targetLangName;
    }
    QString targetLangQualifier() const;
    QString qualifiedTargetLangName() const override
    {
        QString qualifiedName;
        QString pkg = targetLangPackage();
        QString qualifier = targetLangQualifier();

        if (!pkg.isEmpty())
            qualifiedName += pkg + QLatin1Char('.');
        if (!qualifier.isEmpty())
            qualifiedName += qualifier + QLatin1Char('.');
        qualifiedName += targetLangName();

        return qualifiedName;
    }

    QString targetLangApiName() const override;

    QString qualifier() const
    {
        return m_qualifier;
    }
    void setQualifier(const QString &q)
    {
        m_qualifier = q;
    }

    bool preferredConversion() const override
    {
        return false;
    }

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

    bool isEnumValueRejected(const QString &name)
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

    void addEnumValueRedirection(const QString &rejected, const QString &usedValue);
    QString enumValueRedirection(const QString &value) const;

    bool forceInteger() const
    {
        return m_forceInteger;
    }
    void setForceInteger(bool force)
    {
        m_forceInteger = force;
    }

    bool isAnonymous() const
    {
        return m_anonymous;
    }
    void setAnonymous(bool anonymous)
    {
        m_anonymous = anonymous;
    }

private:
    QString m_packageName;
    QString m_qualifier;
    QString m_targetLangName;

    QString m_lowerBound;
    QString m_upperBound;

    QStringList m_rejectedEnums;
    QVector<EnumValueRedirection> m_enumRedirections;

    FlagsTypeEntry *m_flags;

    bool m_extensible;
    bool m_forceInteger;
    bool m_anonymous;
};

class EnumValueTypeEntry : public TypeEntry
{
public:
    EnumValueTypeEntry(const QString& name, const QString& value, const EnumTypeEntry* enclosingEnum, double vr)
        : TypeEntry(name, TypeEntry::EnumValue, vr), m_value(value), m_enclosingEnum(enclosingEnum)
    {
    }

    QString value() const { return m_value; }
    const EnumTypeEntry* enclosingEnum() const { return m_enclosingEnum; }
private:
    QString m_value;
    const EnumTypeEntry* m_enclosingEnum;
};

class FlagsTypeEntry : public TypeEntry
{
public:
    FlagsTypeEntry(const QString &name, double vr) : TypeEntry(name, FlagsType, vr), m_enum(0)
    {
    }

    QString qualifiedTargetLangName() const override;
    QString targetLangName() const override
    {
        return m_targetLangName;
    }
    QString targetLangApiName() const override;
    bool preferredConversion() const override
    {
        return false;
    }

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

    QString targetLangPackage() const override
    {
        return m_enum->targetLangPackage();
    }

private:
    QString m_originalName;
    QString m_targetLangName;
    EnumTypeEntry *m_enum;
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

    ComplexTypeEntry(const QString &name, Type t, double vr)
            : TypeEntry(QString(name).replace(QLatin1String(".*::"), QString()), t, vr),
            m_qualifiedCppName(name),
            m_qobject(false),
            m_polymorphicBase(false),
            m_genericClass(false),
            m_typeFlags(0),
            m_copyableFlag(Unknown),
            m_baseContainerType(0)
    {
    }

    bool isComplex() const
    {
        return true;
    }

    ComplexTypeEntry *copy() const
    {
        ComplexTypeEntry *centry = new ComplexTypeEntry(name(), type(), version());
        centry->setInclude(include());
        centry->setExtraIncludes(extraIncludes());
        centry->setAddedFunctions(addedFunctions());
        centry->setFunctionModifications(functionModifications());
        centry->setFieldModifications(fieldModifications());
        centry->setQObject(isQObject());
        centry->setDefaultSuperclass(defaultSuperclass());
        centry->setCodeSnips(codeSnips());
        centry->setTargetLangPackage(targetLangPackage());
        centry->setBaseContainerType(baseContainerType());
        centry->setDefaultConstructor(defaultConstructor());

        return centry;
    }

    void setLookupName(const QString &name)
    {
        m_lookupName = name;
    }

    QString lookupName() const override
    {
        return m_lookupName.isEmpty() ? targetLangName() : m_lookupName;
    }

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

    QString targetLangPackage() const
    {
        return m_package;
    }
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


    void setExpensePolicy(const ExpensePolicy &policy)
    {
        m_expensePolicy = policy;
    }
    const ExpensePolicy &expensePolicy() const
    {
        return m_expensePolicy;
    }

    QString targetType() const
    {
        return m_targetType;
    }
    void setTargetType(const QString &code)
    {
        m_targetType = code;
    }

    QString targetLangName() const override
    {
        return m_targetLangName.isEmpty()
               ? TypeEntry::targetLangName()
               : m_targetLangName;
    }
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
    ExpensePolicy m_expensePolicy;
    TypeFlags m_typeFlags;
    CopyableFlag m_copyableFlag;
    QString m_hashFunction;

    const ComplexTypeEntry* m_baseContainerType;
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

    ContainerTypeEntry(const QString &name, Type type, double vr)
        : ComplexTypeEntry(name, ContainerType, vr), m_type(type)
    {
        setCodeGeneration(GenerateForSubclass);
    }

    Type type() const
    {
        return m_type;
    }

    QString typeName() const;
    QString targetLangName() const;
    QString targetLangPackage() const;
    QString qualifiedCppName() const;

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
    SmartPointerTypeEntry(const QString &name,
                          const QString &getterName,
                          const QString &smartPointerType,
                          const QString &refCountMethodName,
                          double vr)
        : ComplexTypeEntry(name, SmartPointerType, vr),
          m_getterName(getterName),
          m_smartPointerType(smartPointerType),
          m_refCountMethodName(refCountMethodName)
    {
    }

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
    NamespaceTypeEntry(const QString &name, double vr) : ComplexTypeEntry(name, NamespaceType, vr) { }
};


class ValueTypeEntry : public ComplexTypeEntry
{
public:
    ValueTypeEntry(const QString &name, double vr) : ComplexTypeEntry(name, BasicValueType, vr) { }

    bool isValue() const override
    {
        return true;
    }

    bool isNativeIdBased() const override
    {
        return true;
    }

protected:
    ValueTypeEntry(const QString &name, Type t, double vr) : ComplexTypeEntry(name, t, vr) { }
};


class StringTypeEntry : public ValueTypeEntry
{
public:
    StringTypeEntry(const QString &name, double vr)
            : ValueTypeEntry(name, StringType, vr)
    {
        setCodeGeneration(GenerateNothing);
    }

    QString targetLangApiName() const override;
    QString targetLangName() const override;
    QString targetLangPackage() const override;

    bool isNativeIdBased() const override
    {
        return false;
    }
};

class CharTypeEntry : public ValueTypeEntry
{
public:
    CharTypeEntry(const QString &name, double vr) : ValueTypeEntry(name, CharType, vr)
    {
        setCodeGeneration(GenerateNothing);
    }

    QString targetLangApiName() const override;
    QString targetLangName() const override;
    QString targetLangPackage() const override
    {
        return QString();
    }

    bool isNativeIdBased() const override
    {
        return false;
    }
};

class VariantTypeEntry: public ValueTypeEntry
{
public:
    VariantTypeEntry(const QString &name, double vr) : ValueTypeEntry(name, VariantType, vr) { }

    QString targetLangApiName() const override;
    QString targetLangName() const override;
    QString targetLangPackage() const override;

    bool isNativeIdBased() const override
    {
        return false;
    }
};


class InterfaceTypeEntry : public ComplexTypeEntry
{
public:
    InterfaceTypeEntry(const QString &name, double vr)
            : ComplexTypeEntry(name, InterfaceType, vr) {}

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

    bool isNativeIdBased() const override
    {
        return true;
    }
    QString qualifiedCppName() const override
    {
        const int len = ComplexTypeEntry::qualifiedCppName().length() - interfaceName(QString()).length();
        return ComplexTypeEntry::qualifiedCppName().left(len);
    }

private:
    ObjectTypeEntry *m_origin;
};


class FunctionTypeEntry : public TypeEntry
{
public:
    FunctionTypeEntry(const QString& name, const QString& signature, double vr)
            : TypeEntry(name, FunctionType, vr)
    {
        addSignature(signature);
    }
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
    ObjectTypeEntry(const QString &name, double vr)
            : ComplexTypeEntry(name, ObjectType, vr), m_interface(0) {}

    InterfaceTypeEntry *designatedInterface() const
    {
        return m_interface;
    }
    void setDesignatedInterface(InterfaceTypeEntry *entry)
    {
        m_interface = entry;
    }

    bool isNativeIdBased() const override
    {
        return true;
    }

private:
    InterfaceTypeEntry *m_interface;
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
