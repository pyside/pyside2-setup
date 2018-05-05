/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef PEP384IMPL_H
#define PEP384IMPL_H

#include "sbkpython.h"

extern "C"
{

/*****************************************************************************
 *
 * RESOLVED: memoryobject.h
 *
 */
#ifdef Py_LIMITED_API
/* buffer interface */
// This has been renamed to Pep_buffer and will be used.
typedef struct bufferinfo {
    void *buf;
    PyObject *obj;        /* owned reference */
    Py_ssize_t len;
    Py_ssize_t itemsize;  /* This is Py_ssize_t so it can be
                             pointed to by strides in simple case.*/
    int readonly;
    int ndim;
    char *format;
    Py_ssize_t *shape;
    Py_ssize_t *strides;
    Py_ssize_t *suboffsets;
    void *internal;
} Pep_buffer;

typedef int (*getbufferproc)(PyObject *, Pep_buffer *, int);
typedef void (*releasebufferproc)(PyObject *, Pep_buffer *);

/* Maximum number of dimensions */
#define PyBUF_MAX_NDIM 64

/* Flags for getting buffers */
#define PyBUF_SIMPLE 0
#define PyBUF_WRITABLE 0x0001
/*  we used to include an E, backwards compatible alias  */
#define PyBUF_WRITEABLE PyBUF_WRITABLE
#define PyBUF_FORMAT 0x0004
#define PyBUF_ND 0x0008
#define PyBUF_STRIDES (0x0010 | PyBUF_ND)
#define PyBUF_C_CONTIGUOUS (0x0020 | PyBUF_STRIDES)
#define PyBUF_F_CONTIGUOUS (0x0040 | PyBUF_STRIDES)
#define PyBUF_ANY_CONTIGUOUS (0x0080 | PyBUF_STRIDES)
#define PyBUF_INDIRECT (0x0100 | PyBUF_STRIDES)

#define PyBUF_CONTIG (PyBUF_ND | PyBUF_WRITABLE)
#define PyBUF_CONTIG_RO (PyBUF_ND)

#define PyBUF_STRIDED (PyBUF_STRIDES | PyBUF_WRITABLE)
#define PyBUF_STRIDED_RO (PyBUF_STRIDES)

#define PyBUF_RECORDS (PyBUF_STRIDES | PyBUF_WRITABLE | PyBUF_FORMAT)
#define PyBUF_RECORDS_RO (PyBUF_STRIDES | PyBUF_FORMAT)

#define PyBUF_FULL (PyBUF_INDIRECT | PyBUF_WRITABLE | PyBUF_FORMAT)
#define PyBUF_FULL_RO (PyBUF_INDIRECT | PyBUF_FORMAT)


#define PyBUF_READ  0x100
#define PyBUF_WRITE 0x200

/* End buffer interface */
#endif /* Py_LIMITED_API */

#ifdef Py_LIMITED_API
LIBSHIBOKEN_API PyObject *PyMemoryView_FromBuffer(Pep_buffer *info);
#define Py_buffer Pep_buffer
#endif

/*****************************************************************************
 *
 * RESOLVED: object.h
 *
 */
#ifdef Py_LIMITED_API
// Why the hell is this useful debugging function not allowed?
LIBSHIBOKEN_API void _PyObject_Dump(PyObject *);
#endif

/*
 * There are a few structures that are needed, but cannot be used without
 * breaking the API. We use some heuristics to get those fields anyway
 * and validate that we really found them, see Pepresolve.cpp .
 */

#ifdef Py_LIMITED_API
# define _TYPE_CAST_(o)                     reinterpret_cast<_PepTypeObject*>(o)
#else
// The following type cast will finally removed when Sbk..Type is simplified.
# define _TYPE_CAST_(o)                     reinterpret_cast<PyTypeObject*>(o)
# define PepTypeObject                      PyHeapTypeObject
#endif

#define PepType_tp_methods(o)               (_TYPE_CAST_(o)->tp_methods)
#define PepType_tp_alloc(o)                 (_TYPE_CAST_(o)->tp_alloc)
#define PepType_tp_base(o)                  (_TYPE_CAST_(o)->tp_base)
#define PepType_tp_bases(o)                 (_TYPE_CAST_(o)->tp_bases)
#define PepType_tp_free(o)                  (_TYPE_CAST_(o)->tp_free)
#define PepType_tp_traverse(o)              (_TYPE_CAST_(o)->tp_traverse)
#define PepType_tp_clear(o)                 (_TYPE_CAST_(o)->tp_clear)
#define PepType_tp_descr_get(o)             (_TYPE_CAST_(o)->tp_descr_get)
#define PepType_tp_new(o)                   (_TYPE_CAST_(o)->tp_new)
#define PepType_tp_init(o)                  (_TYPE_CAST_(o)->tp_init)
#define PepType_tp_is_gc(o)                 (_TYPE_CAST_(o)->tp_is_gc)
#define PepType_tp_mro(o)                   (_TYPE_CAST_(o)->tp_mro)
#define PepType_tp_dict(o)                  (_TYPE_CAST_(o)->tp_dict)
#define PepType_tp_name(o)                  (_TYPE_CAST_(o)->tp_name)
#define PepType_tp_str(o)                   (_TYPE_CAST_(o)->tp_str)
#define PepType_tp_call(o)                  (_TYPE_CAST_(o)->tp_call)
#define PepType_tp_weaklistoffset(o)        (_TYPE_CAST_(o)->tp_weaklistoffset)
#define PepType_tp_dictoffset(o)            (_TYPE_CAST_(o)->tp_dictoffset)

#ifdef Py_LIMITED_API

// These are the type object fields that we use.
// We will verify that they never change.
typedef struct __Peptypeobject {
    PyVarObject ob_base;
    const char *tp_name;
    Py_ssize_t tp_basicsize;
    void *X03; // Py_ssize_t tp_itemsize;
    void *X04; // destructor tp_dealloc;
    void *X05; // printfunc tp_print;
    void *X06; // getattrfunc tp_getattr;
    void *X07; // setattrfunc tp_setattr;
    void *X08; // PyAsyncMethods *tp_as_async;
    void *X09; // reprfunc tp_repr;
    void *X10; // PyNumberMethods *tp_as_number;
    void *X11; // PySequenceMethods *tp_as_sequence;
    void *X12; // PyMappingMethods *tp_as_mapping;
    void *X13; // hashfunc tp_hash;
    ternaryfunc tp_call;
    reprfunc tp_str;
    void *X16; // getattrofunc tp_getattro;
    void *X17; // setattrofunc tp_setattro;
    void *X18; // PyBufferProcs *tp_as_buffer;
    void *X19; // unsigned long tp_flags;
    void *X20; // const char *tp_doc;
    traverseproc tp_traverse;
    inquiry tp_clear;
    void *X23; // richcmpfunc tp_richcompare;
    Py_ssize_t tp_weaklistoffset;
    void *X25; // getiterfunc tp_iter;
    void *X26; // iternextfunc tp_iternext;
    struct PyMethodDef *tp_methods;
    void *X28; // struct PyMemberDef *tp_members;
    void *X29; // struct PyGetSetDef *tp_getset;
    struct _typeobject *tp_base;
    PyObject *tp_dict;
    descrgetfunc tp_descr_get;
    void *X33; // descrsetfunc tp_descr_set;
    Py_ssize_t tp_dictoffset;
    initproc tp_init;
    allocfunc tp_alloc;
    newfunc tp_new;
    freefunc tp_free;
    inquiry tp_is_gc; /* For PyObject_IS_GC */
    PyObject *tp_bases;
    PyObject *tp_mro; /* method resolution order */

} _PepTypeObject;

LIBSHIBOKEN_API unaryfunc PepType_nb_index(PyTypeObject *type);

#undef PyIndex_Check

LIBSHIBOKEN_API int PyIndex_Check(PyObject *obj);

#undef PyObject_IS_GC
#define PyObject_IS_GC(o) (PyType_IS_GC(Py_TYPE(o)) && \
    ( PepType_tp_is_gc(Py_TYPE(o)) == NULL || \
      PepType_tp_is_gc(Py_TYPE(o))(o) ))

