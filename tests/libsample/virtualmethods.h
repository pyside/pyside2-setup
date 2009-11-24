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

#ifndef VIRTUALMETHODS_H
#define VIRTUALMETHODS_H

#include "point.h"
#include "complex.h"
#include "str.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API VirtualMethods
{
public:
    VirtualMethods() {}
    ~VirtualMethods() {}

    virtual double virtualMethod0(Point pt, int val, Complex cpx, bool b);
    double callVirtualMethod0(Point pt, int val, Complex cpx, bool b)
    {
        return virtualMethod0(pt, val, cpx, b);
    }

    // Binding modification: rename.
    virtual int sum0(int a0, int a1, int a2) { return a0 + a1 + a2; }
    int callSum0(int a0, int a1, int a2) { return sum0(a0, a1, a2); }

    // Binding modification: set default value for the last argument.
    virtual int sum1(int a0, int a1, int a2) { return a0 + a1 + a2; }
    int callSum1(int a0, int a1, int a2) { return sum1(a0, a1, a2); }

    // Binding modification: remove the last argument and set a default value for it.
    virtual int sum2(int a0, int a1, int a2) { return a0 + a1 + a2; }
    int callSum2(int a0, int a1, int a2) { return sum2(a0, a1, a2); }

    // Binding modification: remove the second argument.
    virtual int sum3(int a0, int a1, int a2) { return a0 + a1 + a2; }
    int callSum3(int a0, int a1, int a2) { return sum3(a0, a1, a2); }

    // Binding modification: remove the second argument and set its default
    // value, then inject code on the binding reimplementation of the virtual
    // (with a native inject-code) to sum the value of the removed
    // argument to the first argument before the method is called.
    virtual int sum4(int a0, int a1, int a2) { return a0 + a1 + a2; }
    int callSum4(int a0, int a1, int a2) { return sum4(a0, a1, a2); }

    // Binding modification: prepend a string to the results of a Python override.
    virtual Str name() { return Str("VirtualMethods"); }
    Str callName() { return name(); }

    // Binding modification: code injection that calls the Python override by itself.
    virtual void callMe() {}
    void callCallMe() { callMe(); }

};

class VirtualDtor
{
public:
    VirtualDtor() {}
    virtual ~VirtualDtor() { VirtualDtor::dtor_called++; }

    static int dtorCalled() { return dtor_called; }

private:
    static int dtor_called;
};


#endif // VIRTUALMETHODS_H

