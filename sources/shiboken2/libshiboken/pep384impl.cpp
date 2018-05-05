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

#include "sbkpython.h"

extern "C"
{

#ifdef Py_LIMITED_API

/*****************************************************************************
 *
 * Support for object.h
 *
 */

// Here will be the verification code.

// static PyType_Slot PyType_Type_slots[] = {
//     {0, 0},     // Here we insert the wanted slot for probing.
//     {0, 0}
// };
// static PyType_Spec PyType_Type_spec = {
//     "python.type",
//     sizeof(PepTypeObject),                   /* tp_basicsize */
//     sizeof(PyMemberDef),                        /* tp_itemsize */
//     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
//     Py_TPFLAGS_BASETYPE | Py_TPFLAGS_TYPE_SUBCLASS,
//     PyType_Type_slots,
// };


// #ifdef IS_PY3K
// #  define SLOT slot
// #else
// #  define SLOT slot_
// #endif

// static void *handle_static_type(PyTypeObject *type, void *what, int length)
// {
//         for (int idx = 0; idx < length; ++idx) {
//             void **thing = (void **)type + idx;
//             if (*thing == what)
//                 break;
//         }
//         Py_DECREF(probe_type);
//         known_offsets[pyslot] = idx;
//     }
//     void **thing = (void **)type + idx;
//     return *thing;
// }


// This structure is only here because Python 3 has an error.
// I will fix that.

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

// temporary structure until we have a generator for the offsets
typedef struct _oldtypeobject {
    PyVarObject ob_base;
    void *X01; // const char *tp_name;
    void *X02; // Py_ssize_t tp_basicsize;
    void *X03; // Py_ssize_t tp_itemsize;
    void *X04; // destructor tp_dealloc;
    void *X05; // printfunc tp_print;
    void *X06; // getattrfunc tp_getattr;
    void *X07; // setattrfunc tp_setattr;
    void *X08; // PyAsyncMethods *tp_as_async;
    void *X09; // reprfunc tp_repr;
    PyNumberMethods *tp_as_number;

} PyOldTypeObject;

// There is a bug in Python 3.6 that turned the Index_Check function
// into a macro without taking care of the limited API.
// This leads to the single problem that we don't have
// access to PyLong_Type's nb_index field which is no heap type.
// We cannot easily create this function by inheritance since it is
// not inherited.
//
// Simple solution: Create the structure and write such a function.
// Long term: Submit a patch to python.org .

unaryfunc
PepType_nb_index(PyTypeObject *type)
{
    return reinterpret_cast<PyOldTypeObject*>(type)->tp_as_number->nb_index;
}

int PyIndex_Check(PyObject *obj)
{
    PyOldTypeObject *type = reinterpret_cast<PyOldTypeObject*>(Py_TYPE(obj));
    return type->tp_as_number != NULL &&
           type->tp_as_number->nb_index != NULL;
}

/*****************************************************************************
 *
 * Support for unicodeobject.h
 *
 */

char *
_PepUnicode_AsString(PyObject *str)
{
    /*
     * We need to keep the string alive but cannot borrow the Python object.
     * Ugly easy way out: We re-code as an interned bytes string. This
     * produces a pseudo-leak as long there are new strings.
     * Typically, this function is used for name strings, and the dict size
     * will not grow so much.
     */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

    static PyObject *cstring_dict = NULL;
    if (cstring_dict == NULL) {
        cstring_dict = PyDict_New();
        if (cstring_dict == NULL)
            Py_FatalError("Error in " AT);
    }
    PyObject *bytesStr = PyUnicode_AsEncodedString(str, "utf8", NULL);
    PyObject *entry = PyDict_GetItem(cstring_dict, bytesStr);
    if (entry == NULL) {
        int e = PyDict_SetItem(cstring_dict, bytesStr, bytesStr);
        if (e != 0)
            Py_FatalError("Error in " AT);
        entry = bytesStr;
    }
    else
        Py_DECREF(bytesStr);
    return PyBytes_AsString(entry);
}

/*****************************************************************************
 *
 * Support for longobject.h
 *
 */

/*
 * This is the original Python function _PyLong_AsInt() from longobject.c .
 * We define it here because we are not allowed to use the function
 * from Python with an underscore.
 */

/* Get a C int from an int object or any object that has an __int__
   method.  Return -1 and set an error if overflow occurs. */

int
_PepLong_AsInt(PyObject *obj)
{
    int overflow;
    long result = PyLong_AsLongAndOverflow(obj, &overflow);
    if (overflow || result > INT_MAX || result < INT_MIN) {
        /* XXX: could be cute and give a different
           message for overflow == -1 */
        PyErr_SetString(PyExc_OverflowError,
                        "Python int too large to convert to C int");
        return -1;
    }
    return (int)result;
}

/*****************************************************************************
 *
 * Support for pydebug.h
 *
 */
static PyObject *sys_flags = NULL;

int
Pep_GetFlag(const char *name)
{
    static int initialized = 0;
    int ret = -1;

    if (!initialized) {
        sys_flags = PySys_GetObject("flags");
        // func gives no error if NULL is returned and does not incref.
        Py_XINCREF(sys_flags);
        initialized = 1;
    }
    if (sys_flags != NULL) {
        PyObject *ob_ret = PyObject_GetAttrString(sys_flags, name);
        if (ob_ret != NULL) {
            long long_ret = PyLong_AsLong(ob_ret);
            Py_DECREF(ob_ret);
            ret = (int) long_ret;
        }
    }
    return ret;
}

int
Pep_GetVerboseFlag()
{
    static int initialized = 0;
    static int verbose_flag = -1;

    if (!initialized) {
        verbose_flag = Pep_GetFlag("verbose");
        if (verbose_flag != -1)
            initialized = 1;
    }
    return verbose_flag;
}

/*****************************************************************************
 *
 * Support for code.h
 *
 */

int
PepCode_Get(PyCodeObject *co, const char *name)
{
    PyObject *ob = (PyObject *)co;
    PyObject *ob_ret;
    int ret = -1;

    ob_ret = PyObject_GetAttrString(ob, name);
    if (ob_ret != NULL) {
        long long_ret = PyLong_AsLong(ob_ret);
        Py_DECREF(ob_ret);
        ret = (int) long_ret;
    }
    return ret;
}

/*****************************************************************************
 *
 * Support for datetime.h
 *
 */

static PyTypeObject *dt_getCheck(const char *name)
{
    PyObject *op = PyObject_GetAttrString(PyDateTimeAPI->module, name);
    if (op == NULL) {
        fprintf(stderr, "datetime.%s not found\n", name);
        Py_FatalError("aborting");
    }
    return (PyTypeObject *)op;
}

// init_DateTime is called earlier than our module init.
// We use the provided PyDateTime_IMPORT machinery.
datetime_struc *
init_DateTime(void)
{
    static int initialized = 0;
    if (!initialized) {
        PyDateTimeAPI = (datetime_struc *)malloc(sizeof(datetime_struc));
        if (PyDateTimeAPI == NULL)
            Py_FatalError("PyDateTimeAPI malloc error, aborting");
        PyDateTimeAPI->module = PyImport_ImportModule("datetime");
        if (PyDateTimeAPI->module == NULL)
            Py_FatalError("datetime module not found, aborting");
        PyDateTimeAPI->DateType     = dt_getCheck("date");
        PyDateTimeAPI->DateTimeType = dt_getCheck("datetime");
        PyDateTimeAPI->TimeType     = dt_getCheck("time");
        PyDateTimeAPI->DeltaType    = dt_getCheck("timedelta");
        PyDateTimeAPI->TZInfoType   = dt_getCheck("tzinfo");
        initialized = 1;
    }
    return PyDateTimeAPI;
}

int
PyDateTime_Get(PyObject *ob, const char *name)
{
    PyObject *ob_ret;
    int ret = -1;

    ob_ret = PyObject_GetAttrString(ob, name);
    if (ob_ret != NULL) {
        long long_ret = PyLong_AsLong(ob_ret);
        Py_DECREF(ob_ret);
        ret = (int) long_ret;
    }
    return ret;
}

PyObject *
PyDate_FromDate(int year, int month, int day)
{
    return PyObject_CallFunction((PyObject *)PyDateTimeAPI->DateType,
                                 (char *)"(iii)", year, month, day);
}

PyObject *
PyDateTime_FromDateAndTime(int year, int month, int day,
                           int hour, int min, int sec, int usec)
{
    return PyObject_CallFunction((PyObject *)PyDateTimeAPI->DateTimeType,
                                 (char *)"(iiiiiii)", year, month, day,
                                  hour, min, sec, usec);
}

PyObject *
PyTime_FromTime(int hour, int min, int sec, int usec)
{
    return PyObject_CallFunction((PyObject *)PyDateTimeAPI->TimeType,
                                 (char *)"(iiii)", hour, min, sec, usec);
}

/*****************************************************************************
 *
 * Support for pythonrun.h
 *
 */

// Flags are ignored in these simple helpers.
PyObject *
PyRun_String(const char *str, int start, PyObject *globals, PyObject *locals)
{
    PyObject* code = Py_CompileString(str, "pyscript", start);
    PyObject* ret = NULL;

    if (code != NULL) {
        ret = PyEval_EvalCode(code, globals, locals);
    }
    Py_XDECREF(code);
    return ret;
}

// This is only a simple local helper that returns a computed variable.
static PyObject *
PepRun_GetResult(const char *command, const char *resvar)
{
    PyObject *d, *v, *res;

    d = PyDict_New();
    if (d == NULL || PyDict_SetItemString(d, "__builtins__",
                                          PyEval_GetBuiltins()) < 0)
        return NULL;
    v = PyRun_String(command, Py_file_input, d, d);
    res = v ? PyDict_GetItemString(d, resvar) : NULL;
    Py_XDECREF(v);
    Py_DECREF(d);
    return res;
}

/*****************************************************************************
 *
 * Support for classobject.h
 *
 */

PyTypeObject *PepMethod_TypePtr = NULL;

static PyTypeObject *getMethodType(void)
{
    static const char prog[] =
        "class _C:\n"
        "    def _m(self): pass\n"
        "MethodType = type(_C()._m)\n";
    return (PyTypeObject *) PepRun_GetResult(prog, "MethodType");
}

// We have no access to PyMethod_New and must call types.MethodType, instead.
PyObject *
PyMethod_New(PyObject *func, PyObject *self)
{
    return PyObject_CallFunction((PyObject *)PepMethod_TypePtr,
                                 (char *)"(OO)", func, self);
}

PyObject *
PyMethod_Function(PyObject *im)
{
    PyObject *ret = PyObject_GetAttrString(im, "__func__");

    // We have to return a borrowed reference.
    Py_DECREF(ret);
    return ret;
}

PyObject *
PyMethod_Self(PyObject *im)
{
    PyObject *ret = PyObject_GetAttrString(im, "__self__");

    // We have to return a borrowed reference.
    // If we don't obey that here, then we get a test error!
    Py_DECREF(ret);
    return ret;
}

/*****************************************************************************
 *
 * Support for funcobject.h
 *
 */

PyObject *
PepFunction_Get(PyObject *ob, const char *name)
{
    PyObject *ret;

    // We have to return a borrowed reference.
    ret = PyObject_GetAttrString(ob, name);
    Py_XDECREF(ret);
    return ret;
}

/*****************************************************************************
 *
 * Copied from abstract.c
 *
 * Py_buffer has been replaced by Pep_buffer
 *
 */

/* Buffer C-API for Python 3.0 */

int
PyObject_GetBuffer(PyObject *obj, Pep_buffer *view, int flags)
{
    PyBufferProcs *pb = PepType_AS_BUFFER(Py_TYPE(obj));

    if (pb == NULL || pb->bf_getbuffer == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "a bytes-like object is required, not '%.100s'",
                     PepType_tp_name(Py_TYPE(obj)));
        return -1;
    }
    return (*pb->bf_getbuffer)(obj, view, flags);
}

