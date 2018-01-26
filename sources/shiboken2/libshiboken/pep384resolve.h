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

#ifndef PEP384RESOLVE_H
#define PEP384RESOLVE_H

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
// This has been renamed to Pep384_buffer and will be used.
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
} Pep384_buffer;

typedef int (*getbufferproc)(PyObject *, Pep384_buffer *, int);
typedef void (*releasebufferproc)(PyObject *, Pep384_buffer *);

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
PyAPI_FUNC(PyObject *) PyMemoryView_FromBuffer(Pep384_buffer *info);
#define Py_buffer Pep384_buffer
#endif

/*****************************************************************************
 *
 * From object.h
 *
 */
#ifdef Py_LIMITED_API
// Why the hell is this useful debugging function not allowed?
PyAPI_FUNC(void) _PyObject_Dump(PyObject *);
#endif

#ifdef Py_LIMITED_API
typedef struct _Py_Identifier {
    struct _Py_Identifier *next;
    const char* string;
    PyObject *object;
} _Py_Identifier;

#define _Py_static_string_init(value) { .next = NULL, .string = value, .object = NULL }
#define _Py_static_string(varname, value)  static _Py_Identifier varname = _Py_static_string_init(value)
#define _Py_IDENTIFIER(varname) _Py_static_string(PyId_##varname, #varname)

#endif /* !Py_LIMITED_API */

#ifdef Py_LIMITED_API
typedef struct {
    /* Number implementations must check *both*
       arguments for proper type and implement the necessary conversions
       in the slot functions themselves. */

    binaryfunc nb_add;
    binaryfunc nb_subtract;
    binaryfunc nb_multiply;
    binaryfunc nb_remainder;
    binaryfunc nb_divmod;
    ternaryfunc nb_power;
    unaryfunc nb_negative;
    unaryfunc nb_positive;
    unaryfunc nb_absolute;
    inquiry nb_bool;
    unaryfunc nb_invert;
    binaryfunc nb_lshift;
    binaryfunc nb_rshift;
    binaryfunc nb_and;
    binaryfunc nb_xor;
    binaryfunc nb_or;
    unaryfunc nb_int;
    void *nb_reserved;  /* the slot formerly known as nb_long */
    unaryfunc nb_float;

    binaryfunc nb_inplace_add;
    binaryfunc nb_inplace_subtract;
    binaryfunc nb_inplace_multiply;
    binaryfunc nb_inplace_remainder;
    ternaryfunc nb_inplace_power;
    binaryfunc nb_inplace_lshift;
    binaryfunc nb_inplace_rshift;
    binaryfunc nb_inplace_and;
    binaryfunc nb_inplace_xor;
    binaryfunc nb_inplace_or;

    binaryfunc nb_floor_divide;
    binaryfunc nb_true_divide;
    binaryfunc nb_inplace_floor_divide;
    binaryfunc nb_inplace_true_divide;

    unaryfunc nb_index;

    binaryfunc nb_matrix_multiply;
    binaryfunc nb_inplace_matrix_multiply;
} PyNumberMethods;

typedef struct {
    lenfunc sq_length;
    binaryfunc sq_concat;
    ssizeargfunc sq_repeat;
    ssizeargfunc sq_item;
    void *was_sq_slice;
    ssizeobjargproc sq_ass_item;
    void *was_sq_ass_slice;
    objobjproc sq_contains;

    binaryfunc sq_inplace_concat;
    ssizeargfunc sq_inplace_repeat;
} PySequenceMethods;

typedef struct {
    lenfunc mp_length;
    binaryfunc mp_subscript;
    objobjargproc mp_ass_subscript;
} PyMappingMethods;

typedef struct {
    unaryfunc am_await;
    unaryfunc am_aiter;
    unaryfunc am_anext;
} PyAsyncMethods;

typedef struct {
     getbufferproc bf_getbuffer;
     releasebufferproc bf_releasebuffer;
} PyBufferProcs;
#endif /* Py_LIMITED_API */

#ifdef Py_LIMITED_API
/* We can't provide a full compile-time check that limited-API
   users won't implement tp_print. However, not defining printfunc
   and making tp_print of a different function pointer type
   should at least cause a warning in most cases. */
typedef int (*printfunc)(PyObject *, FILE *, int);
#endif

