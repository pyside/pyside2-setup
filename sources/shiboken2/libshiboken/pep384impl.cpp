/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "pep384impl.h"
#include "autodecref.h"

extern "C"
{

/*
 * The documentation is located in pep384impl_doc.rst
 */

/*****************************************************************************
 *
 * Support for object.h
 *
 */

/*
 * Here is the verification code for PyTypeObject.
 * We create a type object and check if its fields
 * appear at the right offsets.
 */

#define make_dummy_int(x)   (x * sizeof(void *))
#define make_dummy(x)       (reinterpret_cast<void *>(make_dummy_int(x)))

#ifdef Py_LIMITED_API
datetime_struc *PyDateTimeAPI = NULL;
#endif

static PyObject *
dummy_func(PyObject * /* self */, PyObject * /* args */)
{
    Py_RETURN_NONE;
}

static struct PyMethodDef probe_methoddef[] = {
    {"dummy", dummy_func, METH_NOARGS},
    {nullptr}
};

static PyGetSetDef probe_getseters[] = {
    {nullptr}  /* Sentinel */
};

#define probe_tp_call       make_dummy(1)
#define probe_tp_str        make_dummy(2)
#define probe_tp_traverse   make_dummy(3)
#define probe_tp_clear      make_dummy(4)
#define probe_tp_methods    probe_methoddef
#define probe_tp_getset     probe_getseters
#define probe_tp_descr_get  make_dummy(7)
#define probe_tp_init       make_dummy(8)
#define probe_tp_alloc      make_dummy(9)
#define probe_tp_new        make_dummy(10)
#define probe_tp_free       make_dummy(11)
#define probe_tp_is_gc      make_dummy(12)

#define probe_tp_name       "type.probe"
#define probe_tp_basicsize  make_dummy_int(42)

static PyType_Slot typeprobe_slots[] = {
    {Py_tp_call,        probe_tp_call},
    {Py_tp_str,         probe_tp_str},
    {Py_tp_traverse,    probe_tp_traverse},
    {Py_tp_clear,       probe_tp_clear},
    {Py_tp_methods,     probe_tp_methods},
    {Py_tp_getset,      probe_tp_getset},
    {Py_tp_descr_get,   probe_tp_descr_get},
    {Py_tp_init,        probe_tp_init},
    {Py_tp_alloc,       probe_tp_alloc},
    {Py_tp_new,         probe_tp_new},
    {Py_tp_free,        probe_tp_free},
    {Py_tp_is_gc,       probe_tp_is_gc},
    {0, nullptr}
};
static PyType_Spec typeprobe_spec = {
    probe_tp_name,
    probe_tp_basicsize,
    0,
    Py_TPFLAGS_DEFAULT,
    typeprobe_slots,
};

static void
check_PyTypeObject_valid()
{
    auto *obtype = reinterpret_cast<PyObject *>(&PyType_Type);
    auto *probe_tp_base = reinterpret_cast<PyTypeObject *>(
        PyObject_GetAttrString(obtype, "__base__"));
    PyObject *probe_tp_bases = PyObject_GetAttrString(obtype, "__bases__");
    auto *check = reinterpret_cast<PyTypeObject *>(
        PyType_FromSpecWithBases(&typeprobe_spec, probe_tp_bases));
    auto *typetype = reinterpret_cast<PyTypeObject *>(obtype);
    PyObject *w = PyObject_GetAttrString(obtype, "__weakrefoffset__");
    long probe_tp_weakrefoffset = PyLong_AsLong(w);
    PyObject *d = PyObject_GetAttrString(obtype, "__dictoffset__");
    long probe_tp_dictoffset = PyLong_AsLong(d);
    PyObject *probe_tp_mro = PyObject_GetAttrString(obtype, "__mro__");
    if (false
        || strcmp(probe_tp_name, check->tp_name) != 0
        || probe_tp_basicsize       != check->tp_basicsize
        || probe_tp_call            != check->tp_call
        || probe_tp_str             != check->tp_str
        || probe_tp_traverse        != check->tp_traverse
        || probe_tp_clear           != check->tp_clear
        || probe_tp_weakrefoffset   != typetype->tp_weaklistoffset
        || probe_tp_methods         != check->tp_methods
        || probe_tp_getset          != check->tp_getset
        || probe_tp_base            != typetype->tp_base
        || !PyDict_Check(check->tp_dict)
        || !PyDict_GetItemString(check->tp_dict, "dummy")
        || probe_tp_descr_get       != check->tp_descr_get
        || probe_tp_dictoffset      != typetype->tp_dictoffset
        || probe_tp_init            != check->tp_init
        || probe_tp_alloc           != check->tp_alloc
        || probe_tp_new             != check->tp_new
        || probe_tp_free            != check->tp_free
        || probe_tp_is_gc           != check->tp_is_gc
        || probe_tp_bases           != typetype->tp_bases
        || probe_tp_mro             != typetype->tp_mro)
        Py_FatalError("The structure of type objects has changed!");
    Py_DECREF(check);
    Py_DECREF(probe_tp_base);
    Py_DECREF(w);
    Py_DECREF(d);
    Py_DECREF(probe_tp_bases);
    Py_DECREF(probe_tp_mro);
}


#ifdef Py_LIMITED_API

#if PY_VERSION_HEX < PY_ISSUE33738_SOLVED
#include "pep384_issue33738.cpp"
#endif

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
     * produces a pseudo-leak as long as there are new strings.
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
    PyObject *code = Py_CompileString(str, "pyscript", start);
    PyObject *ret = NULL;

    if (code != NULL) {
        ret = PyEval_EvalCode(code, globals, locals);
    }
    Py_XDECREF(code);
    return ret;
}

