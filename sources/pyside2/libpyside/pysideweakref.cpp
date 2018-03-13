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
#include <shiboken.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    PySideWeakRefFunction weakref_func;
    void*  user_data;
} PySideCallableObject;

static PyObject* CallableObject_call(PyObject* callable_object, PyObject* args, PyObject* kw);

static PyType_Slot PySideCallableObjectType_slots[] = {
    {Py_tp_call, (void *)CallableObject_call},
    {Py_tp_dealloc, (void *)SbkDummyDealloc},
    {0, 0}
};
static PyType_Spec PySideCallableObjectType_spec = {
    const_cast<char*>("PySide.Callable"),
    sizeof(PySideCallableObject),
    0,
    Py_TPFLAGS_DEFAULT,
    PySideCallableObjectType_slots,
};


static PyTypeObject *PySideCallableObjectTypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (type == nullptr)
        type = (PyTypeObject *)PyType_FromSpec(&PySideCallableObjectType_spec);
    return type;
}

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

    if (Py_TYPE(PySideCallableObjectTypeF()) == 0)
    {
        Py_TYPE(PySideCallableObjectTypeF()) = &PyType_Type;
        PyType_Ready(PySideCallableObjectTypeF());
    }

    PySideCallableObject* callable = PyObject_New(PySideCallableObject, PySideCallableObjectTypeF());
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

