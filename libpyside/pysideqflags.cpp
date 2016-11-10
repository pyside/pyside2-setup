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
        PyHeapTypeObject super;
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
    PyTypeObject* create(const char* name, PyNumberMethods* numberMethods)
    {
        PyTypeObject* type = reinterpret_cast<PyTypeObject*>(new PySideQFlagsType);
        ::memset(type, 0, sizeof(PySideQFlagsType));
        Py_TYPE(type) = &PyType_Type;
        type->tp_basicsize = sizeof(PySideQFlagsObject);
        type->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES;
        type->tp_name = name;
        type->tp_new = &PySideQFlagsNew;
        type->tp_as_number = numberMethods;
        type->tp_richcompare = &PySideQFlagsRichCompare;

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
