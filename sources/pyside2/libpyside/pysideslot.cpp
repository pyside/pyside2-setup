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

#include "dynamicqmetaobject_p.h"
#include "pysidesignal_p.h"
#include "pysideslot_p.h"

#include <shiboken.h>

#include <QtCore/QMetaObject>
#include <QtCore/QString>
#include <signature.h>

struct SlotData
{
    QByteArray name;
    QByteArray args;
    QByteArray resultType;
};

typedef struct
{
    PyObject_HEAD
    SlotData *slotData;
} PySideSlot;

extern "C"
{

static int slotTpInit(PyObject *, PyObject *, PyObject *);
static PyObject *slotCall(PyObject *, PyObject *, PyObject *);

// Class Definition -----------------------------------------------
static PyType_Slot PySideSlotType_slots[] = {
    {Py_tp_call, (void *)slotCall},
    {Py_tp_init, (void *)slotTpInit},
    {Py_tp_new, (void *)PyType_GenericNew},
    {Py_tp_dealloc, (void *)Sbk_object_dealloc},
    {0, 0}
};
static PyType_Spec PySideSlotType_spec = {
    "2:PySide2.QtCore.Slot",
    sizeof(PySideSlot),
    0,
    Py_TPFLAGS_DEFAULT,
    PySideSlotType_slots,
};


static PyTypeObject *PySideSlotTypeF(void)
{
    static PyTypeObject *type = reinterpret_cast<PyTypeObject *>(
        SbkType_FromSpec(&PySideSlotType_spec));
    return type;
}

int slotTpInit(PyObject *self, PyObject *args, PyObject *kw)
{
    static PyObject *emptyTuple = nullptr;
    static const char *kwlist[] = {"name", "result", nullptr};
    char *argName = nullptr;
    PyObject *argResult = nullptr;

    if (emptyTuple == 0)
        emptyTuple = PyTuple_New(0);

    if (!PyArg_ParseTupleAndKeywords(emptyTuple, kw, "|sO:QtCore.Slot",
                                     const_cast<char **>(kwlist), &argName, &argResult)) {
        return -1;
    }

    PySideSlot *data = reinterpret_cast<PySideSlot *>(self);
    if (!data->slotData)
        data->slotData = new SlotData;
    for(Py_ssize_t i = 0, i_max = PyTuple_Size(args); i < i_max; i++) {
        PyObject *argType = PyTuple_GET_ITEM(args, i);
        const auto typeName = PySide::Signal::getTypeName(argType);
        if (typeName.isEmpty()) {
            PyErr_Format(PyExc_TypeError, "Unknown signal argument type: %s", Py_TYPE(argType)->tp_name);
            return -1;
        }
        if (!data->slotData->args.isEmpty())
            data->slotData->args += ',';
        data->slotData->args += typeName;
    }

    if (argName)
        data->slotData->name = argName;

    data->slotData->resultType = argResult
        ? PySide::Signal::getTypeName(argResult) : PySide::Signal::voidType();

    return 0;
}

PyObject *slotCall(PyObject *self, PyObject *args, PyObject * /* kw */)
{
    static PyObject *pySlotName = nullptr;
    PyObject *callback;
    callback = PyTuple_GetItem(args, 0);
    Py_INCREF(callback);

    if (PyFunction_Check(callback)) {
        PySideSlot *data = reinterpret_cast<PySideSlot *>(self);

        if (!data->slotData)
            data->slotData = new SlotData;

        if (data->slotData->name.isEmpty())
            data->slotData->name = Shiboken::String::toCString(PepFunction_GetName(callback));

        const QByteArray returnType = QMetaObject::normalizedType(data->slotData->resultType);
        const QByteArray signature =
            returnType + ' ' + data->slotData->name + '(' + data->slotData->args + ')';

        if (!pySlotName)
            pySlotName = Shiboken::String::fromCString(PYSIDE_SLOT_LIST_ATTR);

        PyObject *pySignature = Shiboken::String::fromCString(signature);
        PyObject *signatureList = 0;
        if (PyObject_HasAttr(callback, pySlotName)) {
            signatureList = PyObject_GetAttr(callback, pySlotName);
        } else {
            signatureList = PyList_New(0);
            PyObject_SetAttr(callback, pySlotName, signatureList);
            Py_DECREF(signatureList);
        }

        PyList_Append(signatureList, pySignature);
        Py_DECREF(pySignature);

        //clear data
        delete data->slotData;
        data->slotData = nullptr;
        return callback;
    }
    return callback;
}

} // extern "C"

namespace PySide {
namespace Slot {

static const char *Slot_SignatureStrings[] = {
    "PySide2.QtCore.Slot(*types:type,name:str=nullptr,result:str=nullptr)->typing.Callable[...,typing.Optional[str]]",
    nullptr}; // Sentinel

void init(PyObject *module)
{
    if (SbkSpecial_Type_Ready(module, PySideSlotTypeF(), Slot_SignatureStrings) < 0)
        return;

    Py_INCREF(PySideSlotTypeF());
    PyModule_AddObject(module, "Slot", reinterpret_cast<PyObject *>(PySideSlotTypeF()));
}

} // namespace Slot
} // namespace PySide
