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

#ifndef VIRTUALMETHODS_H
#define VIRTUALMETHODS_H

#include "point.h"
#include "complex.h"
#include "str.h"

#include "libsamplemacros.h"
#include "strlist.h"

class LIBSAMPLE_API VirtualMethods
{
public:
    VirtualMethods(Str name = "VirtualMethods") : m_name(name)
    {
        m_left = m_top = m_right = m_bottom = 0;
    }
    virtual ~VirtualMethods() {}

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
    virtual Str name() { return m_name; }
    Str callName() { return name(); }

    // Binding modification: code injection that calls the Python override by itself.
    virtual void callMe() {}
    void callCallMe() { callMe(); }

    // Passing reference to pointers.
    virtual bool createStr(const char* text, Str*& ret);
    bool callCreateStr(const char* text, Str*& ret) { return createStr(text, ret); }

    // Return a non-binded method
    std::list<Str> callStrListToStdList(const StrList& strList) { return strListToStdList(strList); }
    virtual std::list<Str> strListToStdList(const StrList& strList ) { return strList; }

    void setMargins(int left, int top, int right, int bottom)
    {
        m_left = left;
        m_top = top;
        m_right = right;
        m_bottom = bottom;
    }
    virtual void getMargins(int* left, int* top, int* right, int* bottom) const;
    void callGetMargins(int* left, int* top, int* right, int* bottom) const
    {
        getMargins(left, top, right, bottom);
    }

    virtual int recursionOnModifiedVirtual(Str arg) const { return 0; }
    int callRecursionOnModifiedVirtual(Str arg) const { return recursionOnModifiedVirtual(arg); }

private:
    Str m_name;
    int m_left;
    int m_top;
    int m_right;
    int m_bottom;
};

class LIBSAMPLE_API VirtualDaughter : public VirtualMethods
{
public:
    VirtualDaughter() : VirtualMethods() {}
    VirtualDaughter(Str name) : VirtualMethods(name) {}
};

class LIBSAMPLE_API VirtualDtor
{
public:
    VirtualDtor() {}
    virtual ~VirtualDtor() { dtor_called++; }

    static VirtualDtor* create() { return new VirtualDtor(); }
    static int dtorCalled() { return dtor_called; }
    static void resetDtorCounter() { dtor_called = 0; }

private:
    static int dtor_called;
};

#endif // VIRTUALMETHODS_H

