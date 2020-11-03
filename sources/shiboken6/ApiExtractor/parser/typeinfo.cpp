/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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


#include "typeinfo.h"
#include "codemodel.h"

#include <clangparser/clangutils.h>

#include <QtCore/QDebug>
#include <QtCore/QStack>
#include <QtCore/QTextStream>

#include <iostream>

class TypeInfoData : public QSharedData
{

public:
    TypeInfoData();

    bool isVoid() const;
    bool equals(const TypeInfoData &other) const;
    bool isStdType() const;
    void simplifyStdType();

    QStringList m_qualifiedName;
    QStringList m_arrayElements;
    TypeInfo::TypeInfoList m_arguments;
    TypeInfo::TypeInfoList m_instantiations;
    TypeInfo::Indirections m_indirections;

    union {
        uint flags;

        struct {
            uint m_constant: 1;
            uint m_volatile: 1;
            uint m_functionPointer: 1;
            uint m_padding: 29;
        };
    };

    ReferenceType m_referenceType;
};

TypeInfoData::TypeInfoData() : flags(0), m_referenceType(NoReference)
{
}

TypeInfo::TypeInfo() : d(new TypeInfoData)
{
}

TypeInfo::~TypeInfo() = default;
TypeInfo::TypeInfo(const TypeInfo &) = default;
TypeInfo& TypeInfo::operator=(const TypeInfo &) = default;
TypeInfo::TypeInfo(TypeInfo &&) = default;
TypeInfo& TypeInfo::operator=(TypeInfo &&) = default;


static inline TypeInfo createType(const QString &name)
{
    TypeInfo result;
    result.addName(name);
    return result;
}

TypeInfo TypeInfo::voidType()
{
    static const TypeInfo result = createType(QLatin1String("void"));
    return result;
}

TypeInfo TypeInfo::varArgsType()
{
    static const TypeInfo result = createType(QLatin1String("..."));
    return result;
}

TypeInfo TypeInfo::combine(const TypeInfo &__lhs, const TypeInfo &__rhs)
{
    TypeInfo __result = __lhs;

    __result.setConstant(__result.isConstant() || __rhs.isConstant());
    __result.setVolatile(__result.isVolatile() || __rhs.isVolatile());
    if (__rhs.referenceType() > __result.referenceType())
        __result.setReferenceType(__rhs.referenceType());

    const auto indirections = __rhs.indirectionsV();
    for (auto i : indirections)
        __result.addIndirection(i);

    __result.setArrayElements(__result.arrayElements() + __rhs.arrayElements());

    const auto instantiations = __rhs.instantiations();
    for (const auto &i : instantiations)
        __result.addInstantiation(i);

    return __result;
}

QStringList TypeInfo::qualifiedName() const
{
    return d->m_qualifiedName;
}

void TypeInfo::setQualifiedName(const QStringList &qualified_name)
{
    if (d->m_qualifiedName != qualified_name)
        d->m_qualifiedName = qualified_name;
}

void  TypeInfo::addName(const QString &n)
{
    d->m_qualifiedName.append(n);
}

bool TypeInfoData::isVoid() const
{
    return m_indirections.isEmpty() && m_referenceType == NoReference
        && m_arguments.isEmpty() && m_arrayElements.isEmpty()
        && m_instantiations.isEmpty()
        && m_qualifiedName.size() == 1
        && m_qualifiedName.constFirst() == QLatin1String("void");
}

bool TypeInfo::isVoid() const
{
    return d->isVoid();
}

bool TypeInfo::isConstant() const
{
    return d->m_constant;
}

void TypeInfo::setConstant(bool is)
{
    if (d->m_constant != is)
        d->m_constant = is;
}

bool TypeInfo::isVolatile() const
{
    return d->m_volatile;
}

void TypeInfo::setVolatile(bool is)
{
    if (d->m_volatile != is)
        d->m_volatile = is;
}

ReferenceType TypeInfo::referenceType() const
{
    return d->m_referenceType;
}

