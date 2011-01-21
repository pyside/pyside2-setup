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
#include <cstring>
#include <list>
#include "sbkdbg.h"
#include "autodecref.h"

extern "C"
{

struct SbkEnumObject
{
    PyObject_HEAD
    long ob_ival;
    PyObject* ob_name;
};

#define SBKENUMOBJECT_REPR_STRING   "<enum-item %s.%s (%ld)>"

static PyObject* SbkEnumObject_repr(PyObject* self)
{
    return PyString_FromFormat(SBKENUMOBJECT_REPR_STRING,
                               self->ob_type->tp_name,
                               PyString_AS_STRING(((SbkEnumObject*)self)->ob_name),
                               ((SbkEnumObject*)self)->ob_ival);
}

static int SbkEnumObject_print(PyObject* self, FILE* fp, int)
{
    Py_BEGIN_ALLOW_THREADS
    fprintf(fp, SBKENUMOBJECT_REPR_STRING,
            self->ob_type->tp_name,
            PyString_AS_STRING(((SbkEnumObject*)self)->ob_name),
            ((SbkEnumObject*)self)->ob_ival);
    Py_END_ALLOW_THREADS
    return 0;
}

static PyObject* SbkEnumObject_name(PyObject* self, void*)
{
    Py_INCREF(((SbkEnumObject*)self)->ob_name);
    return ((SbkEnumObject*)self)->ob_name;
}

static PyObject* SbkEnum_tp_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    int itemValue = 0;
    if (!PyArg_ParseTuple(args, "|i:__new__", &itemValue))
        return 0;

    SbkEnumObject* self = PyObject_New(SbkEnumObject, type);
    if (!self)
        return 0;
    self->ob_ival = itemValue;
    return reinterpret_cast<PyObject*>(self);
}

static PyGetSetDef SbkEnumGetSetList[] = {
    {const_cast<char*>("name"), &SbkEnumObject_name},
    {0}  // Sentinel
};

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

} // extern "C"

namespace Shiboken {

class DeclaredEnumTypes
{
public:
    DeclaredEnumTypes();
    ~DeclaredEnumTypes();
    static DeclaredEnumTypes& instance();
    void addEnumType(PyTypeObject* type);
private:
    DeclaredEnumTypes(const DeclaredEnumTypes&);
    DeclaredEnumTypes& operator=(const DeclaredEnumTypes&);
    std::list<PyTypeObject*> m_enumTypes;
};

namespace Enum {

PyObject* newItem(PyTypeObject* enumType, long itemValue, const char* itemName)
{
    if (!itemName)
        itemName = "";
    PyObject* pyItemName = PyString_FromString(itemName);

    SbkEnumObject* enumObj = PyObject_New(SbkEnumObject, enumType);
    if (!enumObj) {
        Py_XDECREF(pyItemName);
        return 0;
    }

    enumObj->ob_name = pyItemName;
    enumObj->ob_ival = itemValue;
    if (itemName) {
        PyObject* values = PyDict_GetItemString(enumType->tp_dict, const_cast<char*>("values"));
        if (!values) {
            values = PyDict_New();
            PyDict_SetItemString(enumType->tp_dict, const_cast<char*>("values"), values);
            Py_DECREF(values); // ^ values still alive, because setitemstring incref it
        }
        PyDict_SetItemString(values, itemName, reinterpret_cast<PyObject*>(enumObj));
    }

    return reinterpret_cast<PyObject*>(enumObj);
}

PyTypeObject* newType(const char* name)
{
    PyTypeObject* type = new PyTypeObject;
    ::memset(type, 0, sizeof(PyTypeObject));
    type->ob_type = &SbkEnumType_Type;
    type->tp_basicsize = sizeof(SbkEnumObject);
    type->tp_print = &SbkEnumObject_print;
    type->tp_repr = &SbkEnumObject_repr;
    type->tp_str = &SbkEnumObject_repr;
    type->tp_flags = Py_TPFLAGS_DEFAULT;
    type->tp_base = &PyInt_Type;
    type->tp_name = name;
    type->tp_getset = SbkEnumGetSetList;
    type->tp_new = SbkEnum_tp_new;

    DeclaredEnumTypes::instance().addEnumType(type);
    return type;
}

long int getValue(PyObject* enumItem)
{
    return reinterpret_cast<SbkEnumObject*>(enumItem)->ob_ival;
}

} // namespace Enum

DeclaredEnumTypes& DeclaredEnumTypes::instance()
{
    static DeclaredEnumTypes me;
    return me;
}

DeclaredEnumTypes::DeclaredEnumTypes()
{
}

DeclaredEnumTypes::~DeclaredEnumTypes()
{
    std::list<PyTypeObject*>::const_iterator it = m_enumTypes.begin();
    for (; it != m_enumTypes.end(); ++it)
        delete *it;
    m_enumTypes.clear();
}

void DeclaredEnumTypes::addEnumType(PyTypeObject* type)
{
    m_enumTypes.push_back(type);
}

}
