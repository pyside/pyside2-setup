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

#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "codemodel_enums.h"
#include "codemodel_fwd.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

QT_FORWARD_DECLARE_CLASS(QDebug)
QT_FORWARD_DECLARE_CLASS(QTextStream)

class TypeInfo
{
    friend class TypeParser;
public:
    using Indirections = QList<Indirection>;
    using TypeInfoList = QList<TypeInfo>;

    TypeInfo() : flags(0), m_referenceType(NoReference) {}

    QStringList qualifiedName() const { return m_qualifiedName; }
    void setQualifiedName(const QStringList &qualified_name)
    {
        m_qualifiedName = qualified_name;
    }

    bool isVoid() const;

    bool isConstant() const { return m_constant; }
    void setConstant(bool is) { m_constant = is; }

    bool isVolatile() const { return m_volatile; }

    void setVolatile(bool is) { m_volatile = is;    }

    ReferenceType referenceType() const { return m_referenceType; }
    void setReferenceType(ReferenceType r) { m_referenceType = r; }

    const Indirections &indirectionsV() const { return m_indirections; }
    void setIndirectionsV(const Indirections &i) { m_indirections = i; }
    void addIndirection(Indirection i) { m_indirections.append(i); }

    // "Legacy", rename?
    int indirections() const { return m_indirections.size(); }

    void setIndirections(int indirections)
    {
        m_indirections = Indirections(indirections, Indirection::Pointer);
    }

    bool isFunctionPointer() const { return m_functionPointer; }
    void setFunctionPointer(bool is) { m_functionPointer = is; }

    const QStringList &arrayElements() const { return m_arrayElements; }
    void setArrayElements(const QStringList &arrayElements)
    {
        m_arrayElements = arrayElements;
    }

    void addArrayElement(const QString &a) { m_arrayElements.append(a); }

    const TypeInfoList &arguments() const { return m_arguments; }
    void setArguments(const TypeInfoList &arguments);

    void addArgument(const TypeInfo &arg)
    {
        m_arguments.append(arg);
    }

    const TypeInfoList &instantiations() const { return m_instantiations; }
    void setInstantiations(const TypeInfoList &i) { m_instantiations = i; }
    void addInstantiation(const TypeInfo &i) { m_instantiations.append(i); }
    void clearInstantiations() { m_instantiations.clear(); }

    bool isStdType() const;

    QPair<int, int> parseTemplateArgumentList(const QString &l, int from = 0);

    bool equals(const TypeInfo &other) const;

    // ### arrays and templates??

    QString toString() const;

    static TypeInfo combine(const TypeInfo &__lhs, const TypeInfo &__rhs);
    static TypeInfo resolveType(TypeInfo const &__type, const ScopeModelItem &__scope);

    void formatTypeSystemSignature(QTextStream &str) const;

#ifndef QT_NO_DEBUG_STREAM
    void formatDebug(QDebug &d) const;
#endif

    static QString indirectionKeyword(Indirection i);

    static bool stripLeadingConst(QString *s);
    static bool stripLeadingVolatile(QString *s);
    static bool stripLeadingQualifier(const QString &qualifier, QString *s);
    static void stripQualifiers(QString *s);

    void simplifyStdType();

private:
    friend class TypeInfoTemplateArgumentHandler;

    static TypeInfo resolveType(CodeModelItem item, TypeInfo const &__type, const ScopeModelItem &__scope);

    QStringList m_qualifiedName;
    QStringList m_arrayElements;
    QList<TypeInfo> m_arguments;
    QList<TypeInfo> m_instantiations;
    Indirections m_indirections;

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

inline bool operator==(const TypeInfo &t1, const TypeInfo &t2)
{ return t1.equals(t2); }

inline bool operator!=(const TypeInfo &t1, const TypeInfo &t2)
{ return !t1.equals(t2); }

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeInfo &t);
#endif

#endif // TYPEINFO_H