// This will hopefully go away, or we need to compute that size for every
// Python 3 version that we use.
#define PYTHON_3_6_HEAPTYPE_SIZE 109
#if PY_MINOR_VERSION == 6
# define PY_HEAPTYPE_SIZE PYTHON_3_6_HEAPTYPE_SIZE
#endif

typedef struct _peptypeobject {
    union {
        _PepTypeObject ht_type;
        void *opaque[PY_HEAPTYPE_SIZE];
    };
} PepTypeObject;

#else
#define PepType_nb_index(o)             (_TYPE_CAST_(o)->nb_index)
#endif // Py_LIMITED_API

// functions used everywhere
LIBSHIBOKEN_API PyObject *PepType_FromSpec(PyType_Spec *spec, int extra);
LIBSHIBOKEN_API PyObject *PepType_FromSpecWithBases(PyType_Spec *spec, PyObject *bases, int extra);
LIBSHIBOKEN_API const char *PepType_GetNameStr(PyTypeObject *type);
LIBSHIBOKEN_API PyObject *PepType___new__(PyTypeObject *self, PyObject *args, PyObject *kwds);

/*****************************************************************************
 *
 * RESOLVED: longobject.h
 *
 */
#ifdef Py_LIMITED_API
LIBSHIBOKEN_API int _PepLong_AsInt(PyObject *);
#else
#define _PepLong_AsInt _PyLong_AsInt
#endif

