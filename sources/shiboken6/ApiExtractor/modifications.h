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

#ifndef MODIFICATIONS_H
#define MODIFICATIONS_H

#include "typesystem_enums.h"
#include "typesystem_typedefs.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QDebug;
QT_END_NAMESPACE

using ArgumentMap = QMap<int, QString>;

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

    QList<CodeSnipFragment> codeList;

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
    QList<ReferenceCount> referenceCounts;

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
    enum ModifierFlag {
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

    Q_DECLARE_FLAGS(Modifiers, ModifierFlag);

    bool isAccessModifier() const
    {
        return (modifiers & AccessModifierMask) != 0;
    }
    Modifiers accessModifier() const
    {
        return modifiers & AccessModifierMask;
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
        return modifiers.testFlag(Final);
    }
    bool isNonFinal() const
    {
        return modifiers.testFlag(NonFinal);
    }
    QString accessModifierString() const;

    bool isDeprecated() const
    {
        return modifiers.testFlag(Deprecated);
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
        return modifiers.testFlag(Rename);
    }

    bool isRemoveModifier() const
    {
        return removal != TypeSystem::NoLanguage;
    }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

    QString renamedToName;
    Modifiers modifiers;
    TypeSystem::Language removal = TypeSystem::NoLanguage;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Modification::Modifiers)

struct FunctionModification: public Modification
{
    using AllowThread = TypeSystem::AllowThread;

    bool isCodeInjection() const
    {
        return modifiers.testFlag(CodeInjection);
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

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

    QString association;
    CodeSnipList snips;

    QList<ArgumentModification> argument_mods;

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
        return modifiers.testFlag(Readable);
    }
    bool isWritable() const
    {
        return modifiers.testFlag(Writable);
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
        int indirections = 0;
        bool isConstant = false;
        bool isReference = false;
    };

    struct Argument
    {
        TypeInfo typeInfo;
        QString name;
        QString defaultValue;
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
    const QList<Argument> &arguments() const
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

    bool isDeclaration() const { return m_isDeclaration; } // <declare-function>
    void setDeclaration(bool value) { m_isDeclaration = value; }

    FunctionModificationList modifications;

private:
    QString m_name;
    QList<Argument> m_arguments;
    TypeInfo m_returnType;
    Access m_access = Protected;
    bool m_isConst = false;
    bool m_isStatic = false;
    bool m_isDeclaration = false;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AddedFunction::TypeInfo &ti);
QDebug operator<<(QDebug d, const AddedFunction::Argument &a);
QDebug operator<<(QDebug d, const AddedFunction &af);
#endif

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

#endif // MODIFICATIONS_H