#endif // Py_LIMITED_API

// This is only a simple local helper that returns a computed variable.
// Used also in Python 2.
#if defined(Py_LIMITED_API) || PY_VERSION_HEX < 0x03000000
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
#endif // Py_LIMITED_API || Python 2

#ifdef Py_LIMITED_API

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

// This became necessary after Windows was activated.

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

static PyTypeObject *
getStaticMethodType(void)
{
    // this works for Python 3, only
    //    "StaticMethodType = type(str.__dict__['maketrans'])\n";
    static const char prog[] =
        "from xxsubtype import spamlist\n"
        "StaticMethod_Type = type(spamlist.__dict__['staticmeth'])\n";
    return (PyTypeObject *) PepRun_GetResult(prog, "StaticMethod_Type");
}

typedef struct {
    PyObject_HEAD
    PyObject *sm_callable;
    PyObject *sm_dict;
} staticmethod;

PyObject *
PyStaticMethod_New(PyObject *callable)
{
    staticmethod *sm = (staticmethod *)
        PyType_GenericAlloc(PepStaticMethod_TypePtr, 0);
    if (sm != NULL) {
        Py_INCREF(callable);
        sm->sm_callable = callable;
    }
    return (PyObject *)sm;
}
#endif // Py_LIMITED_API

#if PY_VERSION_HEX < 0x03000000
PyTypeObject *PepMethodDescr_TypePtr = NULL;

static PyTypeObject *
getMethodDescrType(void)
{
    static const char prog[] =
        "MethodDescr_Type = type(str.split)\n";
    return (PyTypeObject *) PepRun_GetResult(prog, "MethodDescr_Type");
}
#endif

/*****************************************************************************
 *
 * Common newly needed functions
 *
 */

// The introduction of heaptypes converted many type names to the
// dotted form, since PyType_FromSpec uses it to compute the module
// name. This function reverts this effect.
const char *
PepType_GetNameStr(PyTypeObject *type)
{
    const char *ret = type->tp_name;
    const char *nodots = strrchr(ret, '.');
    if (nodots)
        ret = nodots + 1;
    return ret;
}

/*****************************************************************************
 *
 * Extra support for name mangling
 *
 */

