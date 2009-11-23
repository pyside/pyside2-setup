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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "libsamplemacros.h"
#include <list>
#include <utility>
#include "complex.h"
#include "objecttype.h"

enum LIBSAMPLE_API GlobalEnum {
    NoThing,
    FirstThing,
    SecondThing,
    ThirdThing
};

enum LIBSAMPLE_API GlobalOverloadFuncEnum {
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

#endif // FUNCTIONS_H