void TypeInfo::setReferenceType(ReferenceType r)
{
    if (d->m_referenceType != r)
        d->m_referenceType = r;
}

const TypeInfo::Indirections &TypeInfo::indirectionsV() const
{
    return d->m_indirections;
}

void TypeInfo::setIndirectionsV(const TypeInfo::Indirections &i)
{
    if (d->m_indirections != i)
        d->m_indirections = i;
}

int TypeInfo::indirections() const
{
    return d->m_indirections.size();
}

void TypeInfo::setIndirections(int indirections)
{
    const Indirections newValue(indirections, Indirection::Pointer);
    if (d->m_indirections != newValue)
        d->m_indirections = newValue;
}

void TypeInfo::addIndirection(Indirection i)
{
    d->m_indirections.append(i);
}

bool TypeInfo::isFunctionPointer() const
{
    return d->m_functionPointer;
}

void TypeInfo::setFunctionPointer(bool is)
{
    if (d->m_functionPointer != is)
        d->m_functionPointer = is;
}

const QStringList &TypeInfo::arrayElements() const
{
    return d->m_arrayElements;
}

void TypeInfo::setArrayElements(const QStringList &arrayElements)
{
    if (d->m_arrayElements != arrayElements)
        d->m_arrayElements = arrayElements;
}

void TypeInfo::addArrayElement(const QString &a)
{
    d->m_arrayElements.append(a);
}

const QList<TypeInfo> &TypeInfo::arguments() const
{
    return d->m_arguments;
}

void TypeInfo::setArguments(const QList<TypeInfo> &arguments)
{
    if (d->m_arguments != arguments)
        d->m_arguments = arguments;
}

void TypeInfo::addArgument(const TypeInfo &arg)
{
    d->m_arguments.append(arg);
}

const TypeInfo::TypeInfoList &TypeInfo::instantiations() const
{
    return d->m_instantiations;
}

TypeInfo::TypeInfoList &TypeInfo::instantiations()
{
    return d->m_instantiations;
}

void TypeInfo::setInstantiations(const TypeInfoList &i)
{
    if (d->m_instantiations != i)
        d->m_instantiations = i;
}

void TypeInfo::addInstantiation(const TypeInfo &i)
{
    d->m_instantiations.append(i);
}

void TypeInfo::clearInstantiations()
{
    if (!d->m_instantiations.isEmpty())
        d->m_instantiations.clear();
}

TypeInfo TypeInfo::resolveType(TypeInfo const &__type, const ScopeModelItem &__scope)
{
    CodeModel *__model = __scope->model();
    Q_ASSERT(__model != nullptr);

    return TypeInfo::resolveType(__model->findItem(__type.qualifiedName(), __scope),  __type, __scope);
}

TypeInfo TypeInfo::resolveType(CodeModelItem __item, TypeInfo const &__type, const ScopeModelItem &__scope)
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

    if (TemplateTypeAliasModelItem templateTypeAlias = qSharedPointerDynamicCast<_TemplateTypeAliasModelItem>(__item)) {

        TypeInfo combined = TypeInfo::combine(templateTypeAlias->type(), otherType);
        // For the alias "template<typename T> using QList = QVector<T>" with
        // other="QList<int>", replace the instantiations to obtain "QVector<int>".
        auto aliasInstantiations = templateTypeAlias->type().instantiations();
        const auto &concreteInstantiations = otherType.instantiations();
        const int count = qMin(aliasInstantiations.size(), concreteInstantiations.size());
        for (int i = 0; i < count; ++i)
            aliasInstantiations[i] = concreteInstantiations[i];
        combined.setInstantiations(aliasInstantiations);
        const CodeModelItem nextItem = __scope->model()->findItem(combined.qualifiedName(), __scope);
        if (!nextItem)
            return combined;
        return resolveType(nextItem, combined, __scope);
    }

    return otherType;
}

// Handler for clang::parseTemplateArgumentList() that populates
// TypeInfo::m_instantiations
class TypeInfoTemplateArgumentHandler
{
public:
    explicit TypeInfoTemplateArgumentHandler(TypeInfo *t)
    {
        m_parseStack.append(t);
    }

