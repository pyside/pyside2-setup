/*
 * This file is part of the Shiboken Python Bindings Generator project.
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

#include "pyenum.h"

namespace Shiboken
{

PyObject*
SbkEnumObject_New(PyTypeObject *type, long item_value, PyObject* item_name)
{
    if (!item_name)
        item_name = PyString_FromString("");
    SbkEnumObject* enum_obj = (SbkEnumObject*) type->tp_alloc(type, 0);
    enum_obj->ob_name = item_name;
    enum_obj->ob_ival = item_value;
    return (PyObject*) enum_obj;
}

PyObject*
SbkEnumObject_New(PyTypeObject *type, long item_value, const char* item_name)
{
    PyObject* py_item_name = 0;
    if (item_name)
        py_item_name = PyString_FromString(item_name);

    PyObject* enum_obj = SbkEnumObject_New(type, item_value, py_item_name);
    if (!enum_obj) {
        Py_XDECREF(py_item_name);
        return 0;
    }

    if (item_name) {
        PyObject* values = PyDict_GetItemString(type->tp_dict, const_cast<char*>("values"));
        if (!values) {
            values = PyDict_New();
            PyDict_SetItemString(type->tp_dict, const_cast<char*>("values"), values);
        }
        PyDict_SetItemString(values, item_name, enum_obj);
    }

    return enum_obj;
}

extern "C"
{

PyObject*
SbkEnumObject_NonExtensibleNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyErr_SetString(PyExc_TypeError, "enum is not extensible");
    return 0;
}


PyObject*
SbkEnumObject_repr(PyObject* self)
{
    return PyString_FromFormat("<enum-item %s.%s (%ld)>",
                               self->ob_type->tp_name,
                               PyString_AS_STRING(((SbkEnumObject*)self)->ob_name),
                               ((SbkEnumObject*)self)->ob_ival);
}

PyObject*
SbkEnumObject_name(PyObject* self)
{
    Py_INCREF(((SbkEnumObject*)self)->ob_name);
    return ((SbkEnumObject*)self)->ob_name;
}

} // extern "C"

} // namespace Shiboken

