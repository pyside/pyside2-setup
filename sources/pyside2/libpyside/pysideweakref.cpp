/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "pysideweakref.h"

#include <sbkpython.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    PySideWeakRefFunction weakref_func;
    void*  user_data;
} PySideCallableObject;

static PyObject* CallableObject_call(PyObject* callable_object, PyObject* args, PyObject* kw);

static PyTypeObject PySideCallableObjectType = {
    PyVarObject_HEAD_INIT(0, 0)
    const_cast<char*>("PySide.Callable"),
    sizeof(PySideCallableObject),    /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    CallableObject_call,       /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT/*|Py_TPFLAGS_HEAPTYPE*/,        /*tp_flags*/
    0,                         /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    0,                         /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
    0,                         /* tp_free */
    0,                         /* tp_is_gc */
    0,                         /* tp_bases */
    0,                         /* tp_mro */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0,                         /* tp_weaklist */
    0,                         /* tp_del */
    0                          /* tp_version_tag */
};

static PyTypeObject *PySideCallableObjectTypeP = &PySideCallableObjectType;

static PyObject *CallableObject_call(PyObject *callable_object, PyObject *args, PyObject * /* kw */)
{
    PySideCallableObject* obj = reinterpret_cast<PySideCallableObject *>(callable_object);
    obj->weakref_func(obj->user_data);

    Py_XDECREF(PyTuple_GET_ITEM(args, 0)); //kill weak ref object
    Py_RETURN_NONE;
}

namespace PySide { namespace WeakRef {

PyObject* create(PyObject* obj, PySideWeakRefFunction func, void* userData)
{
    if (obj == Py_None)
        return 0;

    if (Py_TYPE(PySideCallableObjectTypeP) == 0)
    {
        Py_TYPE(PySideCallableObjectTypeP) = &PyType_Type;
        PyType_Ready(PySideCallableObjectTypeP);
    }

    PySideCallableObject* callable = PyObject_New(PySideCallableObject, PySideCallableObjectTypeP);
    if (!callable || PyErr_Occurred())
        return 0;

    PyObject* weak = PyWeakref_NewRef(obj, reinterpret_cast<PyObject *>(callable));
    if (!weak || PyErr_Occurred())
        return 0;

    callable->weakref_func = func;
    callable->user_data = userData;
    Py_DECREF(callable); // PYSIDE-79: after decref the callable is undefined (theoretically)

    return reinterpret_cast<PyObject *>(weak);
}

} }  //namespace