static int
_IsFortranContiguous(const Pep_buffer *view)
{
    Py_ssize_t sd, dim;
    int i;

    /* 1) len = product(shape) * itemsize
       2) itemsize > 0
       3) len = 0 <==> exists i: shape[i] = 0 */
    if (view->len == 0) return 1;
    if (view->strides == NULL) {  /* C-contiguous by definition */
        /* Trivially F-contiguous */
        if (view->ndim <= 1) return 1;

        /* ndim > 1 implies shape != NULL */
        assert(view->shape != NULL);

        /* Effectively 1-d */
        sd = 0;
        for (i=0; i<view->ndim; i++) {
            if (view->shape[i] > 1) sd += 1;
        }
        return sd <= 1;
    }

    /* strides != NULL implies both of these */
    assert(view->ndim > 0);
    assert(view->shape != NULL);

    sd = view->itemsize;
    for (i=0; i<view->ndim; i++) {
        dim = view->shape[i];
        if (dim > 1 && view->strides[i] != sd) {
            return 0;
        }
        sd *= dim;
    }
    return 1;
}

static int
_IsCContiguous(const Pep_buffer *view)
{
    Py_ssize_t sd, dim;
    int i;

    /* 1) len = product(shape) * itemsize
       2) itemsize > 0
       3) len = 0 <==> exists i: shape[i] = 0 */
    if (view->len == 0) return 1;
    if (view->strides == NULL) return 1; /* C-contiguous by definition */

    /* strides != NULL implies both of these */
    assert(view->ndim > 0);
    assert(view->shape != NULL);

    sd = view->itemsize;
    for (i=view->ndim-1; i>=0; i--) {
        dim = view->shape[i];
        if (dim > 1 && view->strides[i] != sd) {
            return 0;
        }
        sd *= dim;
    }
    return 1;
}

