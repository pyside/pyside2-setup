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
