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
#include "parser/typeinfo.h"

#include <QtCore/QList>
#include <QtCore/QRegularExpression>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

class ArgumentModificationData;
class FunctionModificationData;
class ModificationData;
class FieldModificationData;

QT_BEGIN_NAMESPACE
class QDebug;
QT_END_NAMESPACE

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
    static QRegularExpression placeHolderRegex(int index);
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
};

class ArgumentModification
{
public:
    ArgumentModification();
    explicit ArgumentModification(int idx);
    ArgumentModification(const ArgumentModification &);
    ArgumentModification &operator=(const ArgumentModification &);
    ArgumentModification(ArgumentModification &&);
    ArgumentModification &operator=(ArgumentModification &&);
    ~ArgumentModification();

    // Reference count flags for this argument
    const QList<ReferenceCount> &referenceCounts() const;
    void addReferenceCount(const ReferenceCount &value);

    // The text given for the new type of the argument
    QString modifiedType() const;
    void setModifiedType(const QString &value);

     // The text of the new default expression of the argument
    QString replacedDefaultExpression() const;
    void setReplacedDefaultExpression(const QString &value);

    // The new definition of ownership for a specific argument
    const QHash<TypeSystem::Language, TypeSystem::Ownership> &ownerships() const;
    void insertOwnership(TypeSystem::Language l, TypeSystem::Ownership o);

    // Different conversion rules
    const CodeSnipList &conversionRules() const;
    CodeSnipList &conversionRules();

    // QObject parent(owner) of this argument
    ArgumentOwner owner() const;
    void setOwner(const ArgumentOwner &value);

    // New name
    QString renamedToName() const;
    void setRenamedToName(const QString &value);

    int index() const;
    void setIndex(int value);

    bool removedDefaultExpression() const;
    void setRemovedDefaultExpression(const uint &value);

    bool isRemoved() const;
    void setRemoved(bool value);

    bool noNullPointers() const;
    void setNoNullPointers(bool value);

    bool resetAfterUse() const;
    void setResetAfterUse(bool value);

    // consider "int*" to be "int[]"
    bool isArray() const;
    void setArray(bool value);

private:
    QSharedDataPointer<ArgumentModificationData> d;
};

class Modification
{
public:

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

    Modification();
    Modification(const Modification &);
    Modification &operator=(const Modification &);
    Modification(Modification &&);
    Modification &operator=(Modification &&);
    ~Modification();

    QString renamedToName() const;
    void setRenamedToName(const QString &value);

    Modifiers modifiers() const;
    void setModifiers(Modifiers m);
    void setModifierFlag(ModifierFlag f);
    void clearModifierFlag(ModifierFlag f);
    bool isRemoved() const;
    void setRemoved(bool r);

    bool isAccessModifier() const
    {
        return (modifiers() & AccessModifierMask) != 0;
    }
    Modifiers accessModifier() const
    {
        return modifiers() & AccessModifierMask;
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
        return modifiers().testFlag(Final);
    }
    bool isNonFinal() const
    {
        return modifiers().testFlag(NonFinal);
    }
    QString accessModifierString() const;

    bool isDeprecated() const
    {
        return modifiers().testFlag(Deprecated);
    }

    bool isRenameModifier() const
    {
        return modifiers().testFlag(Rename);
    }

    bool isRemoveModifier() const { return isRemoved(); }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

private:
    QSharedDataPointer<ModificationData> md;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Modification::Modifiers)

class FunctionModification: public Modification
{
public:
    using AllowThread = TypeSystem::AllowThread;

    FunctionModification();
    FunctionModification(const FunctionModification &);
    FunctionModification &operator=(const FunctionModification &);
    FunctionModification(FunctionModification &&);
    FunctionModification &operator=(FunctionModification &&);
    ~FunctionModification();

    bool isCodeInjection() const
    {
        return modifiers().testFlag(CodeInjection);
    }
    void setIsThread(bool flag);
    bool isThread() const;

    AllowThread allowThread() const;
    void setAllowThread(AllowThread allow);

    bool matches(const QString &functionSignature) const;

    bool setSignature(const QString &s, QString *errorMessage =  nullptr);
    QString signature() const;

    void setOriginalSignature(const QString &s);
    QString originalSignature() const;

    TypeSystem::ExceptionHandling exceptionHandling() const;
    void setExceptionHandling(TypeSystem::ExceptionHandling e);

    int overloadNumber() const;
    void setOverloadNumber(int overloadNumber);

    const CodeSnipList &snips() const;
    CodeSnipList &snips();
    void appendSnip(const CodeSnip &snip);
    void setSnips(const CodeSnipList &snips);

    const QList<ArgumentModification> &argument_mods() const;
    QList<ArgumentModification> &argument_mods();
    void setArgument_mods(const QList<ArgumentModification> &argument_mods);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

private:
    QSharedDataPointer<FunctionModificationData> d;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const ReferenceCount &);
QDebug operator<<(QDebug d, const ArgumentOwner &a);
QDebug operator<<(QDebug d, const ArgumentModification &a);
QDebug operator<<(QDebug d, const FunctionModification &fm);
#endif

class FieldModification
{
public:
    FieldModification();
    FieldModification(const FieldModification &);
    FieldModification &operator=(const FieldModification &);
    FieldModification(FieldModification &&);
    FieldModification &operator=(FieldModification &&);
    ~FieldModification();

    QString name() const;
    void setName(const QString &value);

    bool isRenameModifier() const;
    QString renamedToName() const;
    void setRenamedToName(const QString &value);

    bool isReadable() const;
    void setReadable(bool e);

    bool isWritable() const;
    void setWritable(bool e);

    bool isRemoved() const;
    void setRemoved(bool r);

private:
    QSharedDataPointer<FieldModificationData> d;
};

/**
*   \internal
*   Struct used to store information about functions added by the typesystem.
*   This info will be used later to create a fake AbstractMetaFunction which
*   will be inserted into the right AbstractMetaClass.
*/
struct AddedFunction
{
    using AddedFunctionPtr = QSharedPointer<AddedFunction>;

    /// Function access types.
    enum Access {
        InvalidAccess = 0,
        Protected = 0x1,
        Public =    0x2
    };

    struct Argument
    {
        TypeInfo typeInfo;
        QString name;
        QString defaultValue;
    };

    /// Creates a new AddedFunction with a signature and a return type.
    explicit AddedFunction(const QString &name, const QList<Argument> &arguments,
                           const TypeInfo &returnType);

    static AddedFunctionPtr createAddedFunction(const QString &signatureIn,
                                                const QString &returnTypeIn,
                                                QString *errorMessage);

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
    void setConstant(bool c) { m_isConst = c; };

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
    Access m_access = Public;
    bool m_isConst = false;
    bool m_isStatic = false;
    bool m_isDeclaration = false;
};

#ifndef QT_NO_DEBUG_STREAM
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