#ifdef Py_LIMITED_API
typedef struct _typeobject {
    PyObject_VAR_HEAD
    const char *tp_name; /* For printing, in format "<module>.<name>" */
    Py_ssize_t tp_basicsize, tp_itemsize; /* For allocation */

    /* Methods to implement standard operations */

    destructor tp_dealloc;
    printfunc tp_print;
    getattrfunc tp_getattr;
    setattrfunc tp_setattr;
    PyAsyncMethods *tp_as_async; /* formerly known as tp_compare (Python 2)
                                    or tp_reserved (Python 3) */
    reprfunc tp_repr;

    /* Method suites for standard classes */

    PyNumberMethods *tp_as_number;
    PySequenceMethods *tp_as_sequence;
    PyMappingMethods *tp_as_mapping;

    /* More standard operations (here for binary compatibility) */

    hashfunc tp_hash;
    ternaryfunc tp_call;
    reprfunc tp_str;
    getattrofunc tp_getattro;
    setattrofunc tp_setattro;

    /* Functions to access object as input/output buffer */
    PyBufferProcs *tp_as_buffer;

    /* Flags to define presence of optional/expanded features */
    unsigned long tp_flags;

    const char *tp_doc; /* Documentation string */

    /* Assigned meaning in release 2.0 */
    /* call function for all accessible objects */
    traverseproc tp_traverse;

    /* delete references to contained objects */
    inquiry tp_clear;

    /* Assigned meaning in release 2.1 */
    /* rich comparisons */
    richcmpfunc tp_richcompare;

    /* weak reference enabler */
    Py_ssize_t tp_weaklistoffset;

    /* Iterators */
    getiterfunc tp_iter;
    iternextfunc tp_iternext;

    /* Attribute descriptor and subclassing stuff */
    struct PyMethodDef *tp_methods;
    struct PyMemberDef *tp_members;
    struct PyGetSetDef *tp_getset;
    struct _typeobject *tp_base;
    PyObject *tp_dict;
    descrgetfunc tp_descr_get;
    descrsetfunc tp_descr_set;
    Py_ssize_t tp_dictoffset;
    initproc tp_init;
    allocfunc tp_alloc;
    newfunc tp_new;
    freefunc tp_free; /* Low-level free-memory routine */
    inquiry tp_is_gc; /* For PyObject_IS_GC */
    PyObject *tp_bases;
    PyObject *tp_mro; /* method resolution order */
    PyObject *tp_cache;
    PyObject *tp_subclasses;
    PyObject *tp_weaklist;
    destructor tp_del;

    /* Type attribute cache version tag. Added in version 2.6 */
    unsigned int tp_version_tag;

    destructor tp_finalize;

#ifdef COUNT_ALLOCS
    /* these must be last and never explicitly initialized */
    Py_ssize_t tp_allocs;
    Py_ssize_t tp_frees;
    Py_ssize_t tp_maxalloc;
    struct _typeobject *tp_prev;
    struct _typeobject *tp_next;
#endif
} PyTypeObject;
#endif

#ifdef Py_LIMITED_API
/* The *real* layout of a type object when allocated on the heap */
typedef struct _heaptypeobject {
    /* Note: there's a dependency on the order of these members
       in slotptr() in typeobject.c . */
    PyTypeObject ht_type;
    PyAsyncMethods as_async;
    PyNumberMethods as_number;
    PyMappingMethods as_mapping;
    PySequenceMethods as_sequence; /* as_sequence comes after as_mapping,
                                      so that the mapping wins when both
                                      the mapping and the sequence define
                                      a given operator (e.g. __getitem__).
                                      see add_operators() in typeobject.c . */
    PyBufferProcs as_buffer;
    PyObject *ht_name, *ht_slots, *ht_qualname;
    struct _dictkeysobject *ht_cached_keys;
    /* here are optional user slots, followed by the members. */
} PyHeapTypeObject;
#endif

/*****************************************************************************
 *
 * From longobject.h
 *
 */
#ifdef Py_LIMITED_API
LIBSHIBOKEN_API int _PyLong_AsInt(PyObject *);
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
 * Was before: PyAPI_DATA(int) Py_VerboseFlag;
 */
PyAPI_FUNC(int) Py_GetFlag(const char *name);
PyAPI_FUNC(int) Py_GetVerboseFlag(void);
#define Py_VerboseFlag Py_GetVerboseFlag()
#endif

/*****************************************************************************
 *
 * From unicodeobject.h
 *
 */
#ifdef Py_LIMITED_API

#define PyUnicode_GET_SIZE(op)           PyUnicode_GetSize((PyObject *)(op))

PyAPI_FUNC(char *) PyUnicode_AsUTF8(PyObject *unicode);

#define _PyUnicode_AsString PyUnicode_AsUTF8
/* Return a read-only pointer to the Unicode object's internal
   Py_UNICODE buffer.
   If the wchar_t/Py_UNICODE representation is not yet available, this
   function will calculate it. */

