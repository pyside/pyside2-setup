/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "voidptr.h"
#include "sbkconverter.h"
#include "basewrapper.h"
#include "basewrapper_p.h"

extern "C"
{

// Void pointer object definition.
typedef struct {
    PyObject_HEAD
    void *cptr;
    Py_ssize_t size;
    bool isWritable;
} SbkVoidPtrObject;

PyObject *SbkVoidPtrObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SbkVoidPtrObject *self = reinterpret_cast<SbkVoidPtrObject *>(type->tp_alloc(type, 0));

    if (self != 0) {
        self->cptr = 0;
        self->size = -1;
        self->isWritable = false;
    }

    return reinterpret_cast<PyObject *>(self);
}

#define SbkVoidPtr_Check(op) (Py_TYPE(op) == SbkVoidPtrTypeF())


int SbkVoidPtrObject_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *addressObject;
    Py_ssize_t size = -1;
    int isWritable = 0;
    SbkVoidPtrObject *sbkSelf = reinterpret_cast<SbkVoidPtrObject *>(self);

    static const char *kwlist[] = {"address", "size", "writeable", 0};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ni", const_cast<char **>(kwlist),
                                     &addressObject, &size, &isWritable))
        return -1;

    // Void pointer.
    if (SbkVoidPtr_Check(addressObject)) {
        SbkVoidPtrObject *sbkOther = reinterpret_cast<SbkVoidPtrObject *>(addressObject);
        sbkSelf->cptr = sbkOther->cptr;
        sbkSelf->size = sbkOther->size;
        sbkSelf->isWritable = sbkOther->isWritable;
    }
    // Shiboken::Object wrapper.
    else if (Shiboken::Object::checkType(addressObject)) {
        SbkObject *sbkOther = reinterpret_cast<SbkObject *>(addressObject);
        sbkSelf->cptr = sbkOther->d->cptr[0];
        sbkSelf->size = size;
        sbkSelf->isWritable = isWritable > 0 ? true : false;
    }
    // Python buffer interface.
    else if (PyObject_CheckBuffer(addressObject)) {
        Py_buffer bufferView;

        // Bail out if the object can't provide a simple contiguous buffer.
        if (PyObject_GetBuffer(addressObject, &bufferView, PyBUF_SIMPLE) < 0)
            return 0;

        sbkSelf->cptr = bufferView.buf;
        sbkSelf->size = bufferView.len;
        sbkSelf->isWritable = bufferView.readonly > 0 ? false : true;

        // Release the buffer.
        PyBuffer_Release(&bufferView);
    }
    // An integer representing an address.
    else {
        void *cptr = PyLong_AsVoidPtr(addressObject);
        if (PyErr_Occurred()) {
            PyErr_SetString(PyExc_TypeError,
                            "Creating a VoidPtr object requires an address of a C++ object, "
                            "a wrapped Shiboken Object type, "
                            "an object implementing the Python Buffer interface, "
                            "or another VoidPtr object.");
            return -1;
        }
        sbkSelf->cptr = cptr;
        sbkSelf->size = size;
        sbkSelf->isWritable = isWritable > 0 ? true : false;
    }

    return 0;
}

PyObject *SbkVoidPtrObject_richcmp(PyObject *obj1, PyObject *obj2, int op)
{
    PyObject *result = Py_False;
    void *cptr1 = 0;
    void *cptr2 = 0;
    bool validObjects = true;

    if (SbkVoidPtr_Check(obj1))
        cptr1 = reinterpret_cast<SbkVoidPtrObject *>(obj1)->cptr;
    else
        validObjects = false;

    if (SbkVoidPtr_Check(obj2))
        cptr2 = reinterpret_cast<SbkVoidPtrObject *>(obj2)->cptr;
    else
        validObjects = false;

    if (validObjects) {
        switch (op) {
        case Py_EQ: if (cptr1 == cptr2) result = Py_True; break;
        case Py_NE: if (cptr1 != cptr2) result = Py_True; break;
        case Py_LT: break;
        case Py_LE: break;
        case Py_GT: break;
        case Py_GE: break;
        }
    }

    Py_INCREF(result);
    return result;
}

PyObject *SbkVoidPtrObject_int(PyObject *v)
{
    SbkVoidPtrObject *sbkObject = reinterpret_cast<SbkVoidPtrObject *>(v);
    return PyLong_FromVoidPtr(sbkObject->cptr);
}

static Py_ssize_t SbkVoidPtrObject_length(PyObject *v)
{
    SbkVoidPtrObject *sbkObject = reinterpret_cast<SbkVoidPtrObject *>(v);
    if (sbkObject->size < 0) {
        PyErr_SetString(PyExc_IndexError, "VoidPtr does not have a size set.");
        return -1;
    }

    return sbkObject->size;
}

static const char trueString[] = "True" ;
static const char falseString[] = "False" ;

PyObject *SbkVoidPtrObject_repr(PyObject *v)
{


    SbkVoidPtrObject *sbkObject = reinterpret_cast<SbkVoidPtrObject *>(v);
    PyObject *s = PyBytes_FromFormat("%s(%p, %zd, %s)",
                           Py_TYPE(sbkObject)->tp_name,
                           sbkObject->cptr,
                           sbkObject->size,
                           sbkObject->isWritable ? trueString : falseString);
    Py_XINCREF(s);
    return s;
}

