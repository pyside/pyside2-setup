/*
 * This file is part of the Shiboken Python Binding Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

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