/*****************************************************************************
 *
 * RESOLVED: pydebug.h
 *
 */
#ifdef Py_LIMITED_API
/*
 * We have no direct access to Py_VerboseFlag because debugging is not
 * supported. The python developers are partially a bit too rigorous.
 * Instead, we compute the value and use a function call macro.
 * Was before: extern LIBSHIBOKEN_API int Py_VerboseFlag;
 */
LIBSHIBOKEN_API int Pep_GetFlag(const char *name);
LIBSHIBOKEN_API int Pep_GetVerboseFlag(void);
#define Py_VerboseFlag              Pep_GetVerboseFlag()
#endif

/*****************************************************************************
 *
 * RESOLVED: unicodeobject.h
 *
 */
#ifdef Py_LIMITED_API

LIBSHIBOKEN_API char *_PepUnicode_AsString(PyObject *);

#define PyUnicode_GET_SIZE(op)      PyUnicode_GetSize((PyObject *)(op))

#else
#define _PepUnicode_AsString     PyUnicode_AsUTF8
#endif

/*****************************************************************************
 *
 * RESOLVED: bytesobject.h
 *
 */
#ifdef Py_LIMITED_API
#define PyBytes_AS_STRING(op)       PyBytes_AsString(op)
#define PyBytes_GET_SIZE(op)        PyBytes_Size(op)
#endif

/*****************************************************************************
 *
 * RESOLVED: floatobject.h
 *
 */
#ifdef Py_LIMITED_API
#define PyFloat_AS_DOUBLE(op)       PyFloat_AsDouble(op)
#endif

/*****************************************************************************
 *
 * RESOLVED: tupleobject.h
 *
 */
#ifdef Py_LIMITED_API
#define PyTuple_GET_ITEM(op, i)     PyTuple_GetItem((PyObject *)op, i)
#define PyTuple_GET_SIZE(op)        PyTuple_Size((PyObject *)op)
#define PyTuple_SET_ITEM(op, i, v)  PyTuple_SetItem(op, i, v)
#endif

/*****************************************************************************
 *
 * RESOLVED: listobject.h
 *
 */
#ifdef Py_LIMITED_API
#define PyList_GET_ITEM(op, i)      PyList_GetItem(op, i)
#define PyList_SET_ITEM(op, i, v)   PyList_SetItem(op, i, v)
#define PyList_GET_SIZE(op)         PyList_Size(op)
#endif

