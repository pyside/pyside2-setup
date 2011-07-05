/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SBKENUM_H
#define SBKENUM_H

#include <Python.h>
#include "shibokenmacros.h"

extern "C"
{

extern LIBSHIBOKEN_API PyTypeObject SbkEnumType_Type;

} // extern "C"

namespace Shiboken
{

inline bool isShibokenEnum(PyObject* pyObj)
{
    return pyObj->ob_type->ob_type == &SbkEnumType_Type;
}

namespace Enum
{
    LIBSHIBOKEN_API PyObject* newItem(PyTypeObject* enumType, long itemValue, const char* itemName = 0);

    LIBSHIBOKEN_API PyTypeObject* newType(const char* name); //Deprecated use 'newTypeWithName'
    LIBSHIBOKEN_API PyTypeObject* newTypeWithName(const char* name, const char* cppName);
    LIBSHIBOKEN_API const char* getCppName(PyTypeObject* type);

    LIBSHIBOKEN_API long getValue(PyObject* enumItem);
    LIBSHIBOKEN_API PyObject* getEnumItemFromValue(PyTypeObject* enumType, long itemValue);
}

} // namespace Shiboken

#endif // SKB_PYENUM_H

