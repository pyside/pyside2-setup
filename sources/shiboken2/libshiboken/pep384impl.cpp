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

extern "C"
{

/**********************************************************************
 **********************************************************************


    The New Type API
    ================

    After converting everything but the "object.h" file, we could not
    believe our eyes: it suddenly was clear that we would have no more
    access to type objects, and even more scary that all types which we
    use have to be heap types, only!

    For PySide with it's intense use of heap type extensions in various
    flavors, it seemed to be quite unsolvable. In the end, it was
    nicely solved, but it took almost 3.5 months to get that right.

    Before we see how this is done, we will explain the differences
    between the APIs and their consequences.


    The Interface
    -------------

    The old type API of Python knows static types and heap types.
    Static types are written down as a declaration of a PyTypeObject
    structure with all its fields filled in. Here is for example
    the definition of the Python type "object":

        PyTypeObject PyBaseObject_Type = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "object",                                   |* tp_name *|
            sizeof(PyObject),                           |* tp_basicsize *|
            0,                                          |* tp_itemsize *|
            object_dealloc,                             |* tp_dealloc *|
            0,                                          |* tp_print *|
            0,                                          |* tp_getattr *|
            0,                                          |* tp_setattr *|
            0,                                          |* tp_reserved *|
            object_repr,                                |* tp_repr *|
            0,                                          |* tp_as_number *|
            0,                                          |* tp_as_sequence *|
            0,                                          |* tp_as_mapping *|
            (hashfunc)_Py_HashPointer,                  |* tp_hash *|
            0,                                          |* tp_call *|
            object_str,                                 |* tp_str *|
            PyObject_GenericGetAttr,                    |* tp_getattro *|
            PyObject_GenericSetAttr,                    |* tp_setattro *|
            0,                                          |* tp_as_buffer *|
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   |* tp_flags *|
            PyDoc_STR("object()\n--\n\nThe most base type"),  |* tp_doc *|
            0,                                          |* tp_traverse *|
            0,                                          |* tp_clear *|
            object_richcompare,                         |* tp_richcompare *|
            0,                                          |* tp_weaklistoffset *|
            0,                                          |* tp_iter *|
            0,                                          |* tp_iternext *|
            object_methods,                             |* tp_methods *|
            0,                                          |* tp_members *|
            object_getsets,                             |* tp_getset *|
            0,                                          |* tp_base *|
            0,                                          |* tp_dict *|
            0,                                          |* tp_descr_get *|
            0,                                          |* tp_descr_set *|
            0,                                          |* tp_dictoffset *|
            object_init,                                |* tp_init *|
            PyType_GenericAlloc,                        |* tp_alloc *|
            object_new,                                 |* tp_new *|
            PyObject_Del,                               |* tp_free *|
        };

    We can write the same structure in form of a PyType_Spec structure,
    and there is even a tool that does this for us, but I had to fix a
    few things because there is little support for this.

    The tool is XXX go home and continue.....




    The Transition To Simpler Types
    ===============================

    After all code has been converted to the limited API, there is the
    PyHeapTypeObject remaining as a problem.

    Why a problem? Well, all the type structures in shiboken use
    special extra fields at the end of the heap type object. This
    currently enforces knowledge at compile time about how large the
    heap type object is. In a clean implementation, we would only use
    the PyTypeObject itself and access the fields "behind" the type
    by a pointer that is computed at runtime.


    Excursion: PepTypeObject
    ------------------------

    Before we are going into details, let us motivate the existence of
    the PepTypeObject, an alias to PyTypeObject:

    Originally, we wanted to use PyTypeObject as an opaque type and
    restrict ourselves to only use the access function PyType_GetSlot.
    This function allows access to all fields which are supported by
    the limited API.

    But this is a restriction, because we get no access to tp_dict,
    which we need to support the signature extension. But we can work
    around that.

    The real restriction is that PyType_GetSlot only works for heap
    types. This makes the function quite useless, because we have
    no access to PyType_Type, which is the most important type "type"
    in Python. We need that for instance to compute the size of
    PyHeapTypeObject dynamically.

    With much effort, it is possible to clone PyType_Type as a heap
    type. But due to a bug in the Pep 384 support, we need
    access to the nb_index field of a normal type. Cloning does not
    help because PyNumberMethods fields are not inherited.

    After I realized this dead end, I changed the concept and did not
    use PyType_GetSlot at all (except in function copyNumberMethods),
    but created PepTypeObject as a remake of PyTypeObject with only
    those fields defined that are needed in PySide.

    Is this breakage of the limited API? I don't think so. A special
    function runs on program startup that checks the correct position
    of the fields of PepHeapType, although a change in those fields is
    more than unlikely.
    The really crucial thing is to no longer use PyHeapTypeObject
    explicitly because that _does_ change its layout over time.


    Diversification
    ---------------

    There are multiple SbkXXX structures which all use a "d" field
    for their private data. This makes it not easy to find the right
    fields when switching between types and objects.

        struct LIBSHIBOKEN_API SbkObjectType
        {
            PyHeapTypeObject super;
            SbkObjectTypePrivate *d;
        };

        struct LIBSHIBOKEN_API SbkObject
        {
            PyObject_HEAD
            PyObject *ob_dict;
            PyObject *weakreflist;
            SbkObjectPrivate *d;
        };

    The first step was to rename the SbkObjectTypePrivate from "d" to
    "sotp". It was chosen to be short but easy to remember.


    Abstraction
    -----------

    After renaming the type extension pointers to "sotp", I replaced
    them by function-like macros which did the special access "behind"
    the types, instead of those explicit fields. For instance, the
    expression

        type->sotp->converter

    became

        PepType_SOTP(type)->converter

    The macro expression can be seen here:

    #define _genericTypeExtender(etype) \
        (reinterpret_cast<char*>(etype) + \
            (reinterpret_cast<PepTypeObject *>(&PyType_Type))->tp_basicsize)

    #define PepType_SOTP(etype) \
        (*reinterpret_cast<SbkObjectTypePrivate**>(_genericTypeExtender(etype)))

    It looks complicated, but in the end there is only a single new
    indirection via PyType_Type, which happens at runtime. This is the
    key to fulfil what Pep 384 wants: No version-dependent fields.


    Simplification
    --------------

    After all type extension fields were replaced by macro calls, we
    could remove the version dependent definition

        typedef struct _pepheaptypeobject {
            union {
                PepTypeObject ht_type;
                void *opaque[PY_HEAPTYPE_SIZE];
            };
        } PepHeapTypeObject;

    and the version dependent structure

        struct LIBSHIBOKEN_API SbkObjectType
        {
            PepHeapTypeObject super;
            SbkObjectTypePrivate *sotp;
        };

    could be replaced by the simplified

        struct LIBSHIBOKEN_API SbkObjectType
        {
            PepTypeObject type;
        };

    which is no longer version-dependent.


    Verification Of PepTypeObject
    =============================

    We have introduced PepTypeObject as a new alias for PyTypeObject,
    and now we need to prove that we are allowed to do so.

    When using the limited API as intended, then types are completely
    opaque, and access is only through PyType_FromSpec and (from
    version 3.5 upwards) through PyType_GetSlot.

    Python then uses all the slot definitions in the type description
    and produces a regular type object.


    Unused Information
    ------------------

    But we know many things about types that are not explicitly said,
    but they are inherently clear:

     a) The basic structure of a type is always the same, regardless
        if it is a static type or a heap type.

     b) types are evolving very slowly, and a field is never replaced
        by another field with different semantics.

    Inherent rule a) gives us the following information: If we calculate
    the offsets of the fields, then this info is also usable for non-
    -heap types.

    The validation checks if rule b) is still valid.


    How it Works
    ------------

    The basic idea of the validation is to produce a new type using
    PyType_FromSpec and to see where in the type structure these fields
    show up. So we build a PyType_Slot structure with all the fields we
    are using and make sure that these values are all unique in the
    type.

    Most fields are not investigated by PyType_FromSpec, and so we
    simply used some numeric value. Some fields are interpreted, like
    tp_members. This field must really be a PyMemberDef. And there are
    tp_base and tp_bases which have to be type objects and lists
    thereof. It was easiest to not produce these fields from scratch
    but use them from the "type" object PyType_Type.

    Then one would think to write a function that searches the known
    values in the opaque type structure.

    But we can do better and use optimistically the observation (b):
    We simply use the PepTypeObject structure and assume that every
    field lands exactly where we are awaiting it.

    And that is the whole proof: If we find all the disjoint values at
    the places where we expect them, thenthis is q.e.d. :)


    About tp_dict
    -------------

    One word about the tp_dict field: This field is a bit special in
    the proof, since it does not appear in the spec and cannot easily
    be checked by "type.__dict__" because that creates a dictproxy
    object. So how do we proove that is really the right dict?

    We have to create that PyMethodDef structure anyway, and instead of
    leaving it empty, we insert a dummy function. Then we ask the
    tp_dict field if it has that object in it, and that's q.e.d.


 *********/


/*****************************************************************************
 *
 * Support for object.h
 *
 */

/*
 * Here is the verification code for PepTypeObject.
 * We create a type object and check if its fields
 * appear at the right offsets.
 */

#define make_dummy_int(x)   (x * sizeof(void*))
#define make_dummy(x)       (reinterpret_cast<void*>(make_dummy_int(x)))

#ifdef Py_LIMITED_API
datetime_struc *PyDateTimeAPI = NULL;
#endif

static PyObject *
dummy_func(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
}

static struct PyMethodDef probe_methoddef[] = {
    {"dummy", dummy_func, METH_NOARGS},
    {0}
};

#define probe_tp_call       make_dummy(1)
#define probe_tp_str        make_dummy(2)
#define probe_tp_traverse   make_dummy(3)
#define probe_tp_clear      make_dummy(4)
#define probe_tp_methods    probe_methoddef
#define probe_tp_descr_get  make_dummy(6)
#define probe_tp_init       make_dummy(7)
#define probe_tp_alloc      make_dummy(8)
#define probe_tp_new        make_dummy(9)
#define probe_tp_free       make_dummy(10)
#define probe_tp_is_gc      make_dummy(11)

#define probe_tp_name       "type.probe"
#define probe_tp_basicsize  make_dummy_int(42)

static PyType_Slot typeprobe_slots[] = {
    {Py_tp_call,        probe_tp_call},
    {Py_tp_str,         probe_tp_str},
    {Py_tp_traverse,    probe_tp_traverse},
    {Py_tp_clear,       probe_tp_clear},
    {Py_tp_methods,     probe_tp_methods},
    {Py_tp_descr_get,   probe_tp_descr_get},
    {Py_tp_init,        probe_tp_init},
    {Py_tp_alloc,       probe_tp_alloc},
    {Py_tp_new,         probe_tp_new},
    {Py_tp_free,        probe_tp_free},
    {Py_tp_is_gc,       probe_tp_is_gc},
    {0, 0}
};
static PyType_Spec typeprobe_spec = {
    probe_tp_name,
    probe_tp_basicsize,
    0,
    Py_TPFLAGS_DEFAULT,
    typeprobe_slots,
};

static void
check_PepTypeObject_valid(void)
{
    PyObject *obtype = reinterpret_cast<PyObject *>(&PyType_Type);
    PyTypeObject *probe_tp_base = reinterpret_cast<PyTypeObject *>(
        PyObject_GetAttrString(obtype, "__base__"));
    PyObject *probe_tp_bases = PyObject_GetAttrString(obtype, "__bases__");
    PyTypeObject *check = reinterpret_cast<PyTypeObject *>(
        PyType_FromSpecWithBases(&typeprobe_spec, probe_tp_bases));
    PyTypeObject *typetype = reinterpret_cast<PyTypeObject *>(obtype);
    PyObject *w = PyObject_GetAttrString(obtype, "__weakrefoffset__");
    long probe_tp_weakrefoffset = PyLong_AsLong(w);
    PyObject *d = PyObject_GetAttrString(obtype, "__dictoffset__");
    long probe_tp_dictoffset = PyLong_AsLong(d);
    PyObject *probe_tp_mro = PyObject_GetAttrString(obtype, "__mro__");
    if (false
        || probe_tp_name            != check->tp_name
        || probe_tp_basicsize       != check->tp_basicsize
        || probe_tp_call            != check->tp_call
        || probe_tp_str             != check->tp_str
        || probe_tp_traverse        != check->tp_traverse
        || probe_tp_clear           != check->tp_clear
        || probe_tp_weakrefoffset   != typetype->tp_weaklistoffset
        || probe_tp_methods         != check->tp_methods
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
    PyObject *code = Py_CompileString(str, "pyscript", start);
    PyObject *ret = NULL;

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
 * Module Initialization
 *
 */

void
Pep384_Init()
{
    check_PepTypeObject_valid();
#ifdef Py_LIMITED_API
    Pep_GetVerboseFlag();
    PepMethod_TypePtr = getMethodType();
    PepFunction_TypePtr = getFunctionType();
    PepStaticMethod_TypePtr = getStaticMethodType();
#endif
}

} // extern "C"
