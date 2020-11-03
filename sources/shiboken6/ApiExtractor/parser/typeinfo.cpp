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

TypeInfo TypeInfo::combine(const TypeInfo &__lhs, const TypeInfo &__rhs)
{
    TypeInfo __result = __lhs;

    __result.setConstant(__result.isConstant() || __rhs.isConstant());
    __result.setVolatile(__result.isVolatile() || __rhs.isVolatile());
    if (__rhs.referenceType() > __result.referenceType())
        __result.setReferenceType(__rhs.referenceType());
    __result.m_indirections.append(__rhs.m_indirections);
    __result.setArrayElements(__result.arrayElements() + __rhs.arrayElements());
    __result.m_instantiations.append(__rhs.m_instantiations);

    return __result;
}

bool TypeInfo::isVoid() const
{
    return m_indirections.isEmpty() && m_referenceType == NoReference
        && m_arguments.isEmpty() && m_arrayElements.isEmpty()
        && m_instantiations.isEmpty()
        && m_qualifiedName.size() == 1
        && m_qualifiedName.constFirst() == QLatin1String("void");
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
            Q_ASSERT(!top()->m_instantiations.isEmpty());
            m_parseStack.push(&top()->m_instantiations.back());
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

    tmp += m_qualifiedName.join(QLatin1String("::"));

    if (const int instantiationCount = m_instantiations.size()) {
        tmp += QLatin1Char('<');
        for (int i = 0; i < instantiationCount; ++i) {
            if (i)
                tmp += QLatin1String(", ");
            tmp += m_instantiations.at(i).toString();
        }
        if (tmp.endsWith(QLatin1Char('>')))
            tmp += QLatin1Char(' ');
        tmp += QLatin1Char('>');
    }

    for (Indirection i : m_indirections)
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
        for (int i = 0; i < m_arguments.count(); ++i) {
            if (i != 0)
                tmp += QLatin1String(", ");

            tmp += m_arguments.at(i).toString();
        }
        tmp += QLatin1Char(')');
    }

    for (const QString &elt : m_arrayElements) {
        tmp += QLatin1Char('[');
        tmp += elt;
        tmp += QLatin1Char(']');
    }

    return tmp;
}

bool TypeInfo::equals(const TypeInfo &other) const
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
           && (!m_functionPointer || m_arguments == other.m_arguments)
           && m_instantiations == other.m_instantiations;
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

bool TypeInfo::isStdType() const
{
    return m_qualifiedName.size() > 1
        && m_qualifiedName.constFirst() == QLatin1String("std");
}

static inline bool discardStdType(const QString &name)
{
    return name == QLatin1String("allocator") || name == QLatin1String("less");
}

void TypeInfo::simplifyStdType()
{
    if (isStdType()) {
        if (m_qualifiedName.at(1).startsWith(QLatin1String("__")))
            m_qualifiedName.removeAt(1);
        for (int t = m_instantiations.size() - 1; t >= 0; --t) {
            if (m_instantiations.at(t).isStdType()) {
                if (discardStdType(m_instantiations.at(t).m_qualifiedName.constLast()))
                    m_instantiations.removeAt(t);
                else
                    m_instantiations[t].simplifyStdType();
            }
        }
    }
}

void TypeInfo::formatTypeSystemSignature(QTextStream &str) const
{
    if (m_constant)
        str << "const ";
    str << m_qualifiedName.join(QLatin1String("::"));
    switch (m_referenceType) {
    case NoReference:
        break;
    case LValueReference:
        str << '&';
        break;
    case RValueReference:
        str << "&&";
        break;
    }
    for (auto i : m_indirections) {
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

void TypeInfo::formatDebug(QDebug &d) const
{
    d << '"';
    formatSequence(d, m_qualifiedName.begin(), m_qualifiedName.end(), "\", \"");
    d << '"';
    if (m_constant)
        d << ", [const]";
    if (m_volatile)
        d << ", [volatile]";
    if (!m_indirections.isEmpty()) {
        d << ", indirections=";
        for (auto i : m_indirections)
            d << ' ' << TypeInfo::indirectionKeyword(i);
    }
    switch (m_referenceType) {
    case NoReference:
        break;
    case LValueReference:
        d << ", [ref]";
        break;
    case RValueReference:
        d << ", [rvalref]";
        break;
    }
    if (!m_instantiations.isEmpty()) {
        d << ", template<";
        formatSequence(d, m_instantiations.begin(), m_instantiations.end());
        d << '>';
    }
    if (m_functionPointer) {
        d << ", function ptr(";
        formatSequence(d, m_arguments.begin(), m_arguments.end());
        d << ')';
    }
    if (!m_arrayElements.isEmpty()) {
        d << ", array[" << m_arrayElements.size() << "][";
        formatSequence(d, m_arrayElements.begin(), m_arrayElements.end());
        d << ']';
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
