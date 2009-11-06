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

#ifndef POINT_H
#define POINT_H

#include "complex.h"
#include <utility>

class Point
{
public:
    Point(int x = 0, int y = 0);
    Point(double x, double y);
    ~Point() {}

    double x() const { return m_x; }
    double y() const { return m_y; }

    void setX(double x) { m_x = x; }
    void setY(double y) { m_y = y; }

    Point* copy() const;

    const Point& getConstReferenceToSelf() const { return *this; }
    const Point* getSelf() const { return this; }

    // The != operator is not implemented for the purpose of testing
    // for the absense of the __ne__ method in the Python binding.
    bool operator==(const Point& other);

    Point operator+(const Point& other);
    Point operator-(const Point& other);

    friend Point operator*(const Point& pt, double mult);
    friend Point operator*(const Point& pt, int mult);
    friend Point operator*(double mult, const Point& pt);
    friend Point operator*(int mult, const Point& pt);
    friend Point operator-(const Point& pt);
    friend bool operator!(const Point& pt);

    Point& operator+=(Point &other);
    Point& operator-=(Point &other);

    void show();

private:
    double m_x;
    double m_y;
};

Point operator*(const Point& pt, double mult);
Point operator*(const Point& pt, int mult);
Point operator*(double mult, const Point& pt);
Point operator*(int mult, const Point& pt);
Point operator-(const Point& pt);
bool operator!(const Point& pt);

Complex transmutePointIntoComplex(const Point& point);
Point transmuteComplexIntoPoint(const Complex& cpx);

Point operator*(const Point& pt, double multiplier);

#endif // POINT_H

