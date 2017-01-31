/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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


#ifndef CODEMODEL_H
#define CODEMODEL_H

#include "codemodel_fwd.h"

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

QT_FORWARD_DECLARE_CLASS(QDebug)

#define DECLARE_MODEL_NODE(k) \
    enum { __node_kind = Kind_##k };

class CodeModel
{
public:
    enum AccessPolicy {
        Public,
        Protected,
        Private
    };

    enum FunctionType {
        Normal,
        Signal,
        Slot
    };

    enum ClassType {
        Class,
        Struct,
        Union
    };

public:
    CodeModel();
    virtual ~CodeModel();

    FileList files() const { return m_files; }
    NamespaceModelItem globalNamespace() const;

    void addFile(FileModelItem item);
    FileModelItem findFile(const QString &name) const;

    CodeModelItem findItem(const QStringList &qualifiedName, CodeModelItem scope) const;

private:
    FileList m_files;
    NamespaceModelItem m_globalNamespace;

private:
    CodeModel(const CodeModel &other);
    void operator = (const CodeModel &other);
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const CodeModel *m);
#endif

class TypeInfo
{
public:
    enum ReferenceType {
        NoReference,
        LValueReference,
        RValueReference
    };

    TypeInfo() : flags(0), m_referenceType(NoReference) {}

    QStringList qualifiedName() const
    {
        return m_qualifiedName;
    }

    void setQualifiedName(const QStringList &qualified_name)
    {
        m_qualifiedName = qualified_name;
    }

    bool isConstant() const
    {
        return m_constant;
    }

    void setConstant(bool is)
    {
        m_constant = is;
    }

    bool isVolatile() const
    {
        return m_volatile;
    }

    void setVolatile(bool is)
    {
        m_volatile = is;
    }

    ReferenceType referenceType() const { return m_referenceType; }
    void setReferenceType(ReferenceType r) { m_referenceType = r; }

    int indirections() const
    {
        return m_indirections;
    }

    void setIndirections(int indirections)
    {
        m_indirections = indirections;
    }

    bool isFunctionPointer() const
    {
        return m_functionPointer;
    }
    void setFunctionPointer(bool is)
    {
        m_functionPointer = is;
    }

    QStringList arrayElements() const
    {
        return m_arrayElements;
    }
    void setArrayElements(const QStringList &arrayElements)
    {
        m_arrayElements = arrayElements;
    }

    QList<TypeInfo> arguments() const
    {
        return m_arguments;
    }

    void setArguments(const QList<TypeInfo> &arguments);

    void addArgument(const TypeInfo &arg)
    {
        m_arguments.append(arg);
    }

    bool operator==(const TypeInfo &other);

    bool operator!=(const TypeInfo &other)
    {
        return !(*this == other);
    }

    // ### arrays and templates??

    QString toString() const;

    static TypeInfo combine(const TypeInfo &__lhs, const TypeInfo &__rhs);
    static TypeInfo resolveType(TypeInfo const &__type, CodeModelItem __scope);

private:
    static TypeInfo resolveType(CodeModelItem item, TypeInfo const &__type, CodeModelItem __scope);

    QStringList m_qualifiedName;
    QStringList m_arrayElements;
    QList<TypeInfo> m_arguments;

    union {
        uint flags;

        struct {
            uint m_constant: 1;
            uint m_volatile: 1;
            uint m_functionPointer: 1;
            uint m_indirections: 6;
            uint m_padding: 23;
        };
    };

    ReferenceType m_referenceType;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeInfo &t);
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
        Kind_FunctionDefinition = 6 << FirstKind | Kind_Function,
        Kind_TemplateParameter = 7 << FirstKind,
        Kind_TypeDef = 8 << FirstKind,
        Kind_Variable = 9 << FirstKind | Kind_Member
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
    void setStartPosition(int line, int column);

    void getEndPosition(int *line, int *column);
    void setEndPosition(int line, int column);

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

    ClassList classes() const;
    EnumList enums() const { return m_enums; }
    FunctionDefinitionList functionDefinitions() const;
    FunctionList functions() const;
    TypeDefList typeDefs() const { return m_typeDefs; }
    VariableList variables() const { return m_variables; }

    void addClass(ClassModelItem item);
    void addEnum(EnumModelItem item);
    void addFunction(FunctionModelItem item);
    void addFunctionDefinition(FunctionDefinitionModelItem item);
    void addTypeDef(TypeDefModelItem item);
    void addVariable(VariableModelItem item);

    ClassModelItem findClass(const QString &name) const;
    EnumModelItem findEnum(const QString &name) const;
    FunctionDefinitionList findFunctionDefinitions(const QString &name) const;
    FunctionList findFunctions(const QString &name) const;
    TypeDefModelItem findTypeDef(const QString &name) const;
    VariableModelItem findVariable(const QString &name) const;

    void addEnumsDeclaration(const QString &enumsDeclaration);
    QStringList enumsDeclarations() const { return m_enumsDeclarations; }

    inline QHash<QString, ClassModelItem> classMap() const { return m_classes; }
    inline QMultiHash<QString, FunctionDefinitionModelItem> functionDefinitionMap() const { return m_functionDefinitions; }
    inline QMultiHash<QString, FunctionModelItem> functionMap() const { return m_functions; }

    FunctionModelItem declaredFunction(FunctionModelItem item);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

protected:
    explicit _ScopeModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind) {}
    explicit _ScopeModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind) {}