typedef wchar_t Py_UNICODE;
PyAPI_FUNC(Py_UNICODE *) PyUnicode_AsUnicode(
    PyObject *unicode           /* Unicode object */
    );
#define PyUnicode_AS_UNICODE(op) PyUnicode_AsUnicode((PyObject *)(op))

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
#define Pep384CFunction_GET_NAMESTR(func) \
    ({ \
      PyObject *_V = PyObject_GetAttrString((PyObject *)func, "__name__"); \
      PyObject * _T = PyUnicode_AsEncodedString(_V, "UTF-8", "strict"); \
      char *_c = strdup(PyBytes_AsString(_T)); \
      Py_DECREF(_T); \
      Py_DECREF(_V); \
      _c; \
    })
#else
#define Pep384CFunction_GET_NAMESTR(func)        ((func)->m_ml->ml_name)
#endif

/*****************************************************************************
 *
 * From descrobject.h
 *
 */
#ifdef Py_LIMITED_API
typedef struct {
    PyObject_HEAD
    PyTypeObject *d_type;
    PyObject *d_name;
    PyObject *d_qualname;
} PyDescrObject;

#define PyDescr_COMMON PyDescrObject d_common

#define PyDescr_TYPE(x) (((PyDescrObject *)(x))->d_type)
#define PyDescr_NAME(x) (((PyDescrObject *)(x))->d_name)

typedef struct {
    PyDescr_COMMON;
    PyMethodDef *d_method;
} PyMethodDescrObject;

// typedef struct {
//     PyDescr_COMMON;
//     struct PyMemberDef *d_member;
// } PyMemberDescrObject;

// typedef struct {
//     PyDescr_COMMON;
//     PyGetSetDef *d_getset;
// } PyGetSetDescrObject;
#endif

/*****************************************************************************
 *
 * From pystate.h
 *
 */
#ifdef Py_LIMITED_API
/* Py_tracefunc return -1 when raising an exception, or 0 for success. */
typedef int (*Py_tracefunc)(PyObject *, struct _frame *, int, PyObject *);
#endif

#ifdef Py_LIMITED_API
typedef struct _ts {
    /* See Python/ceval.c for comments explaining most fields */

    struct _ts *prev;
    struct _ts *next;
    PyInterpreterState *interp;

    struct _frame *frame;
    int recursion_depth;
    char overflowed; /* The stack has overflowed. Allow 50 more calls
                        to handle the runtime error. */
    char recursion_critical; /* The current calls must not cause
                                a stack overflow. */
    /* 'tracing' keeps track of the execution depth when tracing/profiling.
       This is to prevent the actual trace/profile code from being recorded in
       the trace/profile. */
    int tracing;
    int use_tracing;

    Py_tracefunc c_profilefunc;
    Py_tracefunc c_tracefunc;
    PyObject *c_profileobj;
    PyObject *c_traceobj;

    PyObject *curexc_type;
    PyObject *curexc_value;
    PyObject *curexc_traceback;

    PyObject *exc_type;
    PyObject *exc_value;
    PyObject *exc_traceback;

    PyObject *dict;  /* Stores per-thread state */

    int gilstate_counter;

    PyObject *async_exc; /* Asynchronous exception to raise */
    long thread_id; /* Thread id where this tstate was created */

    int trash_delete_nesting;
    PyObject *trash_delete_later;

    /* Called when a thread state is deleted normally, but not when it
     * is destroyed after fork().
     * Pain:  to prevent rare but fatal shutdown errors (issue 18808),
     * Thread.join() must wait for the join'ed thread's tstate to be unlinked
     * from the tstate chain.  That happens at the end of a thread's life,
     * in pystate.c.
     * The obvious way doesn't quite work:  create a lock which the tstate
     * unlinking code releases, and have Thread.join() wait to acquire that
     * lock.  The problem is that we _are_ at the end of the thread's life:
     * if the thread holds the last reference to the lock, decref'ing the
     * lock will delete the lock, and that may trigger arbitrary Python code
     * if there's a weakref, with a callback, to the lock.  But by this time
     * _PyThreadState_Current is already NULL, so only the simplest of C code
     * can be allowed to run (in particular it must not be possible to
     * release the GIL).
     * So instead of holding the lock directly, the tstate holds a weakref to
     * the lock:  that's the value of on_delete_data below.  Decref'ing a
     * weakref is harmless.
     * on_delete points to _threadmodule.c's static release_sentinel() function.
     * After the tstate is unlinked, release_sentinel is called with the
     * weakref-to-lock (on_delete_data) argument, and release_sentinel releases
     * the indirectly held lock.
     */
    void (*on_delete)(void *);
    void *on_delete_data;

    PyObject *coroutine_wrapper;
    int in_coroutine_wrapper;

    /* Now used from PyInterpreterState, kept here for ABI
       compatibility with PyThreadState */
    Py_ssize_t _preserve_36_ABI_1;
    freefunc _preserve_36_ABI_2[MAX_CO_EXTRA_USERS];

    PyObject *async_gen_firstiter;
    PyObject *async_gen_finalizer;

    /* XXX signal handlers should also be here */

} PyThreadState;
#endif

