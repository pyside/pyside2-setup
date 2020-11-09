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

#include "abstractmetaenum.h"
#include "typesystem.h"

#include <QtCore/QDebug>

AbstractMetaEnum::AbstractMetaEnum() :
    m_hasQenumsDeclaration(false), m_signed(true)
{
}

AbstractMetaEnum::~AbstractMetaEnum()
{
    qDeleteAll(m_enumValues);
}

template <class String>
AbstractMetaEnumValue *findMatchingEnumValue(const AbstractMetaEnumValueList &list, const String &value)
{
    for (AbstractMetaEnumValue *enumValue : list) {
        if (enumValue->name() == value)
            return enumValue;
    }
    return nullptr;
}

// Find enum values for "enum Enum { e1 }" either for "e1" or "Enum::e1"
AbstractMetaEnumValue *AbstractMetaEnum::findEnumValue(const QString &value) const
{
    if (isAnonymous())
        return findMatchingEnumValue(m_enumValues, value);
    const int sepPos = value.indexOf(QLatin1String("::"));
    if (sepPos == -1)
        return findMatchingEnumValue(m_enumValues, value);
    return name() == QStringView{value}.left(sepPos)
        ? findMatchingEnumValue(m_enumValues, QStringView{value}.right(value.size() - sepPos - 2))
        : nullptr;
}

QString AbstractMetaEnum::name() const
{
    return m_typeEntry->targetLangEntryName();
}

QString AbstractMetaEnum::qualifier() const
{
    return m_typeEntry->targetLangQualifier();
}

QString AbstractMetaEnum::package() const
{
    return m_typeEntry->targetLangPackage();
}

#ifndef QT_NO_DEBUG_STREAM

static void formatMetaEnumValue(QDebug &d, const AbstractMetaEnumValue *v)
{
    const QString &name = v->stringValue();
    if (!name.isEmpty())
        d << name << '=';
    d << v->value();
}

QDebug operator<<(QDebug d, const AbstractMetaEnumValue *v)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaEnumValue(";
    if (v)
        formatMetaEnumValue(d, v);
    else
        d << '0';
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const AbstractMetaEnum *ae)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaEnum(";
    if (ae) {
        d << ae->fullName();
        if (!ae->isSigned())
            d << " (unsigned) ";
        d << '[';
        const AbstractMetaEnumValueList &values = ae->values();
        for (int i = 0, count = values.size(); i < count; ++i) {
            if (i)
                d << ' ';
            formatMetaEnumValue(d, values.at(i));
        }
        d << ']';
    } else {
        d << '0';
    }
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
