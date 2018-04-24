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

#include <sbkpython.h>
#include "pysideclassinfo.h"
#include "pysideclassinfo_p.h"
#include "dynamicqmetaobject.h"

#include <shiboken.h>
#include <QDebug>

#define CLASSINFO_CLASS_NAME    "ClassInfo"

extern "C"
{

static PyObject* classInfoTpNew(PyTypeObject* subtype, PyObject* args, PyObject* kwds);
static int classInfoTpInit(PyObject*, PyObject*, PyObject*);
static void classInfoFree(void*);
static PyObject* classCall(PyObject*, PyObject*, PyObject*);

static PyType_Slot PySideClassInfoType_slots[] = {
    {Py_tp_call, (void *)classCall},
    {Py_tp_init, (void *)classInfoTpInit},
    {Py_tp_new, (void *)classInfoTpNew},
    {Py_tp_free, (void *)classInfoFree},
    {Py_tp_dealloc, (void *)SbkDummyDealloc},
    {0, 0}
};
static PyType_Spec PySideClassInfoType_spec = {
    "PySide2.QtCore." CLASSINFO_CLASS_NAME,
    sizeof(PySideClassInfo),
    0,
    Py_TPFLAGS_DEFAULT,
    PySideClassInfoType_slots,
};


PyTypeObject *PySideClassInfoTypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type)
        type = (PyTypeObject *)PyType_FromSpec(&PySideClassInfoType_spec);
    return type;
}

PyObject *classCall(PyObject *self, PyObject *args, PyObject * /* kw */)
{
    if (!PyTuple_Check(args) || PyTuple_Size(args) != 1) {
        PyErr_Format(PyExc_TypeError,
                     "The ClassInfo decorator takes exactly 1 positional argument (%zd given)",
                     PyTuple_Size(args));
        return 0;
    }

    PySideClassInfo* data = reinterpret_cast<PySideClassInfo*>(self);
    PySideClassInfoPrivate* pData = data->d;

    if (pData->m_alreadyWrapped) {
        PyErr_SetString(PyExc_TypeError, "This instance of ClassInfo() was already used to wrap an object");
        return 0;
    }

    PyObject* klass;
    klass = PyTuple_GetItem(args, 0);
    bool validClass = false;

    // This will sometimes segfault if you mistakenly use it on a function declaration
    if (!PyType_Check(klass)) {
        PyErr_SetString(PyExc_TypeError, "This decorator can only be used on class declarations");
        return 0;
    }

    if (Shiboken::ObjectType::checkType(reinterpret_cast<PyTypeObject*>(klass))) {
        PySide::DynamicQMetaObject* mo = reinterpret_cast<PySide::DynamicQMetaObject*>(Shiboken::ObjectType::getTypeUserData(reinterpret_cast<SbkObjectType*>(klass)));
        if (mo) {
            mo->addInfo(PySide::ClassInfo::getMap(data));
            pData->m_alreadyWrapped = true;
            validClass = true;
        }
    }

    if (!validClass) {
        PyErr_SetString(PyExc_TypeError, "This decorator can only be used on classes that are subclasses of QObject");
        return 0;
    }

    Py_INCREF(klass);
    return klass;
}

static PyObject *classInfoTpNew(PyTypeObject *subtype, PyObject * /* args */, PyObject * /* kwds */)
{
    PySideClassInfo* me = reinterpret_cast<PySideClassInfo*>(subtype->tp_alloc(subtype, 0));
    me->d = new PySideClassInfoPrivate;

    me->d->m_alreadyWrapped = false;

    return reinterpret_cast<PyObject *>(me);
}

int classInfoTpInit(PyObject* self, PyObject* args, PyObject* kwds)
{
    if (PyTuple_Check(args) && PyTuple_Size(args) > 0) {
        PyErr_Format(PyExc_TypeError, "ClassInfo() takes exactly 0 positional arguments (%zd given)", PyTuple_Size(args));
        return -1;
    }

    PySideClassInfo* data = reinterpret_cast<PySideClassInfo*>(self);
    PySideClassInfoPrivate* pData = data->d;

    PyObject* key;
    PyObject* value;
    Py_ssize_t pos = 0;

    // PyDict_Next causes a segfault if kwds is empty
    if (kwds && PyDict_Check(kwds) && PyDict_Size(kwds) > 0) {
        while (PyDict_Next(kwds, &pos, &key, &value)) {
            if (Shiboken::String::check(key) && Shiboken::String::check(value)) {
                pData->m_data[Shiboken::String::toCString(key)] = Shiboken::String::toCString(value);
            } else {
                PyErr_SetString(PyExc_TypeError, "All keys and values provided to ClassInfo() must be strings");
                return -1;
            }
        }
    }

    return PyErr_Occurred() ? -1 : 1;
}

void classInfoFree(void *self)
{
    PyObject* pySelf = reinterpret_cast<PyObject*>(self);
    PySideClassInfo* data = reinterpret_cast<PySideClassInfo*>(self);

    delete data->d;
    pySelf->ob_type->tp_base->tp_free(self);
}


} // extern "C"


namespace PySide { namespace ClassInfo {

void init(PyObject* module)
{
    if (PyType_Ready(PySideClassInfoTypeF()) < 0)
        return;

    Py_INCREF(PySideClassInfoTypeF());
    PyModule_AddObject(module, CLASSINFO_CLASS_NAME, reinterpret_cast<PyObject *>(PySideClassInfoTypeF()));
}

bool checkType(PyObject* pyObj)
{
    if (pyObj)
        return PyType_IsSubtype(pyObj->ob_type, PySideClassInfoTypeF());
    return false;
}

QMap<QByteArray, QByteArray> getMap(PySideClassInfo* obj)
{
    return obj->d->m_data;
}

} //namespace Property
} //namespace PySide
