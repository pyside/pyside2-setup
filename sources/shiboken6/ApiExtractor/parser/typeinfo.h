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
#include <QtCore/QSharedDataPointer>
#include <QtCore/QStringList>

QT_FORWARD_DECLARE_CLASS(QDebug)
QT_FORWARD_DECLARE_CLASS(QTextStream)

class TypeInfoData;

class TypeInfo
{
    friend class TypeParser;
public:
    using Indirections = QList<Indirection>;
    using TypeInfoList = QList<TypeInfo>;

    TypeInfo();
    ~TypeInfo();
    TypeInfo(const TypeInfo &);
    TypeInfo& operator=(const TypeInfo &);
    TypeInfo(TypeInfo &&);
    TypeInfo& operator=(TypeInfo &&);

    static TypeInfo voidType();
    static TypeInfo varArgsType();

    QStringList qualifiedName() const;
    void setQualifiedName(const QStringList &qualified_name);
    void addName(const QString &);

    bool isVoid() const;

    bool isConstant() const;
    void setConstant(bool is);

    bool isVolatile() const;

    void setVolatile(bool is);

    ReferenceType referenceType() const;
    void setReferenceType(ReferenceType r);

    const Indirections &indirectionsV() const;
    void setIndirectionsV(const Indirections &i);
    void addIndirection(Indirection i);

    // "Legacy", rename?
    int indirections() const;

    void setIndirections(int indirections);

    bool isFunctionPointer() const;
    void setFunctionPointer(bool is);

    const QStringList &arrayElements() const;
    void setArrayElements(const QStringList &arrayElements);

    void addArrayElement(const QString &a);

    const TypeInfoList &arguments() const;
    void setArguments(const TypeInfoList &arguments);
    void addArgument(const TypeInfo &arg);

    const TypeInfoList &instantiations() const;
    TypeInfoList &instantiations(); // for parsing only
    void setInstantiations(const TypeInfoList &i);
    void addInstantiation(const TypeInfo &i);
    void clearInstantiations();

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
    QSharedDataPointer<TypeInfoData> d;

    friend class TypeInfoTemplateArgumentHandler;

    static TypeInfo resolveType(CodeModelItem item, TypeInfo const &__type, const ScopeModelItem &__scope);
};

inline bool operator==(const TypeInfo &t1, const TypeInfo &t2)
{ return t1.equals(t2); }

inline bool operator!=(const TypeInfo &t1, const TypeInfo &t2)
{ return !t1.equals(t2); }

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const TypeInfo &t);
#endif

#endif // TYPEINFO_H
