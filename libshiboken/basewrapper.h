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

#ifndef BASEWRAPPER_H
#define BASEWRAPPER_H

#include <Python.h>

namespace Shiboken
{

extern "C"
{

struct PyBaseWrapper
{
    PyObject_HEAD
    PyTypeObject* baseWrapperType;
    void* cptr;
    uint hasOwnership : 1;
    uint validCppObject : 1;
};

} // extern "C"

#define PyBaseWrapper_Check(op) PyObject_TypeCheck(op, &PyBaseWrapper_Type)
#define PyBaseWrapper_CheckExact(op) ((op)->ob_type == &PyBaseWrapper_Type)

#define PyBaseWrapper_cptr(pyobj)                   (((Shiboken::PyBaseWrapper*)pyobj)->cptr)
#define PyBaseWrapper_setCptr(pyobj,c)              (((Shiboken::PyBaseWrapper*)pyobj)->cptr = c)
#define PyBaseWrapper_hasOwnership(pyobj)           (((Shiboken::PyBaseWrapper*)pyobj)->hasOwnership)
#define PyBaseWrapper_setOwnership(pyobj,o)         (((Shiboken::PyBaseWrapper*)pyobj)->hasOwnership = o)
#define PyBaseWrapper_validCppObject(pyobj)         (((Shiboken::PyBaseWrapper*)pyobj)->validCppObject)
#define PyBaseWrapper_setValidCppObject(pyobj,v)    (((Shiboken::PyBaseWrapper*)pyobj)->validCppObject = v)

/* The #defines below were taken from Cython-generated code to allow shiboken to be used with python2.5.
 * Maybe not all of these defines are useful to us, time will tell which ones are really needed or not.
 */

#if PY_VERSION_HEX < 0x02060000
#define Py_REFCNT(ob) (((PyObject*)(ob))->ob_refcnt)
#define Py_TYPE(ob)   (((PyObject*)(ob))->ob_type)
#define Py_SIZE(ob)   (((PyVarObject*)(ob))->ob_size)
#define PyVarObject_HEAD_INIT(type, size) \
        PyObject_HEAD_INIT(type) size,
#define PyType_Modified(t)

typedef struct {
    void *buf;
    PyObject *obj;
    Py_ssize_t len;
    Py_ssize_t itemsize;
    int readonly;
    int ndim;
    char *format;
    Py_ssize_t *shape;
    Py_ssize_t *strides;
    Py_ssize_t *suboffsets;
    void *internal;
} Py_buffer;

#define PyBUF_SIMPLE 0
#define PyBUF_WRITABLE 0x0001
#define PyBUF_LOCK 0x0002
#define PyBUF_FORMAT 0x0004
#define PyBUF_ND 0x0008
#define PyBUF_STRIDES (0x0010 | PyBUF_ND)
#define PyBUF_C_CONTIGUOUS (0x0020 | PyBUF_STRIDES)
#define PyBUF_F_CONTIGUOUS (0x0040 | PyBUF_STRIDES)
#define PyBUF_ANY_CONTIGUOUS (0x0080 | PyBUF_STRIDES)
#define PyBUF_INDIRECT (0x0100 | PyBUF_STRIDES)

#endif
#if PY_MAJOR_VERSION < 3
#define __Pyx_BUILTIN_MODULE_NAME "__builtin__"
#else
#define __Pyx_BUILTIN_MODULE_NAME "builtins"
#endif

#if (PY_VERSION_HEX < 0x02060000) || (PY_MAJOR_VERSION >= 3)
#define Py_TPFLAGS_HAVE_NEWBUFFER 0
#endif


PyAPI_FUNC(PyObject*)
PyBaseWrapper_New(PyTypeObject *instanceType, PyTypeObject *baseWrapperType,
                  const void *cptr, uint hasOwnership = 1);

inline bool
cppObjectIsValid(PyBaseWrapper* self)
{
    if (self->validCppObject)
        return true;
    PyErr_SetString(PyExc_RuntimeError, "internal C++ object already deleted.");
    return false;
}

template <typename T>
PyAPI_FUNC(void)
PyBaseWrapper_Dealloc(PyObject* self)
{
    if (PyBaseWrapper_hasOwnership(self)) {
        delete ((T*)PyBaseWrapper_cptr(self));
    }
    Py_TYPE(((PyBaseWrapper*)self))->tp_free((PyObject*)self);
}

} // namespace Shiboken

#endif // BASEWRAPPER_H
