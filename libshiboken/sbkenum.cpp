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

#include "sbkenum.h"

namespace Shiboken
{

extern "C"
{

PyTypeObject SbkEnumType_Type = {
    PyObject_HEAD_INIT(0)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.EnumType",
    /*tp_basicsize*/        sizeof(PyTypeObject),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          0,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       0,
    /*tp_hash*/             0,
    /*tp_call*/             0,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    /*tp_doc*/              0,
    /*tp_traverse*/         0,
    /*tp_clear*/            0,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          0,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             &PyType_Type,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             0,
    /*tp_alloc*/            0,
    /*tp_new*/              PyType_GenericNew,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0
};

}

PyObject*
SbkEnumObject_New(PyTypeObject *type, long item_value, PyObject* item_name)
{
    if (!item_name)
        item_name = PyString_FromString("");
    SbkEnumObject* enum_obj = PyObject_New(SbkEnumObject, type);
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
            Py_DECREF(values); // ^ values still alive, because setitemstring incref it
        }
        PyDict_SetItemString(values, item_name, enum_obj);
    }

    return enum_obj;
}

extern "C"
{

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

