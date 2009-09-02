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

#ifndef MODIFICATIONS_H
#define MODIFICATIONS_H

#include <utility>
#include "point.h"

class Modifications
{
public:
    Modifications() {}
    ~Modifications() {}

    enum OverloadedModFunc {
        OverloadedNone,
        Overloaded_ibid,
        Overloaded_ibib,
        Overloaded_ibiP,
        Overloaded_ibii,
        Overloaded_ibPP
    };

    // those overloaded methods should be heavily modified
    // to push the overload decisor to its limits
    OverloadedModFunc overloaded(int a0, bool b0, int c0, double d0) { return Overloaded_ibid; }
    OverloadedModFunc overloaded(int a1, bool b1, int c1, bool d1) { return Overloaded_ibib; }
    OverloadedModFunc overloaded(int a2, bool b2, int c2, Point d2) { return Overloaded_ibiP; }
    OverloadedModFunc overloaded(int a3, bool b3, int c3 = 123, int d3 = 456) { return Overloaded_ibii; }
    OverloadedModFunc overloaded(int a4, bool b4, Point c4, Point d4) { return Overloaded_ibPP; }

    // 'ok' must be removed and the return value will be changed
    // to a tuple (PyObject*) containing the expected result plus
    // the 'ok' value as a Python boolean
    std::pair<double, double> pointToPair(Point pt, bool* ok);

    // same as 'pointToPair' except that this time 'ok' is the first argument
    double multiplyPointCoordsPlusValue(bool* ok, Point pt, double value);

    // completely remove 'plus' from the Python side
    int doublePlus(int value, int plus = 0);

    // the default value for both arguments must be changed in Python
    int power(int base = 1, int exponent = 0);

    // in Python set argument default value to 10
    int timesTen(int number);

    // in Python remove the argument default value
    int increment(int number = 0);

    // don't export this method to Python
    void exclusiveCppStuff();

    // change the name of this regular method
    int cppMultiply(int a, int b);

    // change the name of this virtual method
    virtual const char* className();
};

class AbstractModifications : public Modifications
{
public:
    AbstractModifications() {}
    ~AbstractModifications() {}

    bool invert(bool value) { return !value; }

    // completely remove this method in Python
    virtual void pointlessPureVirtualMethod() = 0;
};

#endif // MODIFICATIONS_H