int
PyBuffer_IsContiguous(const Pep_buffer *view, char order)
{

    if (view->suboffsets != NULL) return 0;

    if (order == 'C')
        return _IsCContiguous(view);
    else if (order == 'F')
        return _IsFortranContiguous(view);
    else if (order == 'A')
        return (_IsCContiguous(view) || _IsFortranContiguous(view));
    return 0;
}


void*
PyBuffer_GetPointer(Pep_buffer *view, Py_ssize_t *indices)
{
    char* pointer;
    int i;
    pointer = (char *)view->buf;
    for (i = 0; i < view->ndim; i++) {
        pointer += view->strides[i]*indices[i];
        if ((view->suboffsets != NULL) && (view->suboffsets[i] >= 0)) {
            pointer = *((char**)pointer) + view->suboffsets[i];
        }
    }
    return (void*)pointer;
}


void
_Py_add_one_to_index_F(int nd, Py_ssize_t *index, const Py_ssize_t *shape)
{
    int k;

    for (k=0; k<nd; k++) {
        if (index[k] < shape[k]-1) {
            index[k]++;
            break;
        }
        else {
            index[k] = 0;
        }
    }
}

void
_Py_add_one_to_index_C(int nd, Py_ssize_t *index, const Py_ssize_t *shape)
{
    int k;

    for (k=nd-1; k>=0; k--) {
        if (index[k] < shape[k]-1) {
            index[k]++;
            break;
        }
        else {
            index[k] = 0;
        }
    }
}