    void operator()(int level, QStringView name)
    {
        if (level > m_parseStack.size()) {
            Q_ASSERT(!top()->instantiations().isEmpty());
            m_parseStack.push(&top()->instantiations().back());
        }
        while (level < m_parseStack.size())
            m_parseStack.pop();
        TypeInfo instantiation;
        instantiation.setQualifiedName(qualifiedName(name));
        top()->addInstantiation(instantiation);
   }

private:
    TypeInfo *top() const { return m_parseStack.back(); }

    static QStringList qualifiedName(QStringView name)
    {
        QStringList result;
        const auto nameParts = name.split(u"::");
        result.reserve(nameParts.size());
        for (const auto &p : nameParts)
            result.append(p.toString());
        return result;
    }

    QStack<TypeInfo *> m_parseStack;
};

QPair<int, int> TypeInfo::parseTemplateArgumentList(const QString &l, int from)
{
    return clang::parseTemplateArgumentList(l, clang::TemplateArgumentHandler(TypeInfoTemplateArgumentHandler(this)), from);
}

QString TypeInfo::toString() const
{
    QString tmp;
    if (isConstant())
        tmp += QLatin1String("const ");

    if (isVolatile())
        tmp += QLatin1String("volatile ");

    tmp += d->m_qualifiedName.join(QLatin1String("::"));

    if (const int instantiationCount = d->m_instantiations.size()) {
        tmp += QLatin1Char('<');
        for (int i = 0; i < instantiationCount; ++i) {
            if (i)
                tmp += QLatin1String(", ");
            tmp += d->m_instantiations.at(i).toString();
        }
        if (tmp.endsWith(QLatin1Char('>')))
            tmp += QLatin1Char(' ');
        tmp += QLatin1Char('>');
    }

    for (Indirection i : d->m_indirections)
        tmp.append(indirectionKeyword(i));

    switch (referenceType()) {
    case NoReference:
        break;
    case LValueReference:
        tmp += QLatin1Char('&');
        break;
    case RValueReference:
        tmp += QLatin1String("&&");
        break;
    }

    if (isFunctionPointer()) {
        tmp += QLatin1String(" (*)(");
        for (int i = 0; i < d->m_arguments.count(); ++i) {
            if (i != 0)
                tmp += QLatin1String(", ");

            tmp += d->m_arguments.at(i).toString();
        }
        tmp += QLatin1Char(')');
    }

    for (const QString &elt : d->m_arrayElements) {
        tmp += QLatin1Char('[');
        tmp += elt;
        tmp += QLatin1Char(']');
    }

    return tmp;
}

bool TypeInfoData::equals(const TypeInfoData &other) const
{
    if (m_arrayElements.count() != other.m_arrayElements.count())
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
           && (!m_functionPointer || m_arguments == other.m_arguments)
           && m_instantiations == other.m_instantiations;
}

bool TypeInfo::equals(const TypeInfo &other) const
{
    return d.data() == other.d.data() || d->equals(*other.d);
}

QString TypeInfo::indirectionKeyword(Indirection i)
{
    return i == Indirection::Pointer
        ? QStringLiteral("*") : QStringLiteral("*const");
}

static inline QString constQualifier() { return QStringLiteral("const"); }
static inline QString volatileQualifier() { return QStringLiteral("volatile"); }

bool TypeInfo::stripLeadingConst(QString *s)
{
    return stripLeadingQualifier(constQualifier(), s);
}

bool TypeInfo::stripLeadingVolatile(QString *s)
{
    return stripLeadingQualifier(volatileQualifier(), s);
}

bool TypeInfo::stripLeadingQualifier(const QString &qualifier, QString *s)
{
    // "const int x"
    const int qualifierSize = qualifier.size();
    if (s->size() < qualifierSize + 1 || !s->startsWith(qualifier)
        || !s->at(qualifierSize).isSpace()) {
        return false;
    }
    s->remove(0, qualifierSize + 1);
    while (!s->isEmpty() && s->at(0).isSpace())
        s->remove(0, 1);
    return true;
}

