/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#include <iostream>
#include "pointf.h"

using namespace std;

PointF::PointF(const Point& point) : m_x(point.x()), m_y(point.y())
{
}

PointF::PointF(double x, double y) : m_x(x), m_y(y)
{
}

void
PointF::midpoint(const PointF& other, PointF* midpoint) const
{
    if (!midpoint)
        return;
    midpoint->setX((m_x + other.m_x) / 2.0);
    midpoint->setY((m_y + other.m_y) / 2.0);
}

void
PointF::show()
{
    cout << "(x: " << m_x << ", y: " << m_y << ")";
}

bool
PointF::operator==(const PointF& other)
{
    return m_x == other.m_x && m_y == other.m_y;
}

PointF
PointF::operator+(const PointF& other)
{
    return PointF(m_x + other.m_x, m_y + other.m_y);
}

PointF
PointF::operator-(const PointF& other)
{
    return PointF(m_x - other.m_x, m_y - other.m_y);
}

PointF&
PointF::operator+=(PointF &other)
{
    m_x += other.m_x;
    m_y += other.m_y;
    return *this;
}

PointF&
PointF::operator-=(PointF &other)
{
    m_x -= other.m_x;
    m_y -= other.m_y;
    return *this;
}

PointF
operator*(const PointF& pt, double mult)
{
    return PointF(pt.m_x * mult, pt.m_y * mult);
}

PointF
operator*(const PointF& pt, int mult)
{
    return PointF(((int) pt.m_x) * mult, ((int) pt.m_y) * mult);
}

PointF
operator*(double mult, const PointF& pt)
{
    return PointF(pt.m_x * mult, pt.m_y * mult);
}

PointF
operator*(int mult, const PointF& pt)
{
    return PointF(((int) pt.m_x) * mult, ((int) pt.m_y) * mult);
}

PointF
operator-(const PointF& pt)
{
    return PointF(-pt.m_x, -pt.m_y);
}

bool
operator!(const PointF& pt)
{
    return (pt.m_x == 0.0 && pt.m_y == 0.0);
}