int
PyBuffer_FromContiguous(Pep_buffer *view, void *buf, Py_ssize_t len, char fort)
{
    int k;
    void (*addone)(int, Py_ssize_t *, const Py_ssize_t *);
    Py_ssize_t *indices, elements;
    char *src, *ptr;

    if (len > view->len) {
        len = view->len;
    }

    if (PyBuffer_IsContiguous(view, fort)) {
        /* simplest copy is all that is needed */
        memcpy(view->buf, buf, len);
        return 0;
    }

    /* Otherwise a more elaborate scheme is needed */

    /* view->ndim <= 64 */
    indices = (Py_ssize_t *)PyMem_Malloc(sizeof(Py_ssize_t)*(view->ndim));
    if (indices == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    for (k=0; k<view->ndim;k++) {
        indices[k] = 0;
    }

    if (fort == 'F') {
        addone = _Py_add_one_to_index_F;
    }
    else {
        addone = _Py_add_one_to_index_C;
    }
    src = (char *)buf; // patched by CT
    /* XXX : This is not going to be the fastest code in the world
             several optimizations are possible.
     */
    elements = len / view->itemsize;
    while (elements--) {
        ptr = (char *)PyBuffer_GetPointer(view, indices); // patched by CT
        memcpy(ptr, src, view->itemsize);
        src += view->itemsize;
        addone(view->ndim, indices, view->shape);
    }

    PyMem_Free(indices);
    return 0;
}

int PyObject_CopyData(PyObject *dest, PyObject *src)
{
    Pep_buffer view_dest, view_src;
    int k;
    Py_ssize_t *indices, elements;
    char *dptr, *sptr;

    if (!PyObject_CheckBuffer(dest) ||
        !PyObject_CheckBuffer(src)) {
        PyErr_SetString(PyExc_TypeError,
                        "both destination and source must be "\
                        "bytes-like objects");
        return -1;
    }

    if (PyObject_GetBuffer(dest, &view_dest, PyBUF_FULL) != 0) return -1;
    if (PyObject_GetBuffer(src, &view_src, PyBUF_FULL_RO) != 0) {
        PyBuffer_Release(&view_dest);
        return -1;
    }

    if (view_dest.len < view_src.len) {
        PyErr_SetString(PyExc_BufferError,
                        "destination is too small to receive data from source");
        PyBuffer_Release(&view_dest);
        PyBuffer_Release(&view_src);
        return -1;
    }

    if ((PyBuffer_IsContiguous(&view_dest, 'C') &&
         PyBuffer_IsContiguous(&view_src, 'C')) ||
        (PyBuffer_IsContiguous(&view_dest, 'F') &&
         PyBuffer_IsContiguous(&view_src, 'F'))) {
        /* simplest copy is all that is needed */
        memcpy(view_dest.buf, view_src.buf, view_src.len);
        PyBuffer_Release(&view_dest);
        PyBuffer_Release(&view_src);
        return 0;
    }

    /* Otherwise a more elaborate copy scheme is needed */

    /* XXX(nnorwitz): need to check for overflow! */
    indices = (Py_ssize_t *)PyMem_Malloc(sizeof(Py_ssize_t)*view_src.ndim);
    if (indices == NULL) {
        PyErr_NoMemory();
        PyBuffer_Release(&view_dest);
        PyBuffer_Release(&view_src);
        return -1;
    }
    for (k=0; k<view_src.ndim;k++) {
        indices[k] = 0;
    }
    elements = 1;
    for (k=0; k<view_src.ndim; k++) {
        /* XXX(nnorwitz): can this overflow? */
        elements *= view_src.shape[k];
    }
    while (elements--) {
        _Py_add_one_to_index_C(view_src.ndim, indices, view_src.shape);
        dptr = (char *)PyBuffer_GetPointer(&view_dest, indices); // patched by CT
        sptr = (char *)PyBuffer_GetPointer(&view_src, indices); // patched by CT
        memcpy(dptr, sptr, view_src.itemsize);
    }
    PyMem_Free(indices);
    PyBuffer_Release(&view_dest);
    PyBuffer_Release(&view_src);
    return 0;
}

void
PyBuffer_FillContiguousStrides(int nd, Py_ssize_t *shape,
                               Py_ssize_t *strides, int itemsize,
                               char fort)
{
    int k;
    Py_ssize_t sd;

    sd = itemsize;
    if (fort == 'F') {
        for (k=0; k<nd; k++) {
            strides[k] = sd;
            sd *= shape[k];
        }
    }
    else {
        for (k=nd-1; k>=0; k--) {
            strides[k] = sd;
            sd *= shape[k];
        }
    }
    return;
}

int
PyBuffer_FillInfo(Pep_buffer *view, PyObject *obj, void *buf, Py_ssize_t len,
                  int readonly, int flags)
{
    if (view == NULL) {
        PyErr_SetString(PyExc_BufferError,
            "PyBuffer_FillInfo: view==NULL argument is obsolete");
        return -1;
    }

    if (((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) &&
        (readonly == 1)) {
        PyErr_SetString(PyExc_BufferError,
                        "Object is not writable.");
        return -1;
    }

    view->obj = obj;
    if (obj)
        Py_INCREF(obj);
    view->buf = buf;
    view->len = len;
    view->readonly = readonly;
    view->itemsize = 1;
    view->format = NULL;
    if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT)
        view->format = (char *)"B"; // patched by CT
    view->ndim = 1;
    view->shape = NULL;
    if ((flags & PyBUF_ND) == PyBUF_ND)
        view->shape = &(view->len);
    view->strides = NULL;
    if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES)
        view->strides = &(view->itemsize);
    view->suboffsets = NULL;
    view->internal = NULL;
    return 0;
}