PyObject *SbkVoidPtrObject_str(PyObject *v)
{
    SbkVoidPtrObject *sbkObject = reinterpret_cast<SbkVoidPtrObject *>(v);
    PyObject *s = PyBytes_FromFormat("%s(Address %p, Size %zd, isWritable %s)",
                           Py_TYPE(sbkObject)->tp_name,
                           sbkObject->cptr,
                           sbkObject->size,
                           sbkObject->isWritable ? trueString : falseString);
    Py_XINCREF(s);
    return s;
}


// Void pointer type definition.
static PyType_Slot SbkVoidPtrType_slots[] = {
    {Py_tp_repr, (void *)SbkVoidPtrObject_repr},
    {Py_nb_int, (void *)SbkVoidPtrObject_int},
    {Py_sq_length, (void *)SbkVoidPtrObject_length},
    {Py_tp_str, (void *)SbkVoidPtrObject_str},
    {Py_tp_richcompare, (void *)SbkVoidPtrObject_richcmp},
    {Py_tp_init, (void *)SbkVoidPtrObject_init},
    {Py_tp_new, (void *)SbkVoidPtrObject_new},
    {Py_tp_dealloc, (void *)SbkDummyDealloc},
    {0, 0}
};
static PyType_Spec SbkVoidPtrType_spec = {
    "shiboken2.libshiboken.VoidPtr",
    sizeof(SbkVoidPtrObject),
    0,
    Py_TPFLAGS_DEFAULT,
    SbkVoidPtrType_slots,
};


}

PyTypeObject *SbkVoidPtrTypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (type == nullptr)
        type = (PyTypeObject *)PyType_FromSpec(&SbkVoidPtrType_spec);
    return type;
}

namespace VoidPtr {

static int voidPointerInitialized = false;

void init()
{
    if (PyType_Ready(reinterpret_cast<PyTypeObject *>(SbkVoidPtrTypeF())) < 0)
        Py_FatalError("[libshiboken] Failed to initialize Shiboken.VoidPtr type.");
    else
        voidPointerInitialized = true;
}

void addVoidPtrToModule(PyObject *module)
{
    if (voidPointerInitialized) {
        Py_INCREF(SbkVoidPtrTypeF());
        PyModule_AddObject(module, strrchr(SbkVoidPtrTypeF()->tp_name, '.') + 1,
                           reinterpret_cast<PyObject *>(SbkVoidPtrTypeF()));
    }
}

static PyObject *createVoidPtr(void *cppIn, Py_ssize_t size = 0, bool isWritable = false)
{
    if (!cppIn)
        Py_RETURN_NONE;

    SbkVoidPtrObject *result = PyObject_NEW(SbkVoidPtrObject, SbkVoidPtrTypeF());
    if (!result)
        Py_RETURN_NONE;

    result->cptr = cppIn;
    result->size = size;
    result->isWritable = isWritable;

    return reinterpret_cast<PyObject *>(result);
}

static PyObject *toPython(const void *cppIn)
{
    return createVoidPtr(const_cast<void *>(cppIn));
}

static void VoidPtrToCpp(PyObject *pyIn, void *cppOut)
{
    SbkVoidPtrObject *sbkIn = reinterpret_cast<SbkVoidPtrObject *>(pyIn);
    *reinterpret_cast<void **>(cppOut) = sbkIn->cptr;
}

static PythonToCppFunc VoidPtrToCppIsConvertible(PyObject *pyIn)
{
    return SbkVoidPtr_Check(pyIn) ? VoidPtrToCpp : 0;
}

static void SbkObjectToCpp(PyObject *pyIn, void *cppOut)
{
    SbkObject *sbkIn = reinterpret_cast<SbkObject *>(pyIn);
    *reinterpret_cast<void **>(cppOut) = sbkIn->d->cptr[0];
}

static PythonToCppFunc SbkObjectToCppIsConvertible(PyObject *pyIn)
{
    return Shiboken::Object::checkType(pyIn) ? SbkObjectToCpp : 0;
}

static void PythonBufferToCpp(PyObject *pyIn, void *cppOut)
{
    if (PyObject_CheckBuffer(pyIn)) {
        Py_buffer bufferView;

        // Bail out if the object can't provide a simple contiguous buffer.
        if (PyObject_GetBuffer(pyIn, &bufferView, PyBUF_SIMPLE) < 0)
            return;

        *reinterpret_cast<void **>(cppOut) = bufferView.buf;

        // Release the buffer.
        PyBuffer_Release(&bufferView);
    }
}

static PythonToCppFunc PythonBufferToCppIsConvertible(PyObject *pyIn)
{
    if (PyObject_CheckBuffer(pyIn)) {
        Py_buffer bufferView;

        // Bail out if the object can't provide a simple contiguous buffer.
        if (PyObject_GetBuffer(pyIn, &bufferView, PyBUF_SIMPLE) < 0)
            return 0;

        // Release the buffer.
        PyBuffer_Release(&bufferView);

        return PythonBufferToCpp;
    }
    return 0;
}

SbkConverter *createConverter()
{
    SbkConverter *converter = Shiboken::Conversions::createConverter(SbkVoidPtrTypeF(), toPython);
    Shiboken::Conversions::addPythonToCppValueConversion(converter,
                                                         VoidPtrToCpp,
                                                         VoidPtrToCppIsConvertible);
    Shiboken::Conversions::addPythonToCppValueConversion(converter,
                                                         SbkObjectToCpp,
                                                         SbkObjectToCppIsConvertible);
    Shiboken::Conversions::addPythonToCppValueConversion(converter,
                                                         PythonBufferToCpp,
                                                         PythonBufferToCppIsConvertible);
    return converter;
}

} // namespace VoidPtr


