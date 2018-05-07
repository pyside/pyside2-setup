/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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

#ifndef POLYGON_H
#define POLYGON_H

#include <list>
#include "point.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API Polygon
{
public:
    typedef std::list<Point> PointList;

    Polygon() {}
    Polygon(double x, double y);
    Polygon(Point point);
    Polygon(PointList points);
    ~Polygon() {}

    void addPoint(Point point);

    inline const PointList& points() const { return m_points; }

    // This method intentionally receives and returns copies of a Polygon object.
    static Polygon doublePolygonScale(Polygon polygon);

    // This method invalidates the argument to be used for Polygon(Point) implicit conversion.
    static void stealOwnershipFromPython(Point* point);

    // This method invalidates the argument to be used in a call to doublePolygonScale(Polygon).
    static void stealOwnershipFromPython(Polygon* polygon);

private:
    PointList m_points;
};

#endif // POLYGON_H