void
PyBuffer_Release(Pep_buffer *view)
{
    PyObject *obj = view->obj;
    PyBufferProcs *pb;
    if (obj == NULL)
        return;
    pb = PepType_AS_BUFFER(Py_TYPE(obj));
    if (pb && pb->bf_releasebuffer)
        pb->bf_releasebuffer(obj, view);
    view->obj = NULL;
    Py_DECREF(obj);
}

/*****************************************************************************
 *
 * Support for funcobject.h
 *
 */

// this became necessary after Windows was activated.

PyTypeObject *PepFunction_TypePtr = NULL;

static PyTypeObject *getFunctionType(void)
{
    static const char prog[] =
        "from types import FunctionType\n";
    return (PyTypeObject *) PepRun_GetResult(prog, "FunctionType");
}

/*****************************************************************************
 *
 * Extra support for signature.cpp
 *
 */

PyTypeObject *PepStaticMethod_TypePtr = NULL;

static PyTypeObject *getStaticMethodType(void)
{
    static const char prog[] =
        "StaticMethodType = type(str.__dict__['maketrans'])\n";
    return (PyTypeObject *) PepRun_GetResult(prog, "StaticMethodType");
}

#endif // Py_LIMITED_API

/*****************************************************************************
 *
 * These functions are always needed due to the heaptypes.
 *
 */

