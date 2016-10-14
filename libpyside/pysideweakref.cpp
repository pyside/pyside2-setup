/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    0,                         /* tp_doc */
};

static PyObject* CallableObject_call(PyObject* callable_object, PyObject* args, PyObject* kw)
{
    PySideCallableObject* obj = (PySideCallableObject*)(callable_object);
    obj->weakref_func(obj->user_data);

    Py_XDECREF(PyTuple_GET_ITEM(args, 0)); //kill weak ref object
    Py_RETURN_NONE;
}

namespace PySide { namespace WeakRef {

PyObject* create(PyObject* obj, PySideWeakRefFunction func, void* userData)
{
    if (obj == Py_None)
        return 0;

    if (Py_TYPE(&PySideCallableObjectType) == 0)
    {
        Py_TYPE(&PySideCallableObjectType) = &PyType_Type;
        PyType_Ready(&PySideCallableObjectType);
    }

    PySideCallableObject* callable = PyObject_New(PySideCallableObject, &PySideCallableObjectType);
    if (!callable || PyErr_Occurred())
        return 0;

    PyObject* weak = PyWeakref_NewRef(obj, (PyObject*)callable);
    if (!weak || PyErr_Occurred())
        return 0;

    Py_DECREF(callable);

    callable->weakref_func = func;
    callable->user_data = userData;
    return (PyObject*)weak;
}

} }  //namespace