/*****************************************************************************
 *
 * RESOLVED: methodobject.h
 *
 */

#ifdef Py_LIMITED_API

typedef struct _pycfunc PyCFunctionObject;
#define PyCFunction_GET_FUNCTION(func)  PyCFunction_GetFunction((PyObject *)func)
#define PyCFunction_GET_SELF(func)      PyCFunction_GetSelf((PyObject *)func)
#define PyCFunction_GET_FLAGS(func)     PyCFunction_GetFlags((PyObject *)func)
#define PepCFunction_GET_NAMESTR(func) \
    _PepUnicode_AsString(PyObject_GetAttrString((PyObject *)func, "__name__"))
#else
#define PepCFunction_GET_NAMESTR(func)        ((func)->m_ml->ml_name)
#endif

/*****************************************************************************
 *
 * RESOLVED: descrobject.h
 *
 */
#ifdef Py_LIMITED_API
typedef struct _methoddescr PyMethodDescrObject;
#endif

/*****************************************************************************
 *
 * RESOLVED: pystate.h
 *
 */

/*
 * pystate provides the data structure that is needed for the trashcan
 * algorithm. Unfortunately, it is not included in the limited API.
 * We have two options:
 *
 *  (1) ignore trashcan and live without secured deeply nested structures,
 *  (2) maintain the structure ourselves and make sure it does not change.
 *
 * I have chosen the second option.
 *
 * When a new python version appears, you need to check compatibility of
 * the PyThreadState structure (pystate.h) and the trashcan macros at the
 * end of object.h .
 */

#ifdef Py_LIMITED_API

#define Py_TRASH_MIN_COMPATIBLE 0x03020400
#define Py_TRASH_MAX_COMPATIBLE 0x030700A0

#if PY_VERSION_HEX >= Py_TRASH_MIN_COMPATIBLE && \
    PY_VERSION_HEX <= Py_TRASH_MAX_COMPATIBLE
typedef int (*Py_tracefunc)(PyObject *, struct _frame *, int, PyObject *);

// This structure has the trashcan variables since Python 3.2.4.
// We renamed all but the trashcan fields to make sure that we don't use
// anything else somewhere.

typedef struct _ts {
    struct _ts *Pep_prev;
    struct _ts *Pep_next;
    PyInterpreterState *Pep_interp;

    struct _frame *Pep_frame;
    int Pep_recursion_depth;
    char Pep_overflowed;
    char Pep_recursion_critical;

    int Pep_tracing;
    int Pep_use_tracing;

    Py_tracefunc Pep_c_profilefunc;
    Py_tracefunc Pep_c_tracefunc;
    PyObject *Pep_c_profileobj;
    PyObject *Pep_c_traceobj;

    PyObject *Pep_curexc_type;
    PyObject *Pep_curexc_value;
    PyObject *Pep_curexc_traceback;

    PyObject *Pep_exc_type;
    PyObject *Pep_exc_value;
    PyObject *Pep_exc_traceback;

    PyObject *Pep_dict;

    int Pep_gilstate_counter;

    PyObject *Pep_async_exc;
    long Pep_thread_id;
    // These two variables only are of interest to us.
    int trash_delete_nesting;
    PyObject *trash_delete_later;
    // Here we cut away the rest of the reduced structure.
} PyThreadState;
#else
#error *** Please check compatibility of the trashcan code, see Pep.h ***
#endif

#endif // Py_LIMITED_API

/*****************************************************************************
 *
 * RESOLVED: pythonrun.h
 *
 */
#ifdef Py_LIMITED_API
LIBSHIBOKEN_API PyObject *PyRun_String(const char *, int, PyObject *, PyObject *);
#endif

/*****************************************************************************
 *
 * RESOLVED: abstract.h
 *
 */
#ifdef Py_LIMITED_API