#ifndef QT_NO_DEBUG_STREAM
    void formatScopeItemsDebug(QDebug &d) const;
#endif

private:
    QHash<QString, ClassModelItem> m_classes;
    EnumList m_enums;
    TypeDefList m_typeDefs;
    VariableList m_variables;
    QMultiHash<QString, FunctionDefinitionModelItem> m_functionDefinitions;
    QMultiHash<QString, FunctionModelItem> m_functions;

private:
    QStringList m_enumsDeclarations;
};

class _ClassModelItem: public _ScopeModelItem
{
public:
    DECLARE_MODEL_NODE(Class)

    explicit _ClassModelItem(CodeModel *model, int kind = __node_kind)
        : _ScopeModelItem(model, kind), m_classType(CodeModel::Class) {}
    explicit _ClassModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _ScopeModelItem(model, name, kind), m_classType(CodeModel::Class) {}
    ~_ClassModelItem();

    QStringList baseClasses() const;

    void setBaseClasses(const QStringList &baseClasses);
    void addBaseClass(const QString &baseClass);

    TemplateParameterList templateParameters() const;
    void setTemplateParameters(const TemplateParameterList &templateParameters);

    bool extendsClass(const QString &name) const;

    void setClassType(CodeModel::ClassType type);
    CodeModel::ClassType classType() const;

    void addPropertyDeclaration(const QString &propertyDeclaration);
    QStringList propertyDeclarations() const { return m_propertyDeclarations; }

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

private:
    QStringList m_baseClasses;
    TemplateParameterList m_templateParameters;
    CodeModel::ClassType m_classType;

    QStringList m_propertyDeclarations;
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

    NamespaceList namespaces() const { return m_namespaces; }

    void addNamespace(NamespaceModelItem item);

    NamespaceModelItem findNamespace(const QString &name) const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

private:
    NamespaceList m_namespaces;
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
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
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
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
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

    void addArgument(ArgumentModelItem item);

    CodeModel::FunctionType functionType() const;
    void setFunctionType(CodeModel::FunctionType functionType);

    bool isVirtual() const;
    void setVirtual(bool isVirtual);

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

    bool isSimilar(FunctionModelItem other) const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

private:
    ArgumentList m_arguments;
    CodeModel::FunctionType m_functionType;
    union {
        struct {
            uint m_isVirtual: 1;
            uint m_isInline: 1;
            uint m_isAbstract: 1;
            uint m_isExplicit: 1;
            uint m_isVariadics: 1;
            uint m_isInvokable : 1; // Qt
        };
        uint m_flags;
    };
};

class _FunctionDefinitionModelItem: public _FunctionModelItem
{
public:
    DECLARE_MODEL_NODE(FunctionDefinition)

    explicit _FunctionDefinitionModelItem(CodeModel *model, int kind = __node_kind)
        : _FunctionModelItem(model, kind) {}
    explicit _FunctionDefinitionModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _FunctionModelItem(model, name, kind) {}
    ~_FunctionDefinitionModelItem();
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
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

private:
    TypeInfo m_type;
};

class _EnumModelItem: public _CodeModelItem
{
public:
    DECLARE_MODEL_NODE(Enum)

    explicit _EnumModelItem(CodeModel *model, int kind = __node_kind)
        : _CodeModelItem(model, kind), m_accessPolicy(CodeModel::Public), m_anonymous(false)  {}
    explicit _EnumModelItem(CodeModel *model, const QString &name, int kind = __node_kind)
        : _CodeModelItem(model, name, kind), m_accessPolicy(CodeModel::Public), m_anonymous(false) {}
    ~_EnumModelItem();

    CodeModel::AccessPolicy accessPolicy() const;
    void setAccessPolicy(CodeModel::AccessPolicy accessPolicy);

    EnumeratorList enumerators() const;
    void addEnumerator(EnumeratorModelItem item);
    bool isAnonymous() const;
    void setAnonymous(bool anonymous);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

private:
    CodeModel::AccessPolicy m_accessPolicy;
    EnumeratorList m_enumerators;
    bool m_anonymous;
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

    QString value() const;
    void setValue(const QString &value);

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

private:
    QString m_value;
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
    void formatDebug(QDebug &d) const Q_DECL_OVERRIDE;
#endif

private:
    TypeInfo m_type;
    bool m_defaultValue;
};

#endif // CODEMODEL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
