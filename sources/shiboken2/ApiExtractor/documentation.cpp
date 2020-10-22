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

#include "documentation.h"

Documentation::Documentation(const QString &value, Documentation::Type t, Documentation::Format fmt)
{
    setValue(value, t, fmt);
}

bool Documentation::isEmpty() const
{
    for (int i = 0; i < Type::Last; i++) {
        if (!m_data.value(static_cast<Type>(i)).isEmpty())
            return false;
    }
    return true;
}

QString Documentation::value(Documentation::Type t) const
{
    return m_data.value(t);
}

void Documentation::setValue(const QString &value, Documentation::Type t, Documentation::Format fmt)
{
    const QString v = value.trimmed();
    if (v.isEmpty())
        m_data.remove(t);
    else
        m_data[t] = value.trimmed();
    m_format = fmt;
}

Documentation::Format Documentation::format() const
{
    return m_format;
}

void Documentation::setFormat(Documentation::Format f)
{
    m_format = f;
}

bool Documentation::equals(const Documentation &rhs) const
{
    return m_format == rhs.m_format && m_data == rhs.m_data;
}
