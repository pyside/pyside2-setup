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

#include "pysideqflags.h"
#include <sbkenum.h>
#include <autodecref.h>

extern "C" {
    struct SbkConverter;

    /**
     * Type of all QFlags
     */
    struct PySideQFlagsType
    {
        PepTypeObject super;
        SbkConverter** converterPtr;
        SbkConverter* converter;
    };

    #define PYSIDE_QFLAGS(X) reinterpret_cast<PySideQFlagsObject*>(X)

    PyObject *PySideQFlagsNew(PyTypeObject *type, PyObject *args, PyObject * /* kwds */)
    {
        long val = 0;
        if (PyTuple_GET_SIZE(args)) {
            PyObject* arg = PyTuple_GET_ITEM(args, 0);
            if (Shiboken::isShibokenEnum(arg)) {// faster call
                val = Shiboken::Enum::getValue(arg);
            } else if (PyNumber_Check(arg)) {
                Shiboken::AutoDecRef number(PyNumber_Long(arg));
                val = PyLong_AsLong(number);
            } else {
                PyErr_SetString(PyExc_TypeError,"QFlags must be created using enums or numbers.");
                return 0;
            }
        }
        PySideQFlagsObject* self = PyObject_New(PySideQFlagsObject, type);
        self->ob_value = val;
        return reinterpret_cast<PyObject*>(self);
    }

    static long getNumberValue(PyObject* v)
    {
        Shiboken::AutoDecRef number(PyNumber_Long(v));
        return PyLong_AsLong(number);
    }

    PyObject* PySideQFlagsRichCompare(PyObject* self, PyObject* other, int op)
    {
        int result = 0;
        if (!PyNumber_Check(other)) {
            PyErr_BadArgument();
            return NULL;
        }

        long valA = PYSIDE_QFLAGS(self)->ob_value;
        long valB = getNumberValue(other);

        if (self == other) {
            result = 1;
        } else  {
            switch (op) {
            case Py_EQ:
                result = (valA == valB);
                break;
            case Py_NE:
                result = (valA != valB);
                break;
            case Py_LE:
                result = (valA <= valB);
                break;
            case Py_GE:
                result = (valA >= valB);
                break;
            case Py_LT:
                result = (valA < valB);
                break;
            case Py_GT:
                result = (valA > valB);
                break;
            default:
                PyErr_BadArgument();
                return NULL;
            }
        }
        if (result)
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }
}

namespace PySide
{
namespace QFlags
{
    static PyType_Slot SbkNewQFlagsType_slots[] = {
#ifdef IS_PY3K
        {Py_nb_bool, 0},
#else
        {Py_nb_nonzero, 0},
        {Py_nb_long, 0},
#endif
        {Py_nb_invert, 0},
        {Py_nb_and, 0},
        {Py_nb_xor, 0},
        {Py_nb_or, 0},
        {Py_nb_int, 0},
#ifndef IS_PY3K
        {Py_nb_long, 0},
#endif
        {Py_tp_new, (void *)PySideQFlagsNew},
        {Py_tp_richcompare, (void *)PySideQFlagsRichCompare},
        {Py_tp_dealloc, (void *)SbkDummyDealloc},
        {0, 0}
    };
    static PyType_Spec SbkNewQFlagsType_spec = {
        "missing QFlags name", // to be inserted later
        sizeof(PySideQFlagsObject),
        0,
        Py_TPFLAGS_DEFAULT|Py_TPFLAGS_CHECKTYPES,
        SbkNewQFlagsType_slots,
    };

    PyTypeObject *create(const char* name, PyType_Slot numberMethods[])
    {
        char qualname[200];
        strcpy(qualname, "PySide2.libpyside.");
        strcat(qualname, name);
        // Careful: PyType_FromSpec does not allocate the string.
        PyType_Spec *newspec = new PyType_Spec;
        newspec->name = strdup(qualname);
        newspec->basicsize = SbkNewQFlagsType_spec.basicsize;
        newspec->itemsize = SbkNewQFlagsType_spec.itemsize;
        newspec->flags = SbkNewQFlagsType_spec.flags;
        int idx = -1;
#ifdef IS_PY3K
#  define SLOT slot
#else
#  define SLOT slot_
#endif
        while (numberMethods[++idx].SLOT) {
            assert(SbkNewQFlagsType_slots[idx].SLOT == numberMethods[idx].SLOT);
            SbkNewQFlagsType_slots[idx].pfunc = numberMethods[idx].pfunc;
        }
        newspec->slots = SbkNewQFlagsType_spec.slots;
        PyTypeObject *type = (PyTypeObject *)PepType_FromSpec(newspec, 2);
        Py_TYPE(type) = &PyType_Type;

        PySideQFlagsType* flagsType = reinterpret_cast<PySideQFlagsType*>(type);
        flagsType->converterPtr = &flagsType->converter;

        if (PyType_Ready(type) < 0)
            return 0;
        return type;
    }

    PySideQFlagsObject* newObject(long value, PyTypeObject* type)
    {
        PySideQFlagsObject* qflags = PyObject_New(PySideQFlagsObject, type);
        qflags->ob_value = value;
        return qflags;
    }

    long getValue(PySideQFlagsObject* self)
    {
        return self->ob_value;
    }
}
}