// This definition breaks the limited API a little, because it re-enables the
// buffer functions.
// But this is no problem as we check it's validity for every version.

#define PYTHON_BUFFER_VERSION_COMPATIBLE    (PY_VERSION_HEX >= 0x03030000 && \
                                             PY_VERSION_HEX <  0X0306FFFF)
#if !PYTHON_BUFFER_VERSION_COMPATIBLE
# error Please check the buffer compatibility for this python version!
#endif

typedef struct {
     getbufferproc bf_getbuffer;
     releasebufferproc bf_releasebuffer;
} PyBufferProcs;

typedef struct _Pepbuffertype {
    PyVarObject ob_base;
    void *skip[17];
    PyBufferProcs *tp_as_buffer;
} PepBufferType;

#define PepType_AS_BUFFER(type)   \
    reinterpret_cast<PepBufferType *>(type)->tp_as_buffer

#define PyObject_CheckBuffer(obj) \
    ((PepType_AS_BUFFER(Py_TYPE(obj)) != NULL) &&  \
     (PepType_AS_BUFFER(Py_TYPE(obj))->bf_getbuffer != NULL))

LIBSHIBOKEN_API int PyObject_GetBuffer(PyObject *ob, Pep_buffer *view, int flags);
LIBSHIBOKEN_API void PyBuffer_Release(Pep_buffer *view);
#endif /* Py_LIMITED_API */

/*****************************************************************************
 *
 * RESOLVED: funcobject.h
 *
 */
#ifdef Py_LIMITED_API
typedef struct _func PyFunctionObject;

extern LIBSHIBOKEN_API PyTypeObject *PepFunction_TypePtr;
LIBSHIBOKEN_API PyObject *PepFunction_Get(PyObject *, const char *);

#define PyFunction_Check(op)        (Py_TYPE(op) == PepFunction_TypePtr)
#define PyFunction_GET_CODE(func)   PyFunction_GetCode(func)

#define PyFunction_GetCode(func)        PepFunction_Get((PyObject *)func, "__code__")
#define PepFunction_GetName(func)    PepFunction_Get((PyObject *)func, "__name__")
#else
#define PepFunction_GetName(func)    (((PyFunctionObject *)func)->func_name)
#endif

/*****************************************************************************
 *
 * RESOLVED: classobject.h
 *
 */
#ifdef Py_LIMITED_API

typedef struct _meth PyMethodObject;

extern LIBSHIBOKEN_API PyTypeObject *PepMethod_TypePtr;

LIBSHIBOKEN_API PyObject *PyMethod_New(PyObject *, PyObject *);
LIBSHIBOKEN_API PyObject *PyMethod_Function(PyObject *);
LIBSHIBOKEN_API PyObject *PyMethod_Self(PyObject *);

#define PyMethod_Check(op) ((op)->ob_type == PepMethod_TypePtr)

#define PyMethod_GET_SELF(op)       PyMethod_Self(op)
#define PyMethod_GET_FUNCTION(op)   PyMethod_Function(op)
#endif

/*****************************************************************************
 *
 * RESOLVED: code.h
 *
 */
#ifdef Py_LIMITED_API
/* Bytecode object */
    // we have to grab the code object from python
typedef struct _code PyCodeObject;

LIBSHIBOKEN_API int PepCode_Get(PyCodeObject *co, const char *name);

#define PepCode_GET_FLAGS(o)         PepCode_Get(o, "co_flags")
#define PepCode_GET_ARGCOUNT(o)      PepCode_Get(o, "co_argcount")

/* Masks for co_flags above */
#define CO_OPTIMIZED    0x0001
#define CO_NEWLOCALS    0x0002
#define CO_VARARGS      0x0004
#define CO_VARKEYWORDS  0x0008
#define CO_NESTED       0x0010
#define CO_GENERATOR    0x0020
#else
#define PepCode_GET_FLAGS(o)         ((o)->co_flags)
#define PepCode_GET_ARGCOUNT(o)      ((o)->co_argcount)
#endif

