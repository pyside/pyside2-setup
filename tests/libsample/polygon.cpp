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

#include <iostream>
#include "polygon.h"

using namespace std;

Polygon::Polygon(double x, double y)
{
    m_points.push_back(Point(x, y));
}

Polygon::Polygon(Point point)
{
    m_points.push_back(point);
}

Polygon::Polygon(PointList points)
{
    m_points = points;
}

void
Polygon::addPoint(Point point)
{
    m_points.push_back(point);
}

Polygon
Polygon::doublePolygonScale(Polygon polygon)
{
    Polygon result;
    for(PointList::const_iterator piter = result.points().begin(); piter != result.points().end(); piter++)
        result.addPoint((*piter) * 2.0);
    return result;
}

void
Polygon::stealOwnershipFromPython(Point* point)
{
    delete point;
}

void
Polygon::stealOwnershipFromPython(Polygon* polygon)
{
    delete polygon;
}

