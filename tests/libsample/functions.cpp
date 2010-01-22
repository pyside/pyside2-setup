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

#include "functions.h"
#include <string.h>
#include <iostream>

using namespace std;

void
printSomething()
{
    cout << __FUNCTION__ << endl;
}

int
gimmeInt()
{
    static int val = 2;
    val = val * 1.3;
    return val;
}

double
gimmeDouble()
{
    static double val = 7.77;
    val = val * 1.3;
    return val;
}

std::list<Complex>
gimmeComplexList()
{
    std::list<Complex> lst;
    lst.push_back(Complex());
    lst.push_back(Complex(1.1, 2.2));
    lst.push_back(Complex(1.3, 2.4));
    return lst;
}

Complex
sumComplexPair(std::pair<Complex, Complex> cpx_pair)
{
    return cpx_pair.first + cpx_pair.second;
}

double
multiplyPair(std::pair<double, double> pair)
{
    return pair.first * pair.second;
}

int
countCharacters(const char* text)
{
    int count;
    for(count = 0; text[count] != '\0'; count++)
        ;
    return count;
}

char*
makeCString()
{
    char* string = new char[strlen(__FUNCTION__) + 1];
    strcpy(string, __FUNCTION__);
    return string;
}

const char*
returnCString()
{
    return __FUNCTION__;
}

GlobalOverloadFuncEnum
overloadedFunc(int val)
{
    return GlobalOverloadFunc_i;
}

GlobalOverloadFuncEnum
overloadedFunc(double val)
{
    return GlobalOverloadFunc_d;
}

char*
returnNullPrimitivePointer()
{
    return 0;
}

ObjectType*
returnNullObjectTypePointer()
{
    return 0;
}

Event*
returnNullValueTypePointer()
{
    return 0;
}

unsigned int
doubleUnsignedInt(unsigned int value)
{
    return value * 2;
}

short
doubleShort(short value)
{
    return value * 2;
}

int
acceptInt(int x)
{
    return x;
}

unsigned int
acceptUInt(unsigned int x)
{
    return x;
}

long
acceptLong(long x)
{
    return x;
}

unsigned long
acceptULong(unsigned long x)
{
    return x;
}

double
acceptDouble(double x)
{
    return x;
}
