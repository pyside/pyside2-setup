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

#include "injectcode.h"
#include <sstream>

using namespace std;

InjectCode::InjectCode()
{
}

InjectCode::~InjectCode()
{
}

template<typename T>
const char* InjectCode::toStr(const T& value)
{
    std::ostringstream s;
    s << value;
    m_valueHolder = s.str();
    return m_valueHolder.c_str();
}

const char* InjectCode::simpleMethod1(int arg0, int arg1)
{
    return toStr(arg0 + arg1);
}

const char* InjectCode::simpleMethod2()
{
    return "_";
}

const char* InjectCode::simpleMethod3(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i)
        m_valueHolder += argv[i];
    return m_valueHolder.c_str();
}

const char* InjectCode::overloadedMethod(int arg0, bool arg1)
{
    toStr(arg0);
    m_valueHolder += arg1 ? "true" : "false";
    return m_valueHolder.c_str();
}

const char* InjectCode::overloadedMethod(int arg0, double arg1)
{
    return toStr(arg0 + arg1);
}

const char* InjectCode::overloadedMethod(int argc, char** argv)
{
    return simpleMethod3(argc, argv);
}

const char* InjectCode::virtualMethod(int arg)
{
    return toStr(arg);
}

int InjectCode::arrayMethod(int count, int *values) const
{
    int ret = 0;
    for (int i=0; i < count; i++)
        ret += values[i];
    return ret;
}

int InjectCode::sumArrayAndLength(int* values) const
{
    int sum = 0;

    while(*values) {
        sum = sum + *values + 1;
        values++;
    }

    return sum;
}
