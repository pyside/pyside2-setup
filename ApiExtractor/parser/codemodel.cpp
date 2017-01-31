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


#include "codemodel.h"
#include <algorithm>
#include <iostream>
#include <QDebug>

// ---------------------------------------------------------------------------

CodeModel::CodeModel() : _M_globalNamespace(new _NamespaceModelItem(this))
{
}

CodeModel::~CodeModel()
{
}

FileList CodeModel::files() const
{
    return _M_files.values();
}

NamespaceModelItem CodeModel::globalNamespace() const
{
    return _M_globalNamespace;
}

void CodeModel::addFile(FileModelItem item)
{
    _M_files.insert(item->name(), item);
}

FileModelItem CodeModel::findFile(const QString &name) const
{
    return _M_files.value(name);
}

CodeModelItem CodeModel::findItem(const QStringList &qualifiedName, CodeModelItem scope) const
{
    for (int i = 0; i < qualifiedName.size(); ++i) {
        // ### Extend to look for members etc too.
        const QString &name = qualifiedName.at(i);

        if (NamespaceModelItem ns = qSharedPointerDynamicCast<_NamespaceModelItem>(scope)) {
            if (NamespaceModelItem tmp_ns = ns->findNamespace(name)) {
                scope = tmp_ns;
                continue;
            }
        }

        if (ScopeModelItem ss = qSharedPointerDynamicCast<_ScopeModelItem>(scope)) {
            if (ClassModelItem cs = ss->findClass(name)) {
                scope = cs;
            } else if (EnumModelItem es = ss->findEnum(name)) {
                if (i == qualifiedName.size() - 1)
                    return es;
            } else if (TypeDefModelItem tp = ss->findTypeDef(name)) {
                if (i == qualifiedName.size() - 1)
                    return tp;
            } else {
                // If we don't find the name in the scope chain we
                // need to return an empty item to indicate failure...
                return CodeModelItem();
            }
        }
    }

    return scope;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const CodeModel *m)
{
    QDebugStateSaver s(d);
    d.noquote();
    d.nospace();
    d << "CodeModel(";
    if (m) {
        const NamespaceModelItem globalNamespaceP = m->globalNamespace();
        if (globalNamespaceP.data())
            globalNamespaceP->formatDebug(d);
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
TypeInfo TypeInfo::combine(const TypeInfo &__lhs, const TypeInfo &__rhs)
{
    TypeInfo __result = __lhs;

    __result.setConstant(__result.isConstant() || __rhs.isConstant());
    __result.setVolatile(__result.isVolatile() || __rhs.isVolatile());
    if (__rhs.referenceType() > __result.referenceType())
        __result.setReferenceType(__rhs.referenceType());
    __result.setIndirections(__result.indirections() + __rhs.indirections());
    __result.setArrayElements(__result.arrayElements() + __rhs.arrayElements());

    return __result;
}

TypeInfo TypeInfo::resolveType(TypeInfo const &__type, CodeModelItem __scope)
{
    CodeModel *__model = __scope->model();
    Q_ASSERT(__model != 0);

    return TypeInfo::resolveType(__model->findItem(__type.qualifiedName(), __scope),  __type, __scope);
}

TypeInfo TypeInfo::resolveType(CodeModelItem __item, TypeInfo const &__type, CodeModelItem __scope)
{
    // Copy the type and replace with the proper qualified name. This
    // only makes sence to do if we're actually getting a resolved
    // type with a namespace. We only get this if the returned type
    // has more than 2 entries in the qualified name... This test
    // could be improved by returning if the type was found or not.
    TypeInfo otherType(__type);
    if (__item && __item->qualifiedName().size() > 1) {
        otherType.setQualifiedName(__item->qualifiedName());
    }

    if (TypeDefModelItem __typedef = qSharedPointerDynamicCast<_TypeDefModelItem>(__item)) {
        const TypeInfo combined = TypeInfo::combine(__typedef->type(), otherType);
        const CodeModelItem nextItem = __scope->model()->findItem(combined.qualifiedName(), __scope);
        if (!nextItem)
            return combined;
        // PYSIDE-362, prevent recursion on opaque structs like
        // typedef struct xcb_connection_t xcb_connection_t;
        if (nextItem.data() ==__item.data()) {
            std::cerr << "** WARNING Bailing out recursion of " << __FUNCTION__
                << "() on " << qPrintable(__type.qualifiedName().join(QLatin1String("::")))
                << std::endl;
            return otherType;
        }
        return resolveType(nextItem, combined, __scope);
    }

    return otherType;
}

QString TypeInfo::toString() const
{
    QString tmp;

    tmp += m_qualifiedName.join(QLatin1String("::"));
    if (isConstant())
        tmp += QLatin1String(" const");

    if (isVolatile())
        tmp += QLatin1String(" volatile");

    if (indirections())
        tmp += QString(indirections(), QLatin1Char('*'));

    switch (referenceType()) {
    case NoReference:
        break;
    case TypeInfo::LValueReference:
        tmp += QLatin1Char('&');
        break;
    case TypeInfo::RValueReference:
        tmp += QLatin1String("&&");
        break;
    }

    if (isFunctionPointer()) {
        tmp += QLatin1String(" (*)(");
        for (int i = 0; i < m_arguments.count(); ++i) {
            if (i != 0)
                tmp += QLatin1String(", ");

            tmp += m_arguments.at(i).toString();
        }
        tmp += QLatin1Char(')');
    }

    foreach(QString elt, arrayElements()) {
        tmp += QLatin1Char('[');
        tmp += elt;
        tmp += QLatin1Char(']');
    }

    return tmp;
}

bool TypeInfo::operator==(const TypeInfo &other)
{
    if (arrayElements().count() != other.arrayElements().count())
        return false;

#if defined (RXX_CHECK_ARRAY_ELEMENTS) // ### it'll break
    for (int i = 0; i < arrayElements().count(); ++i) {
        QString elt1 = arrayElements().at(i).trimmed();
        QString elt2 = other.arrayElements().at(i).trimmed();

        if (elt1 != elt2)
            return false;
    }
#endif

    return flags == other.flags
           && m_qualifiedName == other.m_qualifiedName
           && (!m_functionPointer || m_arguments == other.m_arguments);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeInfo &t)
{
    QDebugStateSaver s(d);
    d.noquote();
    d.nospace();
    d << "TypeInfo(" << t.toString() << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
_CodeModelItem::_CodeModelItem(CodeModel *model, int kind)
        : _M_model(model),
        _M_kind(kind),
        _M_startLine(0),
        _M_startColumn(0),
        _M_endLine(0),
        _M_endColumn(0)
{
}

_CodeModelItem::_CodeModelItem(CodeModel *model, const QString &name, int kind)
    : _M_model(model),
    _M_kind(kind),
    _M_startLine(0),
    _M_startColumn(0),
    _M_endLine(0),
    _M_endColumn(0),
    _M_name(name)
{
}

_CodeModelItem::~_CodeModelItem()
{
}

int _CodeModelItem::kind() const
{
    return _M_kind;
}

QStringList _CodeModelItem::qualifiedName() const
{
    QStringList q = scope();

    if (!name().isEmpty())
        q += name();

    return q;
}

QString _CodeModelItem::name() const
{
    return _M_name;
}

void _CodeModelItem::setName(const QString &name)
{
    _M_name = name;
}

QStringList _CodeModelItem::scope() const
{
    return _M_scope;
}

void _CodeModelItem::setScope(const QStringList &scope)
{
    _M_scope = scope;
}

QString _CodeModelItem::fileName() const
{
    return _M_fileName;
}

void _CodeModelItem::setFileName(const QString &fileName)
{
    _M_fileName = fileName;
}

FileModelItem _CodeModelItem::file() const
{
    return model()->findFile(fileName());
}

void _CodeModelItem::getStartPosition(int *line, int *column)
{
    *line = _M_startLine;
    *column = _M_startColumn;
}

void _CodeModelItem::setStartPosition(int line, int column)
{
    _M_startLine = line;
    _M_startColumn = column;
}

void _CodeModelItem::getEndPosition(int *line, int *column)
{
    *line = _M_endLine;
    *column = _M_endColumn;
}

void _CodeModelItem::setEndPosition(int line, int column)
{
    _M_endLine = line;
    _M_endColumn = column;
}

#ifndef QT_NO_DEBUG_STREAM
template <class It>
void formatSequence(QDebug &d, It i1, It i2, const char *separator=", ")
{
    for (It i = i1; i != i2; ++i) {
        if (i != i1)
            d << separator;
        d << *i;
    }
}

void _CodeModelItem::formatKind(QDebug &d, int k)
{
    if ((k & Kind_Variable) == Kind_Variable)
        d << "Variable";
    else if ((k & Kind_TypeDef) == Kind_TypeDef)
        d << "TypeAlias";
    else if ((k & Kind_TemplateParameter) == Kind_TemplateParameter)
        d << "TemplateParameter";
    else if ((k & Kind_FunctionDefinition) == Kind_FunctionDefinition)
        d << "FunctionDefinition";
    else if ((k & Kind_File) == Kind_File)
        d << "File";
    else if ((k & Kind_Enumerator) == Kind_Enumerator)
        d << "Enumerator";
    else if ((k & Kind_Enum) == Kind_Enum)
        d << "Enum";
    else if ((k & Kind_Class) == Kind_Class)
        d << "Class";
    else if ((k & Kind_Argument) == Kind_Argument)
        d << "Argument";
    switch (k & KindMask) {
    case Kind_Function:
        d << "/Function";
        break;
    case Kind_Member:
        d << "/Member";
        break;
    case Kind_Namespace:
        d << "/Namespace";
        break;
    case Kind_Scope:
        d << "/Scope";
        break;
    }
}

void _CodeModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatKind(d, kind());
     d << ", \"" << name() << '"';
     if (!_M_scope.isEmpty()) {
         d << ", scope=";
         formatSequence(d, _M_scope.cbegin(), _M_scope.cend(), "::");
     }
}

QDebug operator<<(QDebug d, const _CodeModelItem *t)
{
    QDebugStateSaver s(d);
    d.noquote();
    d.nospace();
    d << "CodeModelItem(";
    if (t)
        t->formatDebug(d);
    else
        d << '0';
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
_ClassModelItem::~_ClassModelItem()
{
}

QStringList _ClassModelItem::baseClasses() const
{
    return _M_baseClasses;
}

void _ClassModelItem::setBaseClasses(const QStringList &baseClasses)
{
    _M_baseClasses = baseClasses;
}

TemplateParameterList _ClassModelItem::templateParameters() const
{
    return _M_templateParameters;
}

void _ClassModelItem::setTemplateParameters(const TemplateParameterList &templateParameters)
{
    _M_templateParameters = templateParameters;
}

void _ClassModelItem::addBaseClass(const QString &baseClass)
{
    _M_baseClasses.append(baseClass);
}

bool _ClassModelItem::extendsClass(const QString &name) const
{
    return _M_baseClasses.contains(name);
}

void _ClassModelItem::setClassType(CodeModel::ClassType type)
{
    _M_classType = type;
}

CodeModel::ClassType _ClassModelItem::classType() const
{
    return _M_classType;
}

void _ClassModelItem::addPropertyDeclaration(const QString &propertyDeclaration)
{
    _M_propertyDeclarations << propertyDeclaration;
}

#ifndef QT_NO_DEBUG_STREAM
template <class List>
static void formatModelItemList(QDebug &d, const char *prefix, const List &l)
{
    if (const int size = l.size()) {
        d << prefix << '[' << size << "](";
        for (int i = 0; i < size; ++i) {
            if (i)
                d << ", ";
            l.at(i)->formatDebug(d);
        }
        d << ')';
    }
}

void _ClassModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    if (!_M_baseClasses.isEmpty())
        d << ", inherits=" << _M_baseClasses;
    formatModelItemList(d, ", templateParameters=", _M_templateParameters);
    formatScopeItemsDebug(d);
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
FunctionModelItem _ScopeModelItem::declaredFunction(FunctionModelItem item)
{
    FunctionList function_list = findFunctions(item->name());

    foreach(FunctionModelItem fun, function_list) {
        if (fun->isSimilar(item))
            return fun;
    }

    return FunctionModelItem();
}

_ScopeModelItem::~_ScopeModelItem()
{
}

ClassList _ScopeModelItem::classes() const
{
    ClassList result = _M_classes.values();
    qSort(result);
    ClassList::iterator it = std::unique(result.begin(), result.end());
    result.erase(it, result.end());
    return result;
}

TypeDefList _ScopeModelItem::typeDefs() const
{
    return _M_typeDefs.values();
}

VariableList _ScopeModelItem::variables() const
{
    return _M_variables.values();
}

FunctionList _ScopeModelItem::functions() const
{
    return _M_functions.values();
}

void _ScopeModelItem::addEnumsDeclaration(const QString &enumsDeclaration)
{
    _M_enumsDeclarations << enumsDeclaration;
}

FunctionDefinitionList _ScopeModelItem::functionDefinitions() const
{
    return _M_functionDefinitions.values();
}

EnumList _ScopeModelItem::enums() const
{
    EnumList result;
    foreach (const QString& name, _M_enumNames)
        result.append(_M_enums.value(name));
    return result;
}

void _ScopeModelItem::addClass(ClassModelItem item)
{
    QString name = item->name();
    int idx = name.indexOf(QLatin1Char('<'));
    if (idx > 0)
        _M_classes.insert(name.left(idx), item);
    _M_classes.insert(name, item);
}

void _ScopeModelItem::addFunction(FunctionModelItem item)
{
    _M_functions.insert(item->name(), item);
}

void _ScopeModelItem::addFunctionDefinition(FunctionDefinitionModelItem item)
{
    _M_functionDefinitions.insert(item->name(), item);
}

void _ScopeModelItem::addVariable(VariableModelItem item)
{
    _M_variables.insert(item->name(), item);
}

void _ScopeModelItem::addTypeDef(TypeDefModelItem item)
{
    _M_typeDefs.insert(item->name(), item);
}

void _ScopeModelItem::addEnum(EnumModelItem item)
{
    _M_enumNames.removeOne(item->name());
    _M_enums.insert(item->name(), item);
    _M_enumNames.append(item->name());
}

#ifndef QT_NO_DEBUG_STREAM
template <class Hash>
static void formatScopeHash(QDebug &d, const char *prefix, const Hash &h)
{
    typedef typename Hash::ConstIterator HashIterator;
    if (!h.isEmpty()) {
        d << prefix << '[' << h.size() << "](";
        const HashIterator begin = h.begin();
        const HashIterator end = h.end();
        for (HashIterator it = begin; it != end; ++it) { // Omit the names as they are repeated
            if (it != begin)
                d << ", ";
            d << it.value().data();
        }
        d << ')';
    }
}

void _ScopeModelItem::formatScopeItemsDebug(QDebug &d) const
{
    formatScopeHash(d, ", classes=", _M_classes);
    formatScopeHash(d, ", enums=", _M_enums);
    formatScopeHash(d, ", aliases=", _M_typeDefs);
    formatScopeHash(d, ", functionDefs=", _M_functionDefinitions);
    formatScopeHash(d, ", functions=", _M_functions);
}

void  _ScopeModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    formatScopeItemsDebug(d);
}
#endif // !QT_NO_DEBUG_STREAM

ClassModelItem _ScopeModelItem::findClass(const QString &name) const
{
    return _M_classes.value(name);
}

VariableModelItem _ScopeModelItem::findVariable(const QString &name) const
{
    return _M_variables.value(name);
}

TypeDefModelItem _ScopeModelItem::findTypeDef(const QString &name) const
{
    return _M_typeDefs.value(name);
}

EnumModelItem _ScopeModelItem::findEnum(const QString &name) const
{
    return _M_enums.value(name);
}

FunctionList _ScopeModelItem::findFunctions(const QString &name) const
{
    return _M_functions.values(name);
}

FunctionDefinitionList _ScopeModelItem::findFunctionDefinitions(const QString &name) const
{
    return _M_functionDefinitions.values(name);
}

// ---------------------------------------------------------------------------
_NamespaceModelItem::~_NamespaceModelItem()
{
}

NamespaceList _NamespaceModelItem::namespaces() const
{
    return _M_namespaces.values();
}
void _NamespaceModelItem::addNamespace(NamespaceModelItem item)
{
    _M_namespaces.insert(item->name(), item);
}

NamespaceModelItem _NamespaceModelItem::findNamespace(const QString &name) const
{
    return _M_namespaces.value(name);
}

_FileModelItem::~_FileModelItem()
{
}

#ifndef QT_NO_DEBUG_STREAM
void _NamespaceModelItem::formatDebug(QDebug &d) const
{
    _ScopeModelItem::formatDebug(d);
    formatScopeHash(d, ", namespaces=", _M_namespaces);
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
_ArgumentModelItem::~_ArgumentModelItem()
{
}

TypeInfo _ArgumentModelItem::type() const
{
    return _M_type;
}

void _ArgumentModelItem::setType(const TypeInfo &type)
{
    _M_type = type;
}

bool _ArgumentModelItem::defaultValue() const
{
    return _M_defaultValue;
}

void _ArgumentModelItem::setDefaultValue(bool defaultValue)
{
    _M_defaultValue = defaultValue;
}

#ifndef QT_NO_DEBUG_STREAM
void _ArgumentModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    d << ", type=" << _M_type;
    if (_M_defaultValue)
        d << ", defaultValue=\"" << _M_defaultValueExpression << '"';
}
#endif // !QT_NO_DEBUG_STREAM
// ---------------------------------------------------------------------------
_FunctionModelItem::~_FunctionModelItem()
{
}

bool _FunctionModelItem::isSimilar(FunctionModelItem other) const
{
    if (name() != other->name())
        return false;

    if (isConstant() != other->isConstant())
        return false;

    if (isVariadics() != other->isVariadics())
        return false;

    if (arguments().count() != other->arguments().count())
        return false;

    // ### check the template parameters

    for (int i = 0; i < arguments().count(); ++i) {
        ArgumentModelItem arg1 = arguments().at(i);
        ArgumentModelItem arg2 = other->arguments().at(i);

        if (arg1->type() != arg2->type())
            return false;
    }

    return true;
}

ArgumentList _FunctionModelItem::arguments() const
{
    return _M_arguments;
}

void _FunctionModelItem::addArgument(ArgumentModelItem item)
{
    _M_arguments.append(item);
}

CodeModel::FunctionType _FunctionModelItem::functionType() const
{
    return _M_functionType;
}

void _FunctionModelItem::setFunctionType(CodeModel::FunctionType functionType)
{
    _M_functionType = functionType;
}

bool _FunctionModelItem::isVariadics() const
{
    return _M_isVariadics;
}

void _FunctionModelItem::setVariadics(bool isVariadics)
{
    _M_isVariadics = isVariadics;
}

bool _FunctionModelItem::isVirtual() const
{
    return _M_isVirtual;
}

void _FunctionModelItem::setVirtual(bool isVirtual)
{
    _M_isVirtual = isVirtual;
}

bool _FunctionModelItem::isInline() const
{
    return _M_isInline;
}

void _FunctionModelItem::setInline(bool isInline)
{
    _M_isInline = isInline;
}

bool _FunctionModelItem::isExplicit() const
{
    return _M_isExplicit;
}

void _FunctionModelItem::setExplicit(bool isExplicit)
{
    _M_isExplicit = isExplicit;
}

bool _FunctionModelItem::isAbstract() const
{
    return _M_isAbstract;
}

void _FunctionModelItem::setAbstract(bool isAbstract)
{
    _M_isAbstract = isAbstract;
}

// Qt
bool _FunctionModelItem::isInvokable() const
{
    return _M_isInvokable;
}

void _FunctionModelItem::setInvokable(bool isInvokable)
{
    _M_isInvokable = isInvokable;
}

_FunctionDefinitionModelItem::~_FunctionDefinitionModelItem()
{
}

#ifndef QT_NO_DEBUG_STREAM
void _FunctionModelItem::formatDebug(QDebug &d) const
{
    _MemberModelItem::formatDebug(d);
    d << ", type=" << _M_functionType;
    if (_M_isInline)
        d << " [inline]";
    if (_M_isAbstract)
        d << " [abstract]";
    if (_M_isExplicit)
        d << " [explicit]";
    if (_M_isInvokable)
        d << " [invokable]";
    formatModelItemList(d, ", arguments=", _M_arguments);
    if (_M_isVariadics)
        d << ",...";
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
TypeInfo _TypeDefModelItem::type() const
{
    return _M_type;
}

void _TypeDefModelItem::setType(const TypeInfo &type)
{
    _M_type = type;
}

#ifndef QT_NO_DEBUG_STREAM
void _TypeDefModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    d << ", type=" << _M_type;
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
CodeModel::AccessPolicy _EnumModelItem::accessPolicy() const
{
    return _M_accessPolicy;
}

_EnumModelItem::~_EnumModelItem()
{
}

void _EnumModelItem::setAccessPolicy(CodeModel::AccessPolicy accessPolicy)
{
    _M_accessPolicy = accessPolicy;
}

EnumeratorList _EnumModelItem::enumerators() const
{
    return _M_enumerators;
}

void _EnumModelItem::addEnumerator(EnumeratorModelItem item)
{
    _M_enumerators.append(item);
}

bool _EnumModelItem::isAnonymous() const
{
    return _M_anonymous;
}

void _EnumModelItem::setAnonymous(bool anonymous)
{
    _M_anonymous = anonymous;
}

#ifndef QT_NO_DEBUG_STREAM
void _EnumModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    if (_M_anonymous)
         d << " (anonymous)";
    formatModelItemList(d, ", enumerators=", _M_enumerators);
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
_EnumeratorModelItem::~_EnumeratorModelItem()
{
}

QString _EnumeratorModelItem::value() const
{
    return _M_value;
}

void _EnumeratorModelItem::setValue(const QString &value)
{
    _M_value = value;
}

#ifndef QT_NO_DEBUG_STREAM
void _EnumeratorModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    if (!_M_value.isEmpty())
        d << ", value=\"" << _M_value << '"';
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
_TemplateParameterModelItem::~_TemplateParameterModelItem()
{
}

TypeInfo _TemplateParameterModelItem::type() const
{
    return _M_type;
}

void _TemplateParameterModelItem::setType(const TypeInfo &type)
{
    _M_type = type;
}

bool _TemplateParameterModelItem::defaultValue() const
{
    return _M_defaultValue;
}

void _TemplateParameterModelItem::setDefaultValue(bool defaultValue)
{
    _M_defaultValue = defaultValue;
}

#ifndef QT_NO_DEBUG_STREAM
void _TemplateParameterModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    d << ", type=" << _M_type;
    if (_M_defaultValue)
        d << " [defaultValue]";
}
#endif // !QT_NO_DEBUG_STREAM

// ---------------------------------------------------------------------------
TypeInfo _MemberModelItem::type() const
{
    return _M_type;
}

void _MemberModelItem::setType(const TypeInfo &type)
{
    _M_type = type;
}

CodeModel::AccessPolicy _MemberModelItem::accessPolicy() const
{
    return _M_accessPolicy;
}

_MemberModelItem::~_MemberModelItem()
{
}

void _MemberModelItem::setAccessPolicy(CodeModel::AccessPolicy accessPolicy)
{
    _M_accessPolicy = accessPolicy;
}

bool _MemberModelItem::isStatic() const
{
    return _M_isStatic;
}

void _MemberModelItem::setStatic(bool isStatic)
{
    _M_isStatic = isStatic;
}

bool _MemberModelItem::isConstant() const
{
    return _M_isConstant;
}

void _MemberModelItem::setConstant(bool isConstant)
{
    _M_isConstant = isConstant;
}

bool _MemberModelItem::isVolatile() const
{
    return _M_isVolatile;
}

void _MemberModelItem::setVolatile(bool isVolatile)
{
    _M_isVolatile = isVolatile;
}

bool _MemberModelItem::isAuto() const
{
    return _M_isAuto;
}

void _MemberModelItem::setAuto(bool isAuto)
{
    _M_isAuto = isAuto;
}

bool _MemberModelItem::isFriend() const
{
    return _M_isFriend;
}

void _MemberModelItem::setFriend(bool isFriend)
{
    _M_isFriend = isFriend;
}

bool _MemberModelItem::isRegister() const
{
    return _M_isRegister;
}

void _MemberModelItem::setRegister(bool isRegister)
{
    _M_isRegister = isRegister;
}

bool _MemberModelItem::isExtern() const
{
    return _M_isExtern;
}

void _MemberModelItem::setExtern(bool isExtern)
{
    _M_isExtern = isExtern;
}

bool _MemberModelItem::isMutable() const
{
    return _M_isMutable;
}

void _MemberModelItem::setMutable(bool isMutable)
{
    _M_isMutable = isMutable;
}

#ifndef QT_NO_DEBUG_STREAM
void _MemberModelItem::formatDebug(QDebug &d) const
{
    _CodeModelItem::formatDebug(d);
    d << ", type=" << _M_type;
}
#endif // !QT_NO_DEBUG_STREAM

// kate: space-indent on; indent-width 2; replace-tabs on;

