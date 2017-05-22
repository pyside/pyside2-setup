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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "libsamplemacros.h"
#include <list>
#include <utility>
#include "oddbool.h"
#include "complex.h"
#include "objecttype.h"

enum GlobalEnum {
    NoThing,
    FirstThing,
    SecondThing,
    ThirdThing
};

enum GlobalOverloadFuncEnum {
    GlobalOverloadFunc_i,
    GlobalOverloadFunc_d
};

LIBSAMPLE_API void printSomething();
LIBSAMPLE_API int gimmeInt();
LIBSAMPLE_API double gimmeDouble();
LIBSAMPLE_API double multiplyPair(std::pair<double, double> pair);
LIBSAMPLE_API std::list<Complex> gimmeComplexList();
LIBSAMPLE_API Complex sumComplexPair(std::pair<Complex, Complex> cpx_pair);

LIBSAMPLE_API int countCharacters(const char* text);
LIBSAMPLE_API char* makeCString();
LIBSAMPLE_API const char* returnCString();

LIBSAMPLE_API char* returnNullPrimitivePointer();
LIBSAMPLE_API ObjectType* returnNullObjectTypePointer();
LIBSAMPLE_API Event* returnNullValueTypePointer();

// Tests overloading on functions (!methods)
LIBSAMPLE_API GlobalOverloadFuncEnum overloadedFunc(int val);
LIBSAMPLE_API GlobalOverloadFuncEnum overloadedFunc(double val);

LIBSAMPLE_API unsigned int doubleUnsignedInt(unsigned int value);
LIBSAMPLE_API long long doubleLongLong(long long value);
LIBSAMPLE_API unsigned long long doubleUnsignedLongLong(unsigned long long value);
LIBSAMPLE_API short doubleShort(short value);

LIBSAMPLE_API int acceptInt(int x);
LIBSAMPLE_API unsigned int acceptUInt(unsigned int x);
LIBSAMPLE_API long acceptLong(long x);
LIBSAMPLE_API unsigned long acceptULong(unsigned long x);
LIBSAMPLE_API double acceptDouble(double x);

LIBSAMPLE_API int acceptIntReference(int& x);
LIBSAMPLE_API OddBool acceptOddBoolReference(OddBool& x);


class LIBSAMPLE_API ClassWithFunctionPointer
{
public:
    explicit ClassWithFunctionPointer();
    void callFunctionPointer(int dummy, void (*fp)(void *));
    static void doNothing(void *operand);
};

#endif // FUNCTIONS_H
