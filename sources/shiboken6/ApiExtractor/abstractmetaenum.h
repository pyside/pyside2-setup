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

#ifndef ABSTRACTMETAENUM_H
#define ABSTRACTMETAENUM_H

#include "abstractmetalang_typedefs.h"
#include "abstractmetaattributes.h"
#include "enclosingclassmixin.h"
#include "parser/codemodel_enums.h"
#include "parser/enumvalue.h"

#include <QtCore/QString>

QT_FORWARD_DECLARE_CLASS(QDebug)

class EnumTypeEntry;

class AbstractMetaEnumValue
{
public:
    AbstractMetaEnumValue() = default;

    EnumValue value() const { return m_value; }
    void setValue(EnumValue value) { m_value = value; }

    QString stringValue() const { return m_stringValue; }
    void setStringValue(const QString &v) { m_stringValue = v; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    Documentation documentation() const { return m_doc; }
    void setDocumentation(const Documentation& doc) { m_doc = doc; }

private:
    QString m_name;
    QString m_stringValue;

    EnumValue m_value;

    Documentation m_doc;
};

class AbstractMetaEnum : public AbstractMetaAttributes, public EnclosingClassMixin
{
public:
    AbstractMetaEnum();
    ~AbstractMetaEnum();

    const AbstractMetaEnumValueList &values() const { return m_enumValues; }

    void addEnumValue(AbstractMetaEnumValue *enumValue)
    {
        m_enumValues << enumValue;
    }

    AbstractMetaEnumValue *findEnumValue(const QString &value) const;

    QString name() const;

    QString qualifier() const;

    QString package() const;

    QString fullName() const
    {
        return package() + QLatin1Char('.') + qualifier()  + QLatin1Char('.') + name();
    }

    EnumKind enumKind() const { return m_enumKind; }
    void setEnumKind(EnumKind kind) { m_enumKind = kind; }

    bool isAnonymous() const { return m_enumKind == AnonymousEnum; }

    // Has the enum been declared inside a Q_ENUMS() macro in its enclosing class?
    bool hasQEnumsDeclaration() const { return m_hasQenumsDeclaration; }
    void setHasQEnumsDeclaration(bool on) { m_hasQenumsDeclaration = on; }

    EnumTypeEntry *typeEntry() const { return m_typeEntry; }

    void setTypeEntry(EnumTypeEntry *entry) { m_typeEntry = entry; }

    bool isSigned() const { return m_signed; }
    void setSigned(bool s) { m_signed = s; }

private:
    AbstractMetaEnumValueList m_enumValues;
    EnumTypeEntry *m_typeEntry = nullptr;

    EnumKind m_enumKind = CEnum;
    uint m_hasQenumsDeclaration : 1;
    uint m_signed : 1;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaEnum *ae);
#endif

#endif // ABSTRACTMETAENUM_H
