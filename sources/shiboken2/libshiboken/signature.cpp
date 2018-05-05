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

#include "basewrapper.h"

extern "C"
{

/***************************************************************************
 ***************************************************************************


 The signature C extension
 =========================

 This module is a C extension for CPython 3.4 and up, and CPython 2.7.
 It's purpose is to provide support for the __signature__ attribute
 of builtin PyCFunction objects.


 Short excursion on the topic
 ----------------------------

 Beginning with CPython 3.5, Python functions began to grow a __signature__
 attribute for normal Python functions. This is totally optional and just
 a nice-to-have feature in Python.

 PySide, on the other hand, could use __signature__ very much, because the
 typing info for the 14000+ PySide functions is really missing, and it
 would be nice to have this info available directly in Python.


 How this code works
 -------------------

 The basic idea is to create a dummy Python function and to use the inspect
 module to create a signature object. Then, this object is returned as the
 result of the __signature__ attribute of the real PyCFunction.

 There is one thing that really changes Python a bit:

     I added the __signature__ attribute to every function.

 That is a little change to Python that does not harm, but it saves us
 tons of code, that was needed in the former versions.

 The internal work is done in two steps:
 All functions get their "signature text" when the module is imported.
 The actual signature is created later, when the attribute is really used.

 Example:

 The PyCFunction 'QtWidgets.QApplication.palette' is interrogated for its
 signature. That means 'pyside_sm_get___signature__()' is called.
 It calls GetSignature_Function which returns the signature if it is found.

 There are actually 2 locations where late initialization occurs:
 -  'dict' can be no dict but a tuple. That is the argument tuple that
    was saved by 'PySide_BuildSignatureArgs' at module load time.
    If so, then 'pyside_type_init' in 'signature.py' will be called,
    which parses the string and creates the dict.
 -  'props' can be empty. Then 'create_signature' in 'signature_loader.py'
    is called, which uses a dummy function to produce a signature instance
    with the inspect module.

 This module is dedicated to our lovebird "Püppi", who died on 2017-09-15.

 ****************************************************************************
 ****************************************************************************/

#include "signature.h"
#include <structmember.h>

#define EXTENSION_ENABLED \
    PY_VERSION_HEX >= 0x03040000 || \
    (PY_VERSION_HEX < 0x03000000 && PY_VERSION_HEX >= 0x02070000)

#if EXTENSION_ENABLED

// These constants were needed in former versions of the module:
#define PYTHON_HAS_QUALNAME             (PY_VERSION_HEX >= 0x03030000)
#define PYTHON_HAS_UNICODE              (PY_VERSION_HEX >= 0x03000000)
#define PYTHON_HAS_WEAKREF_PYCFUNCTION  (PY_VERSION_HEX >= 0x030500A0)
#define PYTHON_IS_PYTHON3               (PY_VERSION_HEX >= 0x03000000)
#define PYTHON_HAS_KEYWORDONLY          (PYTHON_IS_PYTHON3)
#define PYTHON_USES_PERCENT_V_FORMAT    (PYTHON_IS_PYTHON3)
#define PYTHON_HAS_DESCR_REDUCE         (PY_VERSION_HEX >= 0x03040000)
#define PYTHON_HAS_METH_REDUCE          (PYTHON_HAS_DESCR_REDUCE)
#define PYTHON_NEEDS_ITERATOR_FLAG      (!PYTHON_IS_PYTHON3)
#define PYTHON_EXPOSES_METHODDESCR      (PYTHON_IS_PYTHON3)
#define PYTHON_NO_TYPE_IN_FUNCTIONS     (!PYTHON_IS_PYTHON3 || Py_LIMITED_API)

// These constants are still in use:
#define PYTHON_USES_D_COMMON            (PY_VERSION_HEX >= 0x03020000)

typedef struct safe_globals_struc {
    // init part 1: get arg_dict
    PyObject *helper_module;
    PyObject *arg_dict;
    PyObject *map_dict;
    // init part 2: run module
    PyObject *sigparse_func;
    PyObject *createsig_func;
} safe_globals_struc, *safe_globals;

static safe_globals pyside_globals = 0;

static PyObject *GetSignature_Function(PyCFunctionObject *);
static PyObject *GetSignature_TypeMod(PyObject *);

static PyObject *PySide_BuildSignatureProps(PyObject *class_mod);

const char helper_module_name[] = "signature_loader";
const char bootstrap_name[] = "bootstrap";
const char arg_name[] = "pyside_arg_dict";
const char func_name[] = "pyside_type_init";

static PyObject *
CreateSignature(PyObject *props, const char *sig_kind)
{
    /*
     * Here is the new function to create all signatures. It simply calls
     * into Python and creates a signature object for a dummy-function.
     * This is so much simpler than using all the attributes explicitly
     * to support '_signature_is_functionlike()'.
     */
    return PyObject_CallFunction(pyside_globals->createsig_func,
                                 (char *)"(Os)", props, sig_kind);
}

static PyObject *
pyside_cf_get___signature__(PyObject *func)
{
    return GetSignature_Function((PyCFunctionObject *)func);
}

static PyObject *
pyside_sm_get___signature__(PyObject *sm)
{
    PyObject *func, *ret;

    func = PyObject_GetAttrString(sm, "__func__");
    ret = GetSignature_Function((PyCFunctionObject *)func);
    Py_XDECREF(func);
    return ret;
}

#ifdef Py_LIMITED_API

static int
build_qualname_to_func(PyObject *obtype)
{
    PyTypeObject *type = (PyTypeObject *)obtype;
    PyMethodDef *meth = PepType_tp_methods(type);

    if (meth == 0)
        return 0;

    for (; meth->ml_name != NULL; meth++) {
        PyObject *func = PyCFunction_NewEx(meth, obtype, NULL);
        PyObject *qualname = PyObject_GetAttrString(func, "__qualname__");
        if (func == NULL || qualname == NULL) {
            return -1;
        }
        if (PyDict_SetItem(pyside_globals->map_dict, qualname, func) < 0) {
            return -1;
        }
        Py_DECREF(func);
        Py_DECREF(qualname);
    }
    return 0;
}

static PyObject *
qualname_to_typename(PyObject *qualname)
{
    PyObject *func = PyObject_GetAttrString(qualname, "split");
    PyObject *list = func ? PyObject_CallFunction(func, (char *)"(s)", ".")
                          : NULL;
    PyObject *res = list ? PyList_GetItem(list, 0) : NULL;
    Py_XINCREF(res);
    Py_XDECREF(func);
    Py_XDECREF(list);
    return res;
}

static PyObject *
qualname_to_func(PyObject *ob)
{
    /*
     * If we have __qualname__, then we can easily build a mapping
     * from __qualname__ to PyCFunction. This is necessary when
     * the limited API does not let us go easily from descriptor
     * to PyMethodDef.
     */
    PyObject *ret;
    PyObject *qualname = PyObject_GetAttrString((PyObject *)ob,
                                                "__qualname__");
    if (qualname != NULL) {
        ret = PyDict_GetItem(pyside_globals->map_dict, qualname);
        if (ret == NULL) {
            // do a lazy initialization
            PyObject *type_name = qualname_to_typename(qualname);
            PyObject *type = PyDict_GetItem(pyside_globals->map_dict,
                                            type_name);
            Py_XDECREF(type_name);
            if (type == NULL)
                Py_RETURN_NONE;
            if (build_qualname_to_func(type) < 0)
                return NULL;
            ret = PyDict_GetItem(pyside_globals->map_dict, qualname);
        }
        Py_XINCREF(ret);
        Py_DECREF(qualname);
    }
    else
        Py_RETURN_NONE;
    return ret;
}
#endif

static PyObject *
pyside_md_get___signature__(PyObject *ob)
{
    PyObject *func;
    PyObject *result;
#ifndef Py_LIMITED_API
    PyMethodDescrObject *descr = (PyMethodDescrObject *)ob;

# if PYTHON_USES_D_COMMON
    func = PyCFunction_NewEx(descr->d_method,
                             (PyObject *)descr->d_common.d_type, NULL);
# else
    func = PyCFunction_NewEx(descr->d_method,
                             (PyObject *)descr->d_type, NULL);
# endif
#else
    /*
     * With limited access, we cannot use the fields of a method descriptor,
     * but in Python 3 we have the __qualname__ field which allows us to
     * grab the method object from our registry.
     */
    func = qualname_to_func(ob);
#endif
    if (func == Py_None)
        return Py_None;
    if (func == NULL)
        Py_FatalError("missing mapping in MethodDescriptor");
    result = pyside_cf_get___signature__(func);
    Py_DECREF(func);
    return result;
}

static PyObject *
pyside_tp_get___signature__(PyObject *typemod)
{
    return GetSignature_TypeMod(typemod);
}

static PyObject *
GetSignature_Function(PyCFunctionObject *func)
{
    PyObject *typemod, *type_name, *dict, *props, *value, *selftype;
    PyObject *func_name = PyObject_GetAttrString((PyObject *)func, "__name__");
    const char *sig_kind;
    int flags;

    selftype = PyCFunction_GET_SELF((PyObject *)func);
    if (selftype == NULL)
        selftype = PyDict_GetItem(pyside_globals->map_dict, (PyObject *)func);
    if (selftype == NULL) {
        if (!PyErr_Occurred()) {
            PyErr_Format(PyExc_SystemError,
                "the signature for \"%s\" should exist",
                PepCFunction_GET_NAMESTR(func)
                );
        }
        return NULL;
    }
    if ((PyType_Check(selftype) || PyModule_Check(selftype)))
        typemod = selftype;
    else
        typemod = (PyObject *)Py_TYPE(selftype);
    type_name = PyObject_GetAttrString(typemod, "__name__");
    if (type_name == NULL)
        Py_RETURN_NONE;
    dict = PyDict_GetItem(pyside_globals->arg_dict, type_name);
    Py_DECREF(type_name);
    if (dict == NULL)
        Py_RETURN_NONE;
    if (PyTuple_Check(dict)) {
        /*
         * We do the initialization lazily.
         * This has also the advantage that we can freely import PySide.
         */
        dict = PySide_BuildSignatureProps(typemod);
        if (dict == NULL)
            Py_RETURN_NONE;
    }
    props = PyDict_GetItem(dict, func_name);
    if (props == NULL)
        Py_RETURN_NONE;
    flags = PyCFunction_GET_FLAGS((PyObject *)func);
    if (flags & METH_CLASS)
        sig_kind = "classmethod";
    else if (flags & METH_STATIC)
        sig_kind = "staticmethod";
    else
        sig_kind = "method";
    value = PyDict_GetItemString(props, sig_kind);
    if (value == NULL) {
        // we need to compute a signature object
        value = CreateSignature(props, sig_kind);
        if (value != NULL) {
            if (PyDict_SetItemString(props, sig_kind, value) < 0)
                return NULL;
        }
        else
            Py_RETURN_NONE;
    }
    return Py_INCREF(value), value;
}

static PyObject *
GetSignature_TypeMod(PyObject *ob)
{
    PyObject *ob_name, *dict, *props, *value;
    const char *sig_kind;

    ob_name = PyObject_GetAttrString(ob, "__name__");
    dict = PyDict_GetItem(pyside_globals->arg_dict, ob_name);
    if (dict == NULL)
        Py_RETURN_NONE;
    if (PyTuple_Check(dict)) {
        dict = PySide_BuildSignatureProps(ob);
        if (dict == NULL) {
            Py_RETURN_NONE;
        }
    }
    props = PyDict_GetItem(dict, ob_name);
    Py_DECREF(ob_name);
    if (props == NULL)
        Py_RETURN_NONE;
    sig_kind = "method";
    value = PyDict_GetItemString(props, sig_kind);
    if (value == NULL) {
        // we need to compute a signature object
        value = CreateSignature(props, sig_kind);
        if (value != NULL) {
            if (PyDict_SetItemString(props, sig_kind, value) < 0)
                return NULL;
        }
        else
            Py_RETURN_NONE;
    }
    return Py_INCREF(value), value;
}


static const char PySide_PythonCode[] =
    "from __future__ import print_function, absolute_import\n"
    "import sys, os, traceback\n"

    "pyside_package_dir = os.environ.get('PYSIDE_PACKAGE_DIR', '.')\n"
    "__file__ = os.path.join(pyside_package_dir, 'support', 'signature', 'loader.py')\n"

    "def bootstrap():\n"
    "    try:\n"
    "        with open(__file__) as _f:\n"
    "            exec(compile(_f.read(), __file__, 'exec'))\n"
    "    except Exception as e:\n"
    "        print('Exception:', e)\n"
    "        traceback.print_exc(file=sys.stdout)\n"
    "    globals().update(locals())\n"
     ;

static safe_globals_struc *
init_phase_1(void)
{
    safe_globals_struc *p;
    PyObject *d, *v;

    p = (safe_globals_struc *)malloc(sizeof(safe_globals_struc));
    if (p == NULL)
        goto error;
    p->helper_module = PyImport_AddModule((char *) helper_module_name);
    if (p->helper_module == NULL)
        goto error;

    // Initialize the module
    d = PyModule_GetDict(p->helper_module);
    if (PyDict_SetItemString(d, "__builtins__", PyEval_GetBuiltins()) < 0)
        goto error;
    v = PyRun_String(PySide_PythonCode, Py_file_input, d, d);
    if (v == NULL)
        goto error;
    Py_DECREF(v);

    // build a dict for diverse mappings
    p->map_dict = PyDict_New();
    if (p->map_dict == NULL)
        goto error;

    // Build a dict for the prepared arguments
    p->arg_dict = PyDict_New();
    if (p->arg_dict == NULL)
        goto error;
    if (PyObject_SetAttrString(p->helper_module, arg_name, p->arg_dict) < 0)
        goto error;
    return p;

error:
    PyErr_SetString(PyExc_SystemError, "could not initialize part 1");
    return NULL;
}

static int
init_phase_2(safe_globals_struc *p)
{
    PyObject *bootstrap_func;

    bootstrap_func = PyObject_GetAttrString(p->helper_module, bootstrap_name);
    if (bootstrap_func == NULL)
        goto error;
    if (PyObject_CallFunction(bootstrap_func, (char *)"()") == NULL)
        goto error;
    // now the loader is initialized
    p->sigparse_func = PyObject_GetAttrString(p->helper_module, func_name);
    if (p->sigparse_func == NULL)
        goto error;
    p->createsig_func = PyObject_GetAttrString(p->helper_module, "create_signature");
    if (p->createsig_func == NULL)
        goto error;
    return 0;

error:
    PyErr_SetString(PyExc_SystemError, "could not initialize part 2");
    return -1;
}

static int
add_more_getsets(PyTypeObject *type, PyGetSetDef *gsp)
{
    PyObject *dict = PepType_tp_dict(type);

    for (; gsp->name != NULL; gsp++) {
        PyObject *descr;
        if (PyDict_GetItemString(dict, gsp->name))
            continue;
        descr = PyDescr_NewGetSet(type, gsp);
        if (descr == NULL)
            return -1;
        if (PyDict_SetItemString(dict, gsp->name, descr) < 0) {
            Py_DECREF(descr);
            return -1;
        }
        Py_DECREF(descr);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
//
// Augmenting builtin types with a __signature__ attribute.
//
// This is a harmless change to Python, similar like __text_signature__.
// We could avoid it, but then we would need to copy quite some module
// initialization functions which are pretty version- and word size
// dependent. I think this little patch is the lesser of the two evils.
//
// Please note that in fact we are modifying 'type', the metaclass of all
// objects, because we add new functionality.
//
static PyGetSetDef new_PyCFunction_getsets[] = {
    {(char *) "__signature__", (getter)pyside_cf_get___signature__},
    {0}
};

static PyGetSetDef new_PyStaticMethod_getsets[] = {
    {(char *) "__signature__", (getter)pyside_sm_get___signature__},
    {0}
};

static PyGetSetDef new_PyMethodDescr_getsets[] = {
    {(char *) "__signature__", (getter)pyside_md_get___signature__},
    {0}
};

static PyGetSetDef new_PyType_getsets[] = {
    {(char *) "__signature__", (getter)pyside_tp_get___signature__},
    {0}
};

////////////////////////////////////////////////////////////////////////////
//
// This special Type_Ready does certain initializations earlier with
// our new version.
//

#ifndef _WIN32
////////////////////////////////////////////////////////////////////////////
// a stack trace for linux-like platforms
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {
    void *array[30];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 30);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

////////////////////////////////////////////////////////////////////////////
#endif // _WIN32

static int
PySideType_Ready(PyTypeObject *type)
{
    PyObject *md;
    static int init_done = 0;

    if (!init_done) {
        // Python2 does not expose certain types. We look them up:
        // PyMethodDescr_Type       'type(str.__dict__["split"])'
        // PyClassMethodDescr_Type. 'type(dict.__dict__["fromkeys"])'
        // The latter is not needed until we use class methods in PySide.
        md = PyObject_GetAttrString((PyObject *)&PyString_Type, "split");
        if (md == NULL
            || PyType_Ready(Py_TYPE(md)) < 0
            || add_more_getsets(Py_TYPE(md), new_PyMethodDescr_getsets) < 0
            || add_more_getsets(&PyCFunction_Type, new_PyCFunction_getsets) < 0
            || add_more_getsets(PepStaticMethod_TypePtr, new_PyStaticMethod_getsets) < 0
            || add_more_getsets(&PyType_Type, new_PyType_getsets) < 0)
            return -1;
        Py_DECREF(md);
#ifndef _WIN32
        // We enable the stack trace in CI, only.
        const char *testEnv = getenv("QTEST_ENVIRONMENT");
        if (testEnv && strstr(testEnv, "ci"))
            signal(SIGSEGV, handler);   // install our handler
#endif // _WIN32
        init_done = 1;
    }
    return PyType_Ready(type);
}

static int
build_func_to_type(PyObject *obtype)
{
    PyTypeObject *type = (PyTypeObject *)obtype;
    PyObject *dict = PepType_tp_dict(type);
    PyMethodDef *meth = PepType_tp_methods(type);

    if (meth == 0)
        return 0;

    for (; meth->ml_name != NULL; meth++) {
        if (meth->ml_flags & METH_STATIC) {
            PyObject *descr = PyDict_GetItemString(dict, meth->ml_name);
            if (descr == NULL)
                return -1;
            PyObject *func = PyObject_GetAttrString(descr, "__func__");
            if (func == NULL ||
                PyDict_SetItem(pyside_globals->map_dict, func, obtype) < 0)
                return -1;
            Py_DECREF(func);
        }
    }
    return 0;
}

static int
PySide_BuildSignatureArgs(PyObject *module, PyObject *type,
                          const char *signatures)
{
    PyObject *type_name, *arg_tup;
    const char *name = NULL;
    static int init_done = 0;

    if (!init_done) {
        pyside_globals = init_phase_1();
        if (pyside_globals == NULL)
            return -1;
        init_done = 1;
    }
    arg_tup = Py_BuildValue("(Os)", type, signatures);
    if (arg_tup == NULL)
        return -1;
    if (!PyModule_Check(module))
        return 0;
    name = PyModule_GetName(module);
    if (name == NULL)
        return -1;
    if (strncmp(name, "PySide2.Qt", 10) != 0)
        return 0;
    /*
     * Normally, we would now just call the Python function with the
     * arguments and then continue processing.
     * But it is much better to delay the second part until it is
     * really needed. Why?
     *
     * - by doing it late, we save initialization time when no signatures
     *   are requested,
     * - by calling the python function late, we can freely import PySide
     *   without recursion problems.
     */
    type_name = PyObject_GetAttrString(type, "__name__");
    if (type_name == NULL)
        return -1;
    if (PyDict_SetItem(pyside_globals->arg_dict, type_name, arg_tup) < 0)
        return -1;
    /*
     * We record also a mapping from type name to type. This helps to lazily
     * initialize the Py_LIMITED_API in qualname_to_func().
     */
    if (PyDict_SetItem(pyside_globals->map_dict, type_name, type) < 0)
        return -1;
    return 0;
}

static PyObject *
PySide_BuildSignatureProps(PyObject *classmod)
{
    PyObject *arg_tup, *dict, *type_name;
    static int init_done = 0;

    if (!init_done) {
        if (init_phase_2(pyside_globals) < 0)
            return NULL;
        init_done = 1;
    }
    /*
     * Here is the second part of the function.
     * This part will be called on-demand when needed by some attribute.
     * We simply pick up the arguments that we stored here and replace
     * them by the function result.
     */
    type_name = PyObject_GetAttrString(classmod, "__name__");
    if (type_name == NULL)
        return NULL;
    arg_tup = PyDict_GetItem(pyside_globals->arg_dict, type_name);
    if (arg_tup == NULL)
        return NULL;
    dict = PyObject_CallObject(pyside_globals->sigparse_func, arg_tup);
    if (dict == NULL)
        return NULL;

    // We replace the arguments by the result dict.
    if (PyDict_SetItem(pyside_globals->arg_dict, type_name, dict) < 0)
        return NULL;
    Py_DECREF(type_name);
    return dict;
}

#endif // EXTENSION_ENABLED

int
SbkSpecial_Type_Ready(PyObject *module, PyTypeObject *type,
                      const char *signatures)
{
    int ret;
#if EXTENSION_ENABLED
    if (PySideType_Ready(type) < 0)
        return -1;
    ret = PySide_BuildSignatureArgs(module, (PyObject *)type, signatures);
#else
    ret = PyType_Ready(type);
#endif
    if (ret < 0) {
        PyErr_Print();
        PyErr_SetNone(PyExc_ImportError);
    }
    return ret;
}

#if EXTENSION_ENABLED
static int
PySide_FinishSignatures(PyObject *module, const char *signatures)
{
    const char *name = NULL;

    // CRUCIAL: Do not call this on "testbinding":
    // The module is different and should not get signatures, anyway.
    name = PyModule_GetName(module);
    if (name == NULL)
        return -1;
    if (strncmp(name, "PySide2.Qt", 10) != 0)
        return 0;

    // we abuse the call for types, since they both have a __name__ attribute.
    if (PySide_BuildSignatureArgs(module, module, signatures) < 0)
        return -1;

    /*
     * Python2 does not abuse the 'm_self' field for the type. So we need to
     * supply this for all static methods.
     *
     * Note: This function crashed when called from PySide_BuildSignatureArgs.
     * Probably this was too early.
     *
     * Pep384: We need to switch this always on since we have no access
     * to the PyCFunction attributes. Therefore I simplified things
     * and always use our own mapping.
     */
    {
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        PyObject *dict = PyModule_GetDict(module);

        if (dict == NULL)
            return -1;

        while (PyDict_Next(dict, &pos, &key, &value)) {
            if (PyType_Check(value)) {
                PyObject *type = value;
                if (build_func_to_type(type) < 0)
                    return -1;
            }
        }
    }
    return 0;
}
#endif // EXTENSION_ENABLED

void
FinishSignatureInitialization(PyObject *module, const char *signatures)
{
#if EXTENSION_ENABLED
    if (PySide_FinishSignatures(module, signatures) < 0) {
        PyErr_Print();
        PyErr_SetNone(PyExc_ImportError);
    }
#endif
}

} //extern "C"
