/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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


#ifndef CODEMODEL_H
#define CODEMODEL_H

#include "codemodel_fwd.h"
#include "codemodel_enums.h"
#include "enumvalue.h"
#include "typeinfo.h"

#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

#include <optional>

QT_FORWARD_DECLARE_CLASS(QDebug)

#define DECLARE_MODEL_NODE(k) \
    enum { __node_kind = Kind_##k };

class SourceLocation;

class CodeModel
{
    Q_GADGET
public:
    Q_DISABLE_COPY(CodeModel)

    enum AccessPolicy {
        Public,
        Protected,
        Private
    };
    Q_ENUM(AccessPolicy)

    enum FunctionType {
        Normal,
        Constructor,
        CopyConstructor,
        MoveConstructor,
        Destructor,
        Signal,
        Slot,
        AssignmentOperator,
        CallOperator,
        ConversionOperator,
        DereferenceOperator, // Iterator's operator *
        ReferenceOperator, // operator &
        ArrowOperator,
        ArithmeticOperator,
        BitwiseOperator,
        LogicalOperator,
        ShiftOperator,
        SubscriptOperator,
        ComparisonOperator
    };
    Q_ENUM(FunctionType)

    enum ClassType {
        Class,
        Struct,
        Union
    };
    Q_ENUM(ClassType)

public:
    CodeModel();
    virtual ~CodeModel();

    FileList files() const { return m_files; }
    NamespaceModelItem globalNamespace() const;

    void addFile(const FileModelItem &item);
    FileModelItem findFile(const QString &name) const;

    CodeModelItem findItem(const QStringList &qualifiedName, const ScopeModelItem &scope) const;

private:
    FileList m_files;
    NamespaceModelItem m_globalNamespace;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const CodeModel *m);
#endif

class _CodeModelItem
{
    Q_DISABLE_COPY(_CodeModelItem)
public:
    enum Kind {
        /* These are bit-flags resembling inheritance */
        Kind_Scope = 0x1,
        Kind_Namespace = 0x2 | Kind_Scope,
        Kind_Member = 0x4,
        Kind_Function = 0x8 | Kind_Member,
        KindMask = 0xf,

        /* These are for classes that are not inherited from */
        FirstKind = 0x8,
        Kind_Argument = 1 << FirstKind,
        Kind_Class = 2 << FirstKind | Kind_Scope,
        Kind_Enum = 3 << FirstKind,
        Kind_Enumerator = 4 << FirstKind,
        Kind_File = 5 << FirstKind | Kind_Namespace,
        Kind_TemplateParameter = 7 << FirstKind,
        Kind_TypeDef = 8 << FirstKind,
        Kind_TemplateTypeAlias = 9 << FirstKind,
        Kind_Variable = 10 << FirstKind | Kind_Member
    };

public:
    virtual ~_CodeModelItem();

    int kind() const;

    QStringList qualifiedName() const;

    QString name() const;
    void setName(const QString &name);

    QStringList scope() const;
    void setScope(const QStringList &scope);

    QString fileName() const;
    void setFileName(const QString &fileName);

    FileModelItem file() const;

    void getStartPosition(int *line, int *column);
    int startLine() const { return m_startLine; }
    void setStartPosition(int line, int column);

    void getEndPosition(int *line, int *column);
    void setEndPosition(int line, int column);

    SourceLocation sourceLocation() const;

    inline CodeModel *model() const { return m_model; }

#ifndef QT_NO_DEBUG_STREAM
    static void formatKind(QDebug &d, int k);
    virtual void formatDebug(QDebug &d) const;
#endif

protected:
    explicit _CodeModelItem(CodeModel *model, int kind);
    explicit _CodeModelItem(CodeModel *model, const QString &name, int kind);

private:
    CodeModel *m_model;
    int m_kind;
    int m_startLine;
    int m_startColumn;
    int m_endLine;
    int m_endColumn;
    QString m_name;
    QString m_fileName;
    QStringList m_scope;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const _CodeModelItem *t);
#endif

class _ScopeModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(Scope)

    ~_ScopeModelItem();

    ClassList classes() const { return m_classes; }
    EnumList enums() const { return m_enums; }
    inline const FunctionList &functions() const { return m_functions; }
    TypeDefList typeDefs() const { return m_typeDefs; }
    TemplateTypeAliasList templateTypeAliases() const { return m_templateTypeAliases; }
    VariableList variables() const { return m_variables; }

    void addClass(const ClassModelItem &item);
    void addEnum(const EnumModelItem &item);
    void addFunction(const FunctionModelItem &item);
    void addTypeDef(const TypeDefModelItem &item);
    void addTemplateTypeAlias(const TemplateTypeAliasModelItem &item);
    void addVariable(const VariableModelItem &item);

    ClassModelItem findClass(const QString &name) const;
    EnumModelItem findEnum(const QString &name) const;
    FunctionList findFunctions(const QString &name) const;
    TypeDefModelItem findTypeDef(const QString &name) const;
    TemplateTypeAliasModelItem findTemplateTypeAlias(const QString &name) const;
    VariableModelItem findVariable(const QString &name) const;

    void addEnumsDeclaration(const QString &enumsDeclaration);
    QStringList enumsDeclarations() const { return m_enumsDeclarations; }

    FunctionModelItem declaredFunction(const FunctionModelItem &item);

    bool isEmpty() const;
    void purgeClassDeclarations();

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

protected:
    explicit _ScopeModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind) {}
    explicit _ScopeModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind) {}

    void appendScope(const _ScopeModelItem &other);