// Strip all const/volatile/*/&
void TypeInfo::stripQualifiers(QString *s)
{
    stripLeadingConst(s);
    stripLeadingVolatile(s);
    while (s->endsWith(QLatin1Char('&')) || s->endsWith(QLatin1Char('*'))
        || s->endsWith(QLatin1Char(' '))) {
        s->chop(1);
    }
}

// Helper functionality to simplify a raw standard type as returned by
// clang_getCanonicalType() for g++ standard containers from
// "std::__cxx11::list<int, std::allocator<int> >" or
// "std::__1::list<int, std::allocator<int> >" -> "std::list<int>".

bool TypeInfoData::isStdType() const
{
    return m_qualifiedName.size() > 1
        && m_qualifiedName.constFirst() == QLatin1String("std");
}

bool TypeInfo::isStdType() const
{
    return d->isStdType();
}

static inline bool discardStdType(const QString &name)
{
    return name == QLatin1String("allocator") || name == QLatin1String("less");
}

void TypeInfoData::simplifyStdType()
{
    Q_ASSERT(isStdType());
    if (m_qualifiedName.at(1).startsWith(QLatin1String("__")))
        m_qualifiedName.removeAt(1);
    for (int t = m_instantiations.size() - 1; t >= 0; --t) {
        if (m_instantiations.at(t).isStdType()) {
            if (discardStdType(m_instantiations.at(t).qualifiedName().constLast()))
                m_instantiations.removeAt(t);
            else
                m_instantiations[t].simplifyStdType();
        }
    }
}

void TypeInfo::simplifyStdType()
{
    if (isStdType())
        d->simplifyStdType();
}

void TypeInfo::formatTypeSystemSignature(QTextStream &str) const
{
    if (d->m_constant)
        str << "const ";
    str << d->m_qualifiedName.join(QLatin1String("::"));
    switch (d->m_referenceType) {
    case NoReference:
        break;
    case LValueReference:
        str << '&';
        break;
    case RValueReference:
        str << "&&";
        break;
    }
    for (auto i : d->m_indirections) {
        switch (i) {
        case Indirection::Pointer:
            str << '*';
            break;
        case Indirection::ConstPointer:
            str << "* const";
            break;
        }
    }
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

void TypeInfo::formatDebug(QDebug &debug) const
{
    debug << '"';
    formatSequence(debug, d->m_qualifiedName.begin(), d->m_qualifiedName.end(), "\", \"");
    debug << '"';
    if (d->m_constant)
        debug << ", [const]";
    if (d->m_volatile)
        debug << ", [volatile]";
    if (!d->m_indirections.isEmpty()) {
        debug << ", indirections=";
        for (auto i : d->m_indirections)
            debug << ' ' << TypeInfo::indirectionKeyword(i);
    }
    switch (d->m_referenceType) {
    case NoReference:
        break;
    case LValueReference:
        debug << ", [ref]";
        break;
    case RValueReference:
        debug << ", [rvalref]";
        break;
    }
    if (!d->m_instantiations.isEmpty()) {
        debug << ", template<";
        formatSequence(debug, d->m_instantiations.begin(), d->m_instantiations.end());
        debug << '>';
    }
    if (d->m_functionPointer) {
        debug << ", function ptr(";
        formatSequence(debug, d->m_arguments.begin(), d->m_arguments.end());
        debug << ')';
    }
    if (!d->m_arrayElements.isEmpty()) {
        debug << ", array[" << d->m_arrayElements.size() << "][";
        formatSequence(debug, d->m_arrayElements.begin(), d->m_arrayElements.end());
        debug << ']';
    }
}

QDebug operator<<(QDebug d, const TypeInfo &t)
{
    QDebugStateSaver s(d);
    const int verbosity = d.verbosity();
    d.noquote();
    d.nospace();
    d << "TypeInfo(";
    if (verbosity > 2)
        t.formatDebug(d);
    else
        d << t.toString();
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
