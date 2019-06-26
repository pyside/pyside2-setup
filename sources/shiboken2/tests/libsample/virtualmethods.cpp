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

#include "virtualmethods.h"

int VirtualDtor::dtor_called = 0;

double
VirtualMethods::virtualMethod0(Point pt, int val, Complex cpx, bool b)
{
    return (pt.x() * pt.y() * val) + cpx.imag() + ((int) b);
}

bool
VirtualMethods::createStr(const char* text, Str*& ret)
{
    if (!text) {
        ret = nullptr;
        return false;
    }

    ret = new Str(text);
    return true;
}

void
VirtualMethods::getMargins(int* left, int* top, int* right, int* bottom) const
{
    *left = m_left;
    *top = m_top;
    *right = m_right;
    *bottom = m_bottom;
}

double VirtualDaughter2::virtualMethod0(Point pt, int val, Complex cpx, bool b)
{
    return 42 + VirtualMethods::virtualMethod0(pt, val, cpx, b);
}

int VirtualDaughter2::sum0(int a0, int a1, int a2)
{
    return 42 + VirtualMethods::sum0(a0, a1, a2);
}

double VirtualFinalDaughter::virtualMethod0(Point pt, int val, Complex cpx, bool b)
{
    return 42 + VirtualMethods::virtualMethod0(pt, val, cpx, b);
}

int VirtualFinalDaughter::sum0(int a0, int a1, int a2)
{
    return 42 + VirtualMethods::sum0(a0, a1, a2);
}