PyObject *
PepType_FromSpecWithBases(PyType_Spec *spec, PyObject *bases, int extra)
{
    _PepTypeObject *type = reinterpret_cast<_PepTypeObject*>(&PyType_Type);
    type->tp_basicsize += extra;
    PyObject *ret = PyType_FromSpecWithBases(spec, bases);
    type->tp_basicsize -= extra;
    return ret;
}

PyObject *
PepType_FromSpec(PyType_Spec *spec, int extra)
{
    return PepType_FromSpecWithBases(spec, nullptr, extra);
}

const char *
PepType_GetNameStr(PyTypeObject *type)
{
    // We behave like tp_name, but remove any leading dotted parts.
#ifdef Py_LIMITED_API
    const char *ret = PepType_tp_name(type);
#else
    const char *ret = type->tp_name;
#endif
    const char *nodots = strrchr(ret, '.');
    if (nodots)
        ret = nodots + 1;
    return ret;
}

/*****************************************************************************
 *
 * Module Initialization
 *
 */

void
Pep_Init()
{
#ifdef Py_LIMITED_API
    Pep_GetVerboseFlag();
    PepMethod_TypePtr = getMethodType();
    PepFunction_TypePtr = getFunctionType();
    PepStaticMethod_TypePtr = getStaticMethodType();
#endif
}

} // extern "C"