/*****************************************************************************
 *
 * RESOLVED: pythonrun.h
 *
 */
#ifdef Py_LIMITED_API
PyAPI_FUNC(PyObject *) PyRun_String(const char *, int, PyObject *, PyObject *);
#endif

/*****************************************************************************
 *
 * RESOLVED: abstract.h
 *
 */
#ifdef Py_LIMITED_API
#define PyObject_CheckBuffer(obj) \
    (((obj)->ob_type->tp_as_buffer != NULL) &&  \
     ((obj)->ob_type->tp_as_buffer->bf_getbuffer != NULL))

    PyAPI_FUNC(int) PyObject_GetBuffer(PyObject *obj, Pep384_buffer *view,
                                      int flags);
    PyAPI_FUNC(void) PyBuffer_Release(Pep384_buffer *view);
#endif /* Py_LIMITED_API */

/*****************************************************************************
 *
 * From funcobject.h
 *
 */
#ifdef Py_LIMITED_API
typedef struct {
    PyObject_HEAD
    PyObject *func_code;    /* A code object, the __code__ attribute */
    PyObject *func_globals; /* A dictionary (other mappings won't do) */
    PyObject *func_defaults;    /* NULL or a tuple */
    PyObject *func_kwdefaults;  /* NULL or a dict */
    PyObject *func_closure; /* NULL or a tuple of cell objects */
    PyObject *func_doc;     /* The __doc__ attribute, can be anything */
    PyObject *func_name;    /* The __name__ attribute, a string object */
    PyObject *func_dict;    /* The __dict__ attribute, a dict or NULL */
    PyObject *func_weakreflist; /* List of weak references */
    PyObject *func_module;  /* The __module__ attribute, can be anything */
    PyObject *func_annotations; /* Annotations, a dict or NULL */
    PyObject *func_qualname;    /* The qualified name */

    /* Invariant:
     *     func_closure contains the bindings for func_code->co_freevars, so
     *     PyTuple_Size(func_closure) == PyCode_GetNumFree(func_code)
     *     (func_closure may be NULL if PyCode_GetNumFree(func_code) == 0).
     */
} PyFunctionObject;

PyAPI_DATA(PyTypeObject) PyFunction_Type;

#define PyFunction_Check(op) (Py_TYPE(op) == &PyFunction_Type)
PyAPI_DATA(PyTypeObject) PyStaticMethod_Type;

/* Macros for direct access to these values. Type checks are *not*
   done, so use with care. */
#define PyFunction_GET_CODE(func) \
        (((PyFunctionObject *)func) -> func_code)

#endif

/*****************************************************************************
 *
 * RESOLVED: classobject.h
 *
 */
#ifdef Py_LIMITED_API

typedef struct _meth PyMethodObject;

PyAPI_DATA(PyTypeObject) PyMethod_Type;

PyAPI_FUNC(PyObject *) PyMethod_New(PyObject *, PyObject *);
PyAPI_FUNC(PyObject *) PyMethod_Function(PyObject *);
PyAPI_FUNC(PyObject *) PyMethod_Self(PyObject *);

#define PyMethod_Check(op) ((op)->ob_type == &PyMethod_Type)

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

LIBSHIBOKEN_API int Pep384Code_Get(PyCodeObject *co, const char *name);

#define Pep384Code_GET_FLAGS(o)         Pep384Code_Get(o, "co_flags")
#define Pep384Code_GET_ARGCOUNT(o)      Pep384Code_Get(o, "co_argcount")

/* Masks for co_flags above */
#define CO_OPTIMIZED    0x0001
#define CO_NEWLOCALS    0x0002
#define CO_VARARGS      0x0004
#define CO_VARKEYWORDS  0x0008
#define CO_NESTED       0x0010
#define CO_GENERATOR    0x0020
#else
#define Pep384Code_GET_FLAGS(o)         ((o)->co_flags)
#define Pep384Code_GET_ARGCOUNT(o)      ((o)->co_argcount)
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
 * Module Initialization
 *
 */

LIBSHIBOKEN_API void PEP384_Init(void);

} // extern "C"

#endif // PEP384RESOLVE_H
