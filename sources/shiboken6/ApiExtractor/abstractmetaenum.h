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

#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

#include <optional>

QT_FORWARD_DECLARE_CLASS(QDebug)

class AbstractMetaEnumData;
class AbstractMetaEnumValueData;
class Documentation;
class EnumValue;
class EnumTypeEntry;

class AbstractMetaEnumValue
{
public:
    AbstractMetaEnumValue();
    AbstractMetaEnumValue(const AbstractMetaEnumValue &);
    AbstractMetaEnumValue &operator=(const AbstractMetaEnumValue &);
    AbstractMetaEnumValue(AbstractMetaEnumValue &&);
    AbstractMetaEnumValue &operator=(AbstractMetaEnumValue &&);
    ~AbstractMetaEnumValue();

    EnumValue value() const;
    void setValue(EnumValue value);

    QString stringValue() const;
    void setStringValue(const QString &v);

    QString name() const;
    void setName(const QString &name);

    Documentation documentation() const;
    void setDocumentation(const Documentation& doc);

private:
    QSharedDataPointer<AbstractMetaEnumValueData> d;
};

class AbstractMetaEnum : public AbstractMetaAttributes, public EnclosingClassMixin
{
public:
    AbstractMetaEnum();
    AbstractMetaEnum(const AbstractMetaEnum &);
    AbstractMetaEnum &operator=(const AbstractMetaEnum &);
    AbstractMetaEnum(AbstractMetaEnum &&);
    AbstractMetaEnum &operator=(AbstractMetaEnum &&);
    ~AbstractMetaEnum();

    const AbstractMetaEnumValueList &values() const;
    void addEnumValue(const AbstractMetaEnumValue &enumValue);

    std::optional<AbstractMetaEnumValue> findEnumValue(QStringView value) const;

    QString name() const;
    QString qualifiedCppName() const;

    const Documentation &documentation() const;
    void setDocumentation(const Documentation& doc);

    QString qualifier() const;

    QString package() const;

    QString fullName() const;

    EnumKind enumKind() const;
    void setEnumKind(EnumKind kind);

    bool isAnonymous() const;

    // Has the enum been declared inside a Q_ENUMS() macro in its enclosing class?
    bool hasQEnumsDeclaration() const;
    void setHasQEnumsDeclaration(bool on);

    EnumTypeEntry *typeEntry() const;
    void setTypeEntry(EnumTypeEntry *entry);

    bool isSigned() const;
    void setSigned(bool s);

private:
    QSharedDataPointer<AbstractMetaEnumData> d;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaEnumValue &ae);
QDebug operator<<(QDebug d, const AbstractMetaEnum *ae);
QDebug operator<<(QDebug d, const AbstractMetaEnum &ae);
#endif

#endif // ABSTRACTMETAENUM_H
