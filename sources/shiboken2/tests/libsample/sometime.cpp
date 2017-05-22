/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
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

#include "sometime.h"
#include <stdio.h>

void
Time::setTime()
{
    m_hour = 0;
    m_minute = 0;
    m_second = 0;
    m_msec = 0;
    m_is_null = true;
}

void
Time::setTime(int h, int m, int s, int ms)
{
    m_hour = h;
    m_minute = m;
    m_second = s;
    m_msec = ms;
    m_is_null = false;
}


Time::NumArgs
Time::somethingCompletelyDifferent()
{
    return ZeroArgs;
}

Time::NumArgs
Time::somethingCompletelyDifferent(int h, int m, ImplicitConv ic, ObjectType* type)
{
    if (type)
        return FourArgs;
    if (ic.ctorEnum() == ImplicitConv::CtorThree && ic.objId() == -1)
        return TwoArgs;
    return ThreeArgs;
}

Str
Time::toString() const
{
    if (m_is_null)
        return Str();
    char buffer[13];
    sprintf(buffer, "%02d:%02d:%02d.%03d", m_hour, m_minute, m_second, m_msec);
    return Str(buffer);
}

bool
Time::operator==(const Time& other) const
{
    return m_hour == other.m_hour
            && m_minute == other.m_minute
            && m_second == other.m_second
            && m_msec == other.m_msec
            && m_is_null == other.m_is_null;
}

bool
Time::operator!=(const Time& other) const
{
    return !operator==(other);
}

Time::operator Str() const
{
    return Time::toString();
}

