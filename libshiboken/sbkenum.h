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

namespace Shiboken
{

extern "C"
{

typedef struct {
    PyObject_HEAD
    long ob_ival;
    PyObject* ob_name;
} SbkEnumObject;

extern LIBSHIBOKEN_API PyTypeObject SbkEnumType_Type;

LIBSHIBOKEN_API PyObject* SbkEnumObject_repr(PyObject* self);
LIBSHIBOKEN_API PyObject* SbkEnumObject_name(PyObject* self);

} // extern "C"

inline bool isShibokenEnum(PyObject* pyObj)
{
    return pyObj->ob_type->ob_type == &SbkEnumType_Type;
}

LIBSHIBOKEN_API PyObject* SbkEnumObject_New(PyTypeObject *instanceType,
                           long item_value,
                           const char* item_name);
LIBSHIBOKEN_API PyObject* SbkEnumObject_New(PyTypeObject *instanceType,
                           long item_value,
                           PyObject* item_name = 0);

} // namespace Shiboken

#endif // SKB_PYENUM_H

