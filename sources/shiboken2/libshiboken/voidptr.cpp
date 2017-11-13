/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#define SbkVoidPtr_Check(op) (Py_TYPE(op) == &SbkVoidPtrType)


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

static PyNumberMethods SbkVoidPtrObjectAsNumber = {
     /* nb_add */                   0,
     /* nb_subtract */              0,
     /* nb_multiply */              0,
#ifndef IS_PY3K
     /* nb_divide */                0,
#endif
     /* nb_remainder */             0,
     /* nb_divmod */                0,
     /* nb_power */                 0,
     /* nb_negative */              0,
     /* nb_positive */              0,
     /* nb_absolute */              0,
     /* nb_bool/nb_nonzero */       0,
     /* nb_invert */                0,
     /* nb_lshift */                0,
     /* nb_rshift */                0,
     /* nb_and */                   0,
     /* nb_xor */                   0,
     /* nb_or */                    0,
#ifndef IS_PY3K
     /* nb_coerce */                0,
#endif
     /* nb_int */                   SbkVoidPtrObject_int,
#ifdef IS_PY3K
     /* nb_reserved */              0,
     /* nb_float */                 0,
#else
     /* nb_long */                  0,
     /* nb_float */                 0,
     /* nb_oct */                   0,
     /* nb_hex */                   0,
#endif

     /* nb_inplace_add */           0,
     /* nb_inplace_subtract */      0,
     /* nb_inplace_multiply */      0,
#ifndef IS_PY3K
     /* nb_inplace_div */           0,
#endif
     /* nb_inplace_remainder */     0,
     /* nb_inplace_power */         0,
     /* nb_inplace_lshift */        0,
     /* nb_inplace_rshift */        0,
     /* nb_inplace_and */           0,
     /* nb_inplace_xor */           0,
     /* nb_inplace_or */            0,

     /* nb_floor_divide */          0,
     /* nb_true_divide */           0,
     /* nb_inplace_floor_divide */  0,
     /* nb_inplace_true_divide */   0,

     /* nb_index */                 0
};

static Py_ssize_t SbkVoidPtrObject_length(PyObject *v)
{
    SbkVoidPtrObject *sbkObject = reinterpret_cast<SbkVoidPtrObject *>(v);
    if (sbkObject->size < 0) {
        PyErr_SetString(PyExc_IndexError, "VoidPtr does not have a size set.");
        return -1;
    }

    return sbkObject->size;
}

static PySequenceMethods SbkVoidPtrObjectAsSequence = {
    /* sq_length */                 SbkVoidPtrObject_length,
    /* sq_concat */                 0,
    /* sq_repeat */                 0,
    /* sq_item */                   0,
    /* sq_slice */                  0,
    /* sq_ass_item */               0,
    /* sq_ass_slice */              0,
    /* sq_contains */               0,
    /* sq_inplace_concat */         0,
    /* sq_inplace_repeat */         0
};

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
PyTypeObject SbkVoidPtrType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)                      /*ob_size*/
    "VoidPtr",                                                  /*tp_name*/
    sizeof(SbkVoidPtrObject),                                   /*tp_basicsize*/
    0,                                                          /*tp_itemsize*/
    0,                                                          /*tp_dealloc*/
    0,                                                          /*tp_print*/
    0,                                                          /*tp_getattr*/
    0,                                                          /*tp_setattr*/
    0,                                                          /*tp_compare*/
    SbkVoidPtrObject_repr,                                      /*tp_repr*/
    &SbkVoidPtrObjectAsNumber,                                  /*tp_as_number*/
    &SbkVoidPtrObjectAsSequence,                                /*tp_as_sequence*/
    0,                                                          /*tp_as_mapping*/
    0,                                                          /*tp_hash */
    0,                                                          /*tp_call*/
    SbkVoidPtrObject_str,                                       /*tp_str*/
    0,                                                          /*tp_getattro*/
    0,                                                          /*tp_setattro*/
    0,                                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                                         /*tp_flags*/
    "Void pointer wrapper",                                     /*tp_doc*/
    0,                                                          /*tp_traverse*/
    0,                                                          /*tp_clear*/
    SbkVoidPtrObject_richcmp,                                   /*tp_richcompare*/
    0,                                                          /*tp_weaklistoffset*/
    0,                                                          /*tp_iter*/
    0,                                                          /*tp_iternext*/
    0,                                                          /*tp_methods*/
    0,                                                          /*tp_members*/
    0,                                                          /*tp_getset*/
    0,                                                          /*tp_base*/
    0,                                                          /*tp_dict*/
    0,                                                          /*tp_descr_get*/
    0,                                                          /*tp_descr_set*/
    0,                                                          /*tp_dictoffset*/
    SbkVoidPtrObject_init,                                      /*tp_init*/
    0,                                                          /*tp_alloc*/
    SbkVoidPtrObject_new,                                       /*tp_new*/
    0,                                                          /*tp_free*/
    0,                                                          /*tp_is_gc*/
    0,                                                          /*tp_bases*/
    0,                                                          /*tp_mro*/
    0,                                                          /*tp_cache*/
    0,                                                          /*tp_subclasses*/
    0,                                                          /*tp_weaklist*/
    0,                                                          /*tp_del*/
    0,                                                          /*tp_version_tag*/
#if PY_MAJOR_VERSION > 3 || PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 4
    0                                                           /*tp_finalize*/
#endif
};

}


namespace VoidPtr {

static int voidPointerInitialized = false;

void init()
{
    if (PyType_Ready(reinterpret_cast<PyTypeObject *>(&SbkVoidPtrType)) < 0)
        Py_FatalError("[libshiboken] Failed to initialize Shiboken.VoidPtr type.");
    else
        voidPointerInitialized = true;
}

void addVoidPtrToModule(PyObject *module)
{
    if (voidPointerInitialized) {
        Py_INCREF(&SbkVoidPtrType);
        PyModule_AddObject(module, SbkVoidPtrType.tp_name,
                           reinterpret_cast<PyObject *>(&SbkVoidPtrType));
    }
}

static PyObject *createVoidPtr(void *cppIn, Py_ssize_t size = 0, bool isWritable = false)
{
    if (!cppIn)
        Py_RETURN_NONE;

    SbkVoidPtrObject *result = PyObject_NEW(SbkVoidPtrObject, &SbkVoidPtrType);
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
    SbkConverter *converter = Shiboken::Conversions::createConverter(&SbkVoidPtrType, toPython);
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