#ifdef Py_LIMITED_API
// We keep these definitions local, because they don't work in Python 2.
#define PyUnicode_GET_LENGTH(op)    PyUnicode_GetLength((PyObject *)(op))
#define PyUnicode_READ_CHAR(u, i)   PyUnicode_ReadChar((PyObject *)(u), (i))
#endif

PyObject *
_Pep_PrivateMangle(PyObject *self, PyObject *name)
{
    /*
     * Name mangling: __private becomes _classname__private.
     * This function is modelled after _Py_Mangle, but is optimized
     * a little for our purpose.
     */
#if PY_VERSION_HEX < 0X03000000
    const char *namestr = PyString_AsString(name);
    if (namestr == NULL || namestr[0] != '_' || namestr[1] != '_') {
        Py_INCREF(name);
        return name;
    }
    size_t nlen = strlen(namestr);
    /* Don't mangle __id__ or names with dots. */
    if ((namestr[nlen-1] == '_' && namestr[nlen-2] == '_')
        || strchr(namestr, '.')) {
        Py_INCREF(name);
        return name;
    }
#else
    if (PyUnicode_READ_CHAR(name, 0) != '_' ||
        PyUnicode_READ_CHAR(name, 1) != '_') {
        Py_INCREF(name);
        return name;
    }
    size_t nlen = PyUnicode_GET_LENGTH(name);
    /* Don't mangle __id__ or names with dots. */
    if ((PyUnicode_READ_CHAR(name, nlen-1) == '_' &&
         PyUnicode_READ_CHAR(name, nlen-2) == '_') ||
        PyUnicode_FindChar(name, '.', 0, nlen, 1) != -1) {
        Py_INCREF(name);
        return name;
    }
#endif
    Shiboken::AutoDecRef privateobj(PyObject_GetAttrString(
        reinterpret_cast<PyObject *>(Py_TYPE(self)), "__name__"));
#ifndef Py_LIMITED_API
    return _Py_Mangle(privateobj, name);
#else
    // For some reason, _Py_Mangle is not in the Limited API. Why?
    size_t plen = PyUnicode_GET_LENGTH(privateobj);
    /* Strip leading underscores from class name */
    size_t ipriv = 0;
    while (PyUnicode_READ_CHAR(privateobj, ipriv) == '_')
        ipriv++;
    if (ipriv == plen) {
        Py_INCREF(name);
        return name; /* Don't mangle if class is just underscores */
    }
    plen -= ipriv;

    if (plen + nlen >= PY_SSIZE_T_MAX - 1) {
        PyErr_SetString(PyExc_OverflowError,
                        "private identifier too large to be mangled");
        return NULL;
    }
    size_t const amount = ipriv + 1 + plen + nlen;
    size_t const big_stack = 1000;
    wchar_t bigbuf[big_stack];
    wchar_t *resbuf = amount <= big_stack ? bigbuf : (wchar_t *)malloc(sizeof(wchar_t) * amount);
    if (!resbuf)
        return 0;
    /* ident = "_" + priv[ipriv:] + ident # i.e. 1+plen+nlen bytes */
    resbuf[0] = '_';
    if (PyUnicode_AsWideChar(privateobj, resbuf + 1, ipriv + plen) < 0)
        return 0;
    if (PyUnicode_AsWideChar(name, resbuf + ipriv + plen + 1, nlen) < 0)
        return 0;
    PyObject *result = PyUnicode_FromWideChar(resbuf + ipriv, 1 + plen + nlen);
    if (amount > big_stack)
        free(resbuf);
    return result;
#endif  // Py_LIMITED_API
}

/*****************************************************************************
 *
 * Module Initialization
 *
 */

void
Pep384_Init()
{
    check_PyTypeObject_valid();
#ifdef Py_LIMITED_API
    Pep_GetVerboseFlag();
    PepMethod_TypePtr = getMethodType();
    PepFunction_TypePtr = getFunctionType();
    PepStaticMethod_TypePtr = getStaticMethodType();
#endif
#if PY_VERSION_HEX < 0x03000000
    PepMethodDescr_TypePtr = getMethodDescrType();
#endif
}

} // extern "C"
