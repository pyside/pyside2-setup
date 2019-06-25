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

#include <iostream>
#include <cstdlib>
#include <time.h>
#include "samplenamespace.h"

using namespace std;

namespace SampleNamespace
{

// PYSIDE-817, scoped enums must not be converted to int in the wrappers generated
// for the protected hacks
SomeClass::PublicScopedEnum SomeClass::protectedMethodReturningPublicScopedEnum() const
{
    return PublicScopedEnum::v1;
}

OutValue
enumInEnumOut(InValue in)
{
    OutValue retval;
    switch(in) {
        case ZeroIn:
            retval = ZeroOut;
            break;
        case OneIn:
            retval = OneOut;
            break;
        case TwoIn:
            retval = TwoOut;
            break;
        default:
            retval = (OutValue) -1;
    }
    return retval;
}

Option
enumArgumentWithDefaultValue(Option opt)
{
    return opt;
}

int
getNumber(Option opt)
{
    int retval;
    switch(opt) {
        case RandomNumber:
            retval = rand() % 100;
            break;
        case UnixTime:
            retval = (int) time(nullptr);
            break;
        default:
            retval = 0;
    }
    return retval;
}

void
doSomethingWithArray(const unsigned char* data, unsigned int size, const char* format)
{
    // This function does nothing in fact.
    // It is here as a dummy copy of QPixmap.loadFromData method
    // to check compilation issues, i.e. if it compiles, it's ok.
}

int
enumItemAsDefaultValueToIntArgument(int value)
{
    return value;
}

void
forceDecisorSideA(ObjectType* object)
{
}

void
forceDecisorSideA(const Point& pt, const Str& text, ObjectType* object)
{
}

void
forceDecisorSideB(int a, ObjectType* object)
{
}

void
forceDecisorSideB(int a, const Point& pt, const Str& text, ObjectType* object)
{
}

double
passReferenceToValueType(const Point& point, double multiplier)
{
    return (point.x() + point.y()) * multiplier;
}

int
passReferenceToObjectType(const ObjectType& obj, int multiplier)
{
    return obj.objectName().size() * multiplier;
}

} // namespace SampleNamespace