/*****************************************************************************
 *
 * RESOLVED: datetime.h
 *
 */
#ifdef Py_LIMITED_API

LIBSHIBOKEN_API int PyDateTime_Get(PyObject *ob, const char *name);

#define PyDateTime_GetYear(o)         PyDateTime_Get(o, "year")
#define PyDateTime_GetMonth(o)        PyDateTime_Get(o, "month")
#define PyDateTime_GetDay(o)          PyDateTime_Get(o, "day")
#define PyDateTime_GetHour(o)         PyDateTime_Get(o, "hour")
#define PyDateTime_GetMinute(o)       PyDateTime_Get(o, "minute")
#define PyDateTime_GetSecond(o)       PyDateTime_Get(o, "second")
#define PyDateTime_GetMicrosecond(o)  PyDateTime_Get(o, "microsecond")
#define PyDateTime_GetFold(o)         PyDateTime_Get(o, "fold")

#define PyDateTime_GET_YEAR(o)              PyDateTime_GetYear(o)
#define PyDateTime_GET_MONTH(o)             PyDateTime_GetMonth(o)
#define PyDateTime_GET_DAY(o)               PyDateTime_GetDay(o)

#define PyDateTime_DATE_GET_HOUR(o)         PyDateTime_GetHour(o)
#define PyDateTime_DATE_GET_MINUTE(o)       PyDateTime_GetMinute(o)
#define PyDateTime_DATE_GET_SECOND(o)       PyDateTime_GetSecond(o)
#define PyDateTime_DATE_GET_MICROSECOND(o)  PyDateTime_GetMicrosecond(o)
#define PyDateTime_DATE_GET_FOLD(o)         PyDateTime_GetFold(o)

#define PyDateTime_TIME_GET_HOUR(o)         PyDateTime_GetHour(o)
#define PyDateTime_TIME_GET_MINUTE(o)       PyDateTime_GetMinute(o)
#define PyDateTime_TIME_GET_SECOND(o)       PyDateTime_GetSecond(o)
#define PyDateTime_TIME_GET_MICROSECOND(o)  PyDateTime_GetMicrosecond(o)
#define PyDateTime_TIME_GET_FOLD(o)         PyDateTime_GetFold(o)

/* Define structure slightly similar to C API. */
typedef struct {
    PyObject *module;
    /* type objects */
    PyTypeObject *DateType;
    PyTypeObject *DateTimeType;
    PyTypeObject *TimeType;
    PyTypeObject *DeltaType;
    PyTypeObject *TZInfoType;
} datetime_struc;

LIBSHIBOKEN_API datetime_struc *init_DateTime(void);

#define PyDateTime_IMPORT     PyDateTimeAPI = init_DateTime()

static datetime_struc *PyDateTimeAPI = NULL;

#define PyDate_Check(op)      PyObject_TypeCheck(op, PyDateTimeAPI->DateType)
#define PyDateTime_Check(op)  PyObject_TypeCheck(op, PyDateTimeAPI->DateTimeType)
#define PyTime_Check(op)      PyObject_TypeCheck(op, PyDateTimeAPI->TimeType)

LIBSHIBOKEN_API PyObject *PyDate_FromDate(int year, int month, int day);
LIBSHIBOKEN_API PyObject *PyDateTime_FromDateAndTime(
    int year, int month, int day, int hour, int min, int sec, int usec);
LIBSHIBOKEN_API PyObject *PyTime_FromTime(
    int hour, int minute, int second, int usecond);

#endif /* Py_LIMITED_API */

/*****************************************************************************
 *
 * Extra support for signature.cpp
 *
 */

#ifdef Py_LIMITED_API
extern LIBSHIBOKEN_API PyTypeObject *PepStaticMethod_TypePtr;
#else
#define PepStaticMethod_TypePtr &PyStaticMethod_Type
#endif

/*****************************************************************************
 *
 * Module Initialization
 *
 */

LIBSHIBOKEN_API void Pep_Init(void);

} // extern "C"

#endif // PEP384IMPL_H
