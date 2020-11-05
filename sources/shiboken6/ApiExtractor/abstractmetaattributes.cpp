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

#include "abstractmetaattributes.h"

#ifndef QT_NO_DEBUG_STREAM
#  include <QtCore/QDebug>
#  include <QtCore/QMetaEnum>
#  include <QtCore/QMetaObject>
#endif

AbstractMetaAttributes::AbstractMetaAttributes() = default;
AbstractMetaAttributes::~AbstractMetaAttributes() = default;

void AbstractMetaAttributes::assignMetaAttributes(const AbstractMetaAttributes &other)
{
    m_attributes = other.m_attributes;
    m_originalAttributes = other.m_originalAttributes;
    m_doc = other.m_doc;
}

#ifndef QT_NO_DEBUG_STREAM
void AbstractMetaAttributes::formatMetaAttributes(QDebug &d, AbstractMetaAttributes::Attributes value)
{
    static const int meIndex = AbstractMetaAttributes::staticMetaObject.indexOfEnumerator("Attribute");
    Q_ASSERT(meIndex >= 0);
    const QMetaEnum me = AbstractMetaAttributes::staticMetaObject.enumerator(meIndex);
    d << me.valueToKeys(value);
}

QDebug operator<<(QDebug d, const AbstractMetaAttributes *aa)
{
    QDebugStateSaver saver(d);
    d.noquote();
    d.nospace();
    d << "AbstractMetaAttributes(";
    if (aa)
        d << aa->attributes();
    else
        d << '0';
    d << ')';
    return d;
}
#endif // !QT_NO_DEBUG_STREAM