#ifndef QT_NO_DEBUG_STREAM
    void formatScopeItemsDebug(QDebug &d) const;
#endif

private:
    ClassList m_classes;
    EnumList m_enums;
    TypeDefList m_typeDefs;
    TemplateTypeAliasList m_templateTypeAliases;
    VariableList m_variables;
    FunctionList m_functions;

private:
    QStringList m_enumsDeclarations;
};

class _ClassModelItem: public _ScopeModelItem
{
public:
    DECLARE_MODEL_NODE(Class)

    struct BaseClass
    {
        QString name;
        CodeModel::AccessPolicy accessPolicy = CodeModel::Public;
    };

    explicit _ClassModelItem(CodeModel *model, int kind = __node_kind)
        : _ScopeModelItem(model, kind), m_classType(CodeModel::Class) {}
    explicit _ClassModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _ScopeModelItem(model, name, kind), m_classType(CodeModel::Class) {}
    ~_ClassModelItem();

    QList<BaseClass> baseClasses() const { return m_baseClasses; }

    void addBaseClass(const QString &name, CodeModel::AccessPolicy accessPolicy);

    TemplateParameterList templateParameters() const;
    void setTemplateParameters(const TemplateParameterList &templateParameters);

    bool extendsClass(const QString &name) const;

    void setClassType(CodeModel::ClassType type);
    CodeModel::ClassType classType() const;

    void addPropertyDeclaration(const QString &propertyDeclaration);
    QStringList propertyDeclarations() const { return m_propertyDeclarations; }

    bool isFinal() const { return m_final; }
    void setFinal(bool f) { m_final = f; }

    bool isEmpty() const;
    bool isTemplate() const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    QList<BaseClass> m_baseClasses;
    TemplateParameterList m_templateParameters;
    CodeModel::ClassType m_classType;

    QStringList m_propertyDeclarations;
    bool m_final = false;
};

class _NamespaceModelItem: public _ScopeModelItem
{
public:
    DECLARE_MODEL_NODE(Namespace)

    explicit _NamespaceModelItem(CodeModel *model, int kind = __node_kind)
        : _ScopeModelItem(model, kind) {}
    explicit _NamespaceModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _ScopeModelItem(model, name, kind) {}
    ~_NamespaceModelItem();

    const NamespaceList &namespaces() const { return m_namespaces; }

     NamespaceType type() const { return m_type; }
     void setType(NamespaceType t) { m_type = t; }

    void addNamespace(NamespaceModelItem item);

    NamespaceModelItem findNamespace(const QString &name) const;

    void appendNamespace(const _NamespaceModelItem &other);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    NamespaceList m_namespaces;
    NamespaceType m_type = NamespaceType::Default;
};

