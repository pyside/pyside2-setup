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

#include "functions.h"
#include <string.h>
#include <algorithm>
#include <iostream>
#include <numeric>

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
    if (!text)
        return -1;
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

long long
doubleLongLong(long long value)
{
    return value * 2;
}

unsigned long long
doubleUnsignedLongLong(unsigned long long value)
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

int
acceptIntReference(int& x)
{
    return x;
}

OddBool
acceptOddBoolReference(OddBool& x)
{
    return x;
}

int sumIntArray(int array[4])
{
    return std::accumulate(array, array + 4, 0);
}

double sumDoubleArray(double array[4])
{
    return std::accumulate(array, array + 4, double(0));
}

ClassWithFunctionPointer::ClassWithFunctionPointer()
{
    callFunctionPointer(0, &ClassWithFunctionPointer::doNothing);
}

void ClassWithFunctionPointer::callFunctionPointer(int dummy, void (*fp)(void *))
{
    size_t a = dummy;
    fp(reinterpret_cast<void *>(a));
}

void ClassWithFunctionPointer::doNothing(void *operand)
{
    (void) operand;
}
