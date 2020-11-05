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

#include "abstractmetafield.h"
#include "abstractmetalang.h"
#include "abstractmetalang_helpers.h"
#include "modifications.h"
#include "typesystem.h"

#include <QtCore/QDebug>

AbstractMetaVariable::AbstractMetaVariable() = default;

AbstractMetaVariable::~AbstractMetaVariable() = default;

void AbstractMetaVariable::assignMetaVariable(const AbstractMetaVariable &other)
{
    m_originalName = other.m_originalName;
    m_name = other.m_name;
    m_type = other.m_type;
    m_hasName = other.m_hasName;
    m_doc = other.m_doc;
}

AbstractMetaField::AbstractMetaField() = default;

AbstractMetaField *AbstractMetaField::copy() const
{
    auto *returned = new AbstractMetaField;
    returned->assignMetaVariable(*this);
    returned->assignMetaAttributes(*this);
    returned->setEnclosingClass(nullptr);
    return returned;
}

AbstractMetaField *AbstractMetaField::find(const AbstractMetaFieldList &haystack,
                                           const QString &needle)
{
    return findByName(haystack, needle);
}

/*******************************************************************************
 * Indicates that this field has a modification that removes it
 */
bool AbstractMetaField::isModifiedRemoved(int types) const
{
    const FieldModificationList &mods = modifications();
    for (const FieldModification &mod : mods) {
        if (!mod.isRemoveModifier())
            continue;

        if ((mod.removal & types) == types)
            return true;
    }

    return false;
}

FieldModificationList AbstractMetaField::modifications() const
{
    const FieldModificationList &mods = enclosingClass()->typeEntry()->fieldModifications();
    FieldModificationList returned;

    for (const FieldModification &mod : mods) {
        if (mod.name == name())
            returned += mod;
    }

    return returned;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const AbstractMetaVariable *av)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaVariable(";
    if (av) {
        d << av->type().name() << ' ' << av->name();
    } else {
        d << '0';
      }
    d << ')';
    return d;
}

void AbstractMetaField::formatDebug(QDebug &d) const
{
    AbstractMetaAttributes::formatMetaAttributes(d, attributes());
    d << ' ' << type().name() << " \"" << name() << '"';
}

QDebug operator<<(QDebug d, const AbstractMetaField *af)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaField(";
    if (af)
        af->formatDebug(d);
    else
        d << '0';
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