class _FileModelItem: public _NamespaceModelItem
{
public:
    DECLARE_MODEL_NODE(File)

    explicit _FileModelItem(CodeModel *model, int kind = __node_kind)
        : _NamespaceModelItem(model, kind) {}
    explicit _FileModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _NamespaceModelItem(model, name, kind) {}
    ~_FileModelItem();
};

class _ArgumentModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(Argument)

    explicit _ArgumentModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind), m_defaultValue(false) {}
    explicit _ArgumentModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind), m_defaultValue(false) {}
    ~_ArgumentModelItem();

    TypeInfo type() const;
    void setType(const TypeInfo &type);

    bool defaultValue() const;
    void setDefaultValue(bool defaultValue);

    QString defaultValueExpression() const { return m_defaultValueExpression; }
    void setDefaultValueExpression(const QString &expr) { m_defaultValueExpression = expr; }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    TypeInfo m_type;
    QString m_defaultValueExpression;
    bool m_defaultValue;
};

class _MemberModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(Member)

    explicit _MemberModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind), m_accessPolicy(CodeModel::Public), m_flags(0) {}
    explicit _MemberModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind), m_accessPolicy(CodeModel::Public), m_flags(0) {}
    ~_MemberModelItem();

    bool isConstant() const;
    void setConstant(bool isConstant);

    bool isVolatile() const;
    void setVolatile(bool isVolatile);

    bool isStatic() const;
    void setStatic(bool isStatic);

    bool isAuto() const;
    void setAuto(bool isAuto);

    bool isFriend() const;
    void setFriend(bool isFriend);

    bool isRegister() const;
    void setRegister(bool isRegister);

    bool isExtern() const;
    void setExtern(bool isExtern);

    bool isMutable() const;
    void setMutable(bool isMutable);

    CodeModel::AccessPolicy accessPolicy() const;
    void setAccessPolicy(CodeModel::AccessPolicy accessPolicy);

    TemplateParameterList templateParameters() const { return m_templateParameters; }
    void setTemplateParameters(const TemplateParameterList &templateParameters) { m_templateParameters = templateParameters; }

    TypeInfo type() const;
    void setType(const TypeInfo &type);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    TemplateParameterList m_templateParameters;
    TypeInfo m_type;
    CodeModel::AccessPolicy m_accessPolicy;
    union {
        struct {
            uint m_isConstant: 1;
            uint m_isVolatile: 1;
            uint m_isStatic: 1;
            uint m_isAuto: 1;
            uint m_isFriend: 1;
            uint m_isRegister: 1;
            uint m_isExtern: 1;
            uint m_isMutable: 1;
        };
        uint m_flags;
    };

};

class _FunctionModelItem: public _MemberModelItem
{
public:
    DECLARE_MODEL_NODE(Function)

    explicit _FunctionModelItem(CodeModel *model, int kind = __node_kind)
        : _MemberModelItem(model, kind), m_functionType(CodeModel::Normal), m_flags(0) {}
    explicit _FunctionModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _MemberModelItem(model, name, kind), m_functionType(CodeModel::Normal), m_flags(0) {}
    ~_FunctionModelItem();

    ArgumentList arguments() const;

    void addArgument(const ArgumentModelItem& item);

    CodeModel::FunctionType functionType() const;
    void setFunctionType(CodeModel::FunctionType functionType);

    static std::optional<CodeModel::FunctionType> functionTypeFromName(QStringView name);

    bool isDeleted() const;
    void setDeleted(bool d);

    bool isDeprecated() const;
    void setDeprecated(bool d);

    bool isVirtual() const;
    void setVirtual(bool isVirtual);

    bool isOverride() const;
    void setOverride(bool o);

    bool isFinal() const;
    void setFinal(bool f);

    bool isInline() const;
    void setInline(bool isInline);

    bool isExplicit() const;
    void setExplicit(bool isExplicit);

    bool isInvokable() const; // Qt
    void setInvokable(bool isInvokable); // Qt

