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
#include "abstractmetalang.h"
#include "documentation.h"
#include "typesystem.h"
#include "parser/enumvalue.h"

#include <QtCore/QDebug>

class AbstractMetaEnumValueData : public QSharedData
{
public:
    QString m_name;
    QString m_stringValue;
    EnumValue m_value;
    Documentation m_doc;
};

AbstractMetaEnumValue::AbstractMetaEnumValue() :
    d(new AbstractMetaEnumValueData)
{
}

AbstractMetaEnumValue::AbstractMetaEnumValue(const AbstractMetaEnumValue &) = default;
AbstractMetaEnumValue &AbstractMetaEnumValue::operator=(const AbstractMetaEnumValue &) = default;
AbstractMetaEnumValue::AbstractMetaEnumValue(AbstractMetaEnumValue &&) = default;
AbstractMetaEnumValue &AbstractMetaEnumValue::operator=(AbstractMetaEnumValue &&) = default;
AbstractMetaEnumValue::~AbstractMetaEnumValue() = default;

EnumValue AbstractMetaEnumValue::value() const
{
    return d->m_value;
}

void AbstractMetaEnumValue::setValue(EnumValue value)
{
    if (d->m_value != value)
        d->m_value = value;
}

QString AbstractMetaEnumValue::stringValue() const
{
    return d->m_stringValue;
}

void AbstractMetaEnumValue::setStringValue(const QString &v)
{
    if (d->m_stringValue != v)
        d->m_stringValue = v;
}

QString AbstractMetaEnumValue::name() const
{
    return d->m_name;
}

void AbstractMetaEnumValue::setName(const QString &name)
{
    if (d->m_name != name)
        d->m_name = name;
}

Documentation AbstractMetaEnumValue::documentation() const
{
    return d->m_doc;
}

void AbstractMetaEnumValue::setDocumentation(const Documentation &doc)
{
    if (d->m_doc != doc)
        d->m_doc = doc;
}

// --------------------- AbstractMetaEnum

class AbstractMetaEnumData : public QSharedData
{
public:
    AbstractMetaEnumData()  : m_hasQenumsDeclaration(false), m_signed(true)
    {
    }

    AbstractMetaEnumValueList m_enumValues;

    EnumTypeEntry *m_typeEntry = nullptr;
    Documentation m_doc;

    EnumKind m_enumKind = CEnum;
    uint m_hasQenumsDeclaration : 1;
    uint m_signed : 1;
};

AbstractMetaEnum::AbstractMetaEnum() : d(new AbstractMetaEnumData)
{
}

AbstractMetaEnum::AbstractMetaEnum(const AbstractMetaEnum &) = default;
AbstractMetaEnum &AbstractMetaEnum::operator=(const AbstractMetaEnum&) = default;
AbstractMetaEnum::AbstractMetaEnum(AbstractMetaEnum &&) = default;
AbstractMetaEnum &AbstractMetaEnum::operator=(AbstractMetaEnum &&) = default;
AbstractMetaEnum::~AbstractMetaEnum() = default;

const AbstractMetaEnumValueList &AbstractMetaEnum::values() const
{
    return d->m_enumValues;
}

void AbstractMetaEnum::addEnumValue(const AbstractMetaEnumValue &enumValue)
{
    d->m_enumValues << enumValue;
}

std::optional<AbstractMetaEnumValue>
    findMatchingEnumValue(const AbstractMetaEnumValueList &list, QStringView value)
{
    for (const AbstractMetaEnumValue &enumValue : list) {
        if (enumValue.name() == value)
            return enumValue;
    }
    return {};
}

// Find enum values for "enum Enum { e1 }" either for "e1" or "Enum::e1"
std::optional<AbstractMetaEnumValue>
    AbstractMetaEnum::findEnumValue(QStringView value) const
{
    if (isAnonymous())
        return findMatchingEnumValue(d->m_enumValues, value);
    const int sepPos = value.indexOf(QLatin1String("::"));
    if (sepPos == -1)
        return findMatchingEnumValue(d->m_enumValues, value);
    if (name() == value.left(sepPos))
        return findMatchingEnumValue(d->m_enumValues, value.right(value.size() - sepPos - 2));
    return {};
}

QString AbstractMetaEnum::name() const
{
    return d->m_typeEntry->targetLangEntryName();
}

QString AbstractMetaEnum::qualifiedCppName() const
{
    return enclosingClass()
        ? enclosingClass()->qualifiedCppName() + QLatin1String("::") + name()
        : name();
}

const Documentation &AbstractMetaEnum::documentation() const
{
    return d->m_doc;
}

void AbstractMetaEnum::setDocumentation(const Documentation &doc)
{
    if (d->m_doc != doc)
        d->m_doc = doc;
}

QString AbstractMetaEnum::qualifier() const
{
    return d->m_typeEntry->targetLangQualifier();
}

QString AbstractMetaEnum::package() const
{
    return d->m_typeEntry->targetLangPackage();
}

QString AbstractMetaEnum::fullName() const
{
    return package() + QLatin1Char('.') + qualifier()  + QLatin1Char('.') + name();
}

EnumKind AbstractMetaEnum::enumKind() const
{
    return d->m_enumKind;
}

void AbstractMetaEnum::setEnumKind(EnumKind kind)
{
    if (d->m_enumKind != kind)
        d->m_enumKind = kind;
}

bool AbstractMetaEnum::isAnonymous() const
{
    return d->m_enumKind == AnonymousEnum;
}

bool AbstractMetaEnum::hasQEnumsDeclaration() const
{
    return d->m_hasQenumsDeclaration;
}

void AbstractMetaEnum::setHasQEnumsDeclaration(bool on)
{
    if (d->m_hasQenumsDeclaration != on)
        d->m_hasQenumsDeclaration = on;
}

EnumTypeEntry *AbstractMetaEnum::typeEntry() const
{
    return d->m_typeEntry;
}

void AbstractMetaEnum::setTypeEntry(EnumTypeEntry *entry)
{
    if (d->m_typeEntry != entry)
        d->m_typeEntry = entry;
}

bool AbstractMetaEnum::isSigned() const
{
    return d->m_signed;
}

void AbstractMetaEnum::setSigned(bool s)
{
    if (d->m_signed != s)
        d->m_signed = s;
}

#ifndef QT_NO_DEBUG_STREAM

static void formatMetaEnumValue(QDebug &d, const AbstractMetaEnumValue &v)
{
    const QString &name = v.stringValue();
    if (!name.isEmpty())
        d << name << '=';
    d << v.value();
}

QDebug operator<<(QDebug d, const AbstractMetaEnumValue &v)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaEnumValue(";
    formatMetaEnumValue(d, v);
    d << ')';
    return d;
}

static void formatMetaEnum(QDebug &d, const AbstractMetaEnum &e)
{
    d << e.fullName();
    if (!e.isSigned())
        d << " (unsigned) ";
    d << '[';
    const AbstractMetaEnumValueList &values = e.values();
    for (int i = 0, count = values.size(); i < count; ++i) {
        if (i)
            d << ' ';
        formatMetaEnumValue(d, values.at(i));
    }
    d << ']';
}

QDebug operator<<(QDebug d, const AbstractMetaEnum *ae)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaEnum(";
    if (ae)
         formatMetaEnum(d, *ae);
    else
        d << '0';
    d << ')';
    return d;
}

QDebug operator<<(QDebug d, const AbstractMetaEnum &ae)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaEnum(";
    formatMetaEnum(d, ae);
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