    bool isAbstract() const;
    void setAbstract(bool isAbstract);

    bool isVariadics() const;
    void setVariadics(bool isVariadics);


    bool isSimilar(const FunctionModelItem &other) const;

    bool isNoExcept() const;

    ExceptionSpecification exceptionSpecification() const;
    void setExceptionSpecification(ExceptionSpecification e);

    QString typeSystemSignature() const; // For dumping out type system files

    // Private, for usage by the clang builder.
    void _determineType();

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    CodeModel::FunctionType _determineTypeHelper() const;

    ArgumentList m_arguments;
    CodeModel::FunctionType m_functionType;
    union {
        struct {
            uint m_isDeleted: 1;
            uint m_isVirtual: 1;
            uint m_isOverride: 1;
            uint m_isFinal: 1;
            uint m_isDeprecated: 1;
            uint m_isInline: 1;
            uint m_isAbstract: 1;
            uint m_isExplicit: 1;
            uint m_isVariadics: 1;
            uint m_isInvokable : 1; // Qt
        };
        uint m_flags;
    };
    ExceptionSpecification m_exceptionSpecification = ExceptionSpecification::Unknown;
};

class _VariableModelItem: public _MemberModelItem
{
public:
    DECLARE_MODEL_NODE(Variable)

    explicit _VariableModelItem(CodeModel *model, int kind = __node_kind)
        : _MemberModelItem(model, kind) {}
    explicit _VariableModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _MemberModelItem(model, name, kind) {}
};

class _TypeDefModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(TypeDef)

    explicit _TypeDefModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind) {}
    explicit _TypeDefModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind) {}

    TypeInfo type() const;
    void setType(const TypeInfo &type);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    TypeInfo m_type;
};

class _TemplateTypeAliasModelItem : public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(TemplateTypeAlias)

    explicit _TemplateTypeAliasModelItem(CodeModel *model, int kind = __node_kind);
    explicit _TemplateTypeAliasModelItem(CodeModel *model, const QString &name,
                                         int kind = __node_kind);

    TemplateParameterList templateParameters() const;
    void addTemplateParameter(const TemplateParameterModelItem &templateParameter);

    TypeInfo type() const;
    void setType(const TypeInfo &type);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    TemplateParameterList m_templateParameters;
    TypeInfo m_type;
};

class _EnumModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(Enum)

    explicit _EnumModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind) {}
    explicit _EnumModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind) {}
    ~_EnumModelItem();

    CodeModel::AccessPolicy accessPolicy() const;
    void setAccessPolicy(CodeModel::AccessPolicy accessPolicy);

    bool hasValues() const { return !m_enumerators.isEmpty(); }
    EnumeratorList enumerators() const;
    void addEnumerator(const EnumeratorModelItem &item);

    EnumKind enumKind() const { return m_enumKind; }
    void setEnumKind(EnumKind kind) { m_enumKind = kind; }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

    bool isSigned() const;
    void setSigned(bool s);

private:
    CodeModel::AccessPolicy m_accessPolicy = CodeModel::Public;
    EnumeratorList m_enumerators;
    EnumKind m_enumKind = CEnum;
    bool m_signed = true;
};

class _EnumeratorModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(Enumerator)

    explicit _EnumeratorModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind) {}
    explicit _EnumeratorModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind) {}
    ~_EnumeratorModelItem();

    QString stringValue() const;
    void setStringValue(const QString &stringValue);

    EnumValue value() const { return m_value; }
    void setValue(EnumValue v) { m_value = v; }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    QString m_stringValue;
    EnumValue m_value;
};

class _TemplateParameterModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(TemplateParameter)

    explicit _TemplateParameterModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind), m_defaultValue(false) {}
    explicit _TemplateParameterModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind), m_defaultValue(false) {}
    ~_TemplateParameterModelItem();

    TypeInfo type() const;
    void setType(const TypeInfo &type);

    bool defaultValue() const;
    void setDefaultValue(bool defaultValue);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const override;
#endif

private:
    TypeInfo m_type;
    bool m_defaultValue;
};

#endif // CODEMODEL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
