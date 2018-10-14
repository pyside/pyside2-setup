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

#include "basewrapper.h"
#include "autodecref.h"

extern "C"
{

/*
 * The documentation is located in file signature_doc.rst
 */

#include "signature.h"
#include <structmember.h>

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

static PyObject *GetClassKey(PyObject *ob);

static PyObject *GetSignature_Function(PyObject *, const char *);
static PyObject *GetSignature_TypeMod(PyObject *, const char *);
static PyObject *GetSignature_Wrapper(PyObject *, const char *);
static PyObject *get_signature(PyObject *self, PyObject *args);

static PyObject *PySide_BuildSignatureProps(PyObject *class_mod);

static void init_module_1(void);
static void init_module_2(void);

const char helper_module_name[] = "signature_loader";
const char bootstrap_name[] = "bootstrap";
const char arg_name[] = "pyside_arg_dict";
const char func_name[] = "pyside_type_init";

static PyObject *
CreateSignature(PyObject *props, PyObject *key)
{
    /*
     * Here is the new function to create all signatures. It simply calls
     * into Python and creates a signature object for a dummy-function.
     * This is so much simpler than using all the attributes explicitly
     * to support '_signature_is_functionlike()'.
     */
    return PyObject_CallFunction(pyside_globals->createsig_func,
                                 (char *)"(OO)", props, key);
}

static PyObject *
pyside_cf_get___signature__(PyObject *func, const char *modifier)
{
    return GetSignature_Function(func, modifier);
}

static PyObject *
pyside_sm_get___signature__(PyObject *sm, const char *modifier)
{
    Shiboken::AutoDecRef func(PyObject_GetAttrString(sm, "__func__"));
    return GetSignature_Function(func, modifier);
}

static PyObject *
_get_class_of_cf(PyObject *ob_cf)
{
    PyObject *selftype = PyCFunction_GET_SELF(ob_cf);
    if (selftype == NULL)
        selftype = PyDict_GetItem(pyside_globals->map_dict, (PyObject *)ob_cf);
    if (selftype == NULL) {
        if (!PyErr_Occurred())
            Py_RETURN_NONE;
        return NULL;
    }
    PyObject *typemod = (PyType_Check(selftype) || PyModule_Check(selftype))
                        ? selftype : (PyObject *)Py_TYPE(selftype);
    // do we support module functions?
    Py_INCREF(typemod);
    return typemod;
}

static PyObject *
_get_class_of_sm(PyObject *ob_sm)
{
    Shiboken::AutoDecRef func(PyObject_GetAttrString(ob_sm, "__func__"));
    return _get_class_of_cf(func);
}

static PyObject *
_get_class_of_descr(PyObject *ob)
{
    Shiboken::AutoDecRef func_name(PyObject_GetAttrString(ob, "__name__"));
    return PyObject_GetAttrString(ob, "__objclass__");
}

static PyObject *
GetClassOfFunc(PyObject *ob)
{
    if (PyType_Check(ob))
        return ob;
    if (Py_TYPE(ob) == &PyCFunction_Type)
        return _get_class_of_cf(ob);
    if (Py_TYPE(ob) == PepStaticMethod_TypePtr)
        return _get_class_of_sm(ob);
    if (Py_TYPE(ob) == PepMethodDescr_TypePtr)
        return _get_class_of_descr(ob);
    if (Py_TYPE(ob) == &PyWrapperDescr_Type)
        return _get_class_of_descr(ob);
    Py_FatalError("unexpected type in GetClassOfFunc");
    return nullptr;
}

static PyObject *
compute_name_key(PyObject *ob)
{
    if (PyType_Check(ob))
        return GetClassKey(GetClassOfFunc(ob));
    PyObject *func = ob;
    if (Py_TYPE(ob) == PepStaticMethod_TypePtr)
        func = PyObject_GetAttrString(ob, "__func__");
    else
        Py_INCREF(func);
    Shiboken::AutoDecRef func_name(PyObject_GetAttrString(func, "__name__"));
    Py_DECREF(func);
    if (func_name.isNull())
        Py_FatalError("unexpected name problem in compute_name_key");
    Shiboken::AutoDecRef type_key(GetClassKey(GetClassOfFunc(ob)));
    return Py_BuildValue("(OO)", type_key.object(), func_name.object());
}

static int
build_name_key_to_func(PyObject *obtype)
{
    PyTypeObject *type = (PyTypeObject *)obtype;
    PyMethodDef *meth = type->tp_methods;

    if (meth == 0)
        return 0;

    for (; meth->ml_name != NULL; meth++) {
        Shiboken::AutoDecRef func(PyCFunction_NewEx(meth, obtype, NULL));
        Shiboken::AutoDecRef name_key(compute_name_key(func));
        if (func.isNull() || name_key.isNull()
            || PyDict_SetItem(pyside_globals->map_dict, name_key, func) < 0)
            return -1;
    }
    return 0;
}

static PyObject *
name_key_to_func(PyObject *ob)
{
    /*
     * We build a mapping from name_key to function.
     * This could also be computed directly, but the Limited API
     * makes this impossible. So we always build our own mapping.
     */
    Shiboken::AutoDecRef name_key(compute_name_key(ob));
    if (name_key.isNull())
        Py_RETURN_NONE;

    PyObject *ret = PyDict_GetItem(pyside_globals->map_dict, name_key);
    if (ret == NULL) {
        // do a lazy initialization
        Shiboken::AutoDecRef type_key(GetClassKey(GetClassOfFunc(ob)));
        PyObject *type = PyDict_GetItem(pyside_globals->map_dict,
                                        type_key);
        if (type == nullptr)
            Py_RETURN_NONE;
        assert(PyType_Check(type));
        if (build_name_key_to_func(type) < 0)
            return NULL;
        ret = PyDict_GetItem(pyside_globals->map_dict, name_key);
    }
    Py_XINCREF(ret);
    return ret;
}

static PyObject *
pyside_md_get___signature__(PyObject *ob_md, const char *modifier)
{
    Shiboken::AutoDecRef func(name_key_to_func(ob_md));
    if (func.object() == Py_None)
        return Py_None;
    if (func.isNull())
        Py_FatalError("missing mapping in MethodDescriptor");
    return pyside_cf_get___signature__(func, modifier);
}

static PyObject *
pyside_wd_get___signature__(PyObject *ob, const char *modifier)
{
    return GetSignature_Wrapper(ob, modifier);
}

static PyObject *
pyside_tp_get___signature__(PyObject *typemod, const char *modifier)
{
    return GetSignature_TypeMod(typemod, modifier);
}

// forward
static PyObject *
GetSignature_Cached(PyObject *props, const char *sig_kind, const char *modifier);

static PyObject *
GetClassKey(PyObject *ob)
{
    assert(PyType_Check(ob) || PyModule_Check(ob));
    /*
     * We obtain a unique key using the module name and the class name.
     *
     * The class name is a bit funny when modules are nested.
     * Example:
     *
     * "sample.Photon.ValueIdentity" is a class.
     *   name:   "ValueIdentity"
     *   module: "sample.Photon"
     *
     * This is the PyCFunction behavior, as opposed to Python functions.
     */
    Shiboken::AutoDecRef class_name(PyObject_GetAttrString(ob, "__name__"));
    Shiboken::AutoDecRef module_name(PyObject_GetAttrString(ob, "__module__"));

    if (module_name.isNull())
        PyErr_Clear();

    // Note: if we have a module, then __module__ is null, and we get
    // the module name through __name__ .
    if (class_name.isNull())
        return nullptr;
    if (module_name.object())
        return Py_BuildValue("(OO)", module_name.object(), class_name.object());
    return Py_BuildValue("O", class_name.object());
}

static PyObject *
GetSignature_Function(PyObject *ob_func, const char *modifier)
{
    Shiboken::AutoDecRef typemod(GetClassOfFunc(ob_func));
    Shiboken::AutoDecRef type_key(GetClassKey(typemod));
    if (type_key.isNull())
        Py_RETURN_NONE;
    PyObject *dict = PyDict_GetItem(pyside_globals->arg_dict, type_key);
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
    Shiboken::AutoDecRef func_name(PyObject_GetAttrString(ob_func, "__name__"));
    PyObject *props = !func_name.isNull() ? PyDict_GetItem(dict, func_name) : nullptr;
    if (props == NULL)
        Py_RETURN_NONE;

    int flags = PyCFunction_GET_FLAGS(ob_func);
    const char *sig_kind;
    if (PyModule_Check(typemod))
        sig_kind = "function";
    else if (flags & METH_CLASS)
        sig_kind = "classmethod";
    else if (flags & METH_STATIC)
        sig_kind = "staticmethod";
    else
        sig_kind = "method";
    PyObject *ret = GetSignature_Cached(props, sig_kind, modifier);
    return ret;
}

static PyObject *
GetSignature_Wrapper(PyObject *ob, const char *modifier)
{
    Shiboken::AutoDecRef func_name(PyObject_GetAttrString(ob, "__name__"));
    Shiboken::AutoDecRef objclass(PyObject_GetAttrString(ob, "__objclass__"));
    Shiboken::AutoDecRef class_key(GetClassKey(objclass));

    if (func_name.isNull() || objclass.isNull() || class_key.isNull())
        return nullptr;
    PyObject *dict = PyDict_GetItem(pyside_globals->arg_dict, class_key);
    if (dict == NULL)
        Py_RETURN_NONE;
    if (PyTuple_Check(dict)) {
        /*
         * We do the initialization lazily.
         * This has also the advantage that we can freely import PySide.
         */
        dict = PySide_BuildSignatureProps(objclass);
        if (dict == NULL)
            Py_RETURN_NONE;
    }
    PyObject *props = PyDict_GetItem(dict, func_name);
    if (props == NULL)
        Py_RETURN_NONE;
    return GetSignature_Cached(props, "method", modifier);
}

static PyObject *
GetSignature_TypeMod(PyObject *ob, const char *modifier)
{
    Shiboken::AutoDecRef ob_name(PyObject_GetAttrString(ob, "__name__"));
    Shiboken::AutoDecRef ob_key(GetClassKey(ob));

    PyObject *dict = PyDict_GetItem(pyside_globals->arg_dict, ob_key);
    if (dict == NULL)
        Py_RETURN_NONE;

    if (PyTuple_Check(dict)) {
        dict = PySide_BuildSignatureProps(ob);
        if (dict == NULL) {
            Py_RETURN_NONE;
        }
    }
    PyObject *props = PyDict_GetItem(dict, ob_name);
    if (props == NULL)
        Py_RETURN_NONE;
    return GetSignature_Cached(props, "method", modifier);
}

static PyObject *
GetSignature_Cached(PyObject *props, const char *sig_kind, const char *modifier)
{
    Shiboken::AutoDecRef key(modifier == nullptr
                             ? Py_BuildValue("s", sig_kind)
                             : Py_BuildValue("(ss)", sig_kind, modifier));
    PyObject *value = PyDict_GetItem(props, key);
    if (value == nullptr) {
        // we need to compute a signature object
        value = CreateSignature(props, key);
        if (value != nullptr) {
            if (PyDict_SetItem(props, key, value) < 0)
                // this is an error
                return nullptr;
        }
        else {
            // key not found
            Py_RETURN_NONE;
        }
    }
    return Py_INCREF(value), value;
}

static const char PySide_PythonCode[] =
    "from __future__ import print_function, absolute_import\n" R"~(if True:

    import sys, os, traceback
    # We avoid imports in phase 1 that could fail. "import shiboken" of the
    # binary would even crash in FinishSignatureInitialization.

    def bootstrap():
        global __file__
        import PySide2 as root
        rp = os.path.realpath(os.path.dirname(root.__file__))
        __file__ = os.path.join(rp, 'support', 'signature', 'loader.py')
        try:
            with open(__file__) as _f:
                exec(compile(_f.read(), __file__, 'exec'))
        except Exception as e:
            print('Exception:', e)
            traceback.print_exc(file=sys.stdout)
        globals().update(locals())

    )~";

static safe_globals_struc *
init_phase_1(void)
{
    PyObject *d, *v;
    safe_globals_struc *p = (safe_globals_struc *)
                            malloc(sizeof(safe_globals_struc));
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

    // build a dict for the prepared arguments
    p->arg_dict = PyDict_New();
    if (p->arg_dict == NULL
        || PyObject_SetAttrString(p->helper_module, arg_name, p->arg_dict) < 0)
        goto error;
    return p;

error:
    PyErr_SetString(PyExc_SystemError, "could not initialize part 1");
    return NULL;
}

static int
init_phase_2(safe_globals_struc *p, PyMethodDef *methods)
{
    PyObject *bootstrap_func, *v = nullptr;
    PyMethodDef *ml;

    // The single function to be called, but maybe more to come.
    for (ml = methods; ml->ml_name != NULL; ml++) {
        v = PyCFunction_NewEx(ml, nullptr, nullptr);
        if (v == nullptr
            || PyObject_SetAttrString(p->helper_module, ml->ml_name, v) != 0)
            goto error;
        Py_DECREF(v);
    }
    bootstrap_func = PyObject_GetAttrString(p->helper_module, bootstrap_name);
    if (bootstrap_func == NULL
        || PyObject_CallFunction(bootstrap_func, (char *)"()") == NULL)
        goto error;
    // now the loader should be initialized
    p->sigparse_func = PyObject_GetAttrString(p->helper_module, func_name);
    if (p->sigparse_func == NULL)
        goto error;
    p->createsig_func = PyObject_GetAttrString(p->helper_module, "create_signature");
    if (p->createsig_func == NULL)
        goto error;
    return 0;

error:
    Py_XDECREF(v);
    PyErr_SetString(PyExc_SystemError, "could not initialize part 2");
    return -1;
}

static int
add_more_getsets(PyTypeObject *type, PyGetSetDef *gsp)
{
    assert(PyType_Check(type));
    PyType_Ready(type);
    PyObject *dict = type->tp_dict;
    for (; gsp->name != NULL; gsp++) {
        if (PyDict_GetItemString(dict, gsp->name))
            continue;
        Shiboken::AutoDecRef descr(PyDescr_NewGetSet(type, gsp));
        if (descr.isNull())
            return -1;
        if (PyDict_SetItemString(dict, gsp->name, descr) < 0)
            return -1;
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

static PyGetSetDef new_PyWrapperDescr_getsets[] = {
    {(char *) "__signature__", (getter)pyside_wd_get___signature__},
    {0}
};

////////////////////////////////////////////////////////////////////////////
//
// get_signature  --  providing a superior interface
//
// Additionally to the interface via __signature__, we also provide
// a general function, which allows for different signature layouts.
// The "modifier" argument is a string that is passed in from loader.py .
// Configuration what the modifiers mean is completely in Python.
//

static PyObject *
get_signature(PyObject *self, PyObject *args)
{
    PyObject *ob;
    const char *modifier = nullptr;

    init_module_1();
    init_module_2();

    if (!PyArg_ParseTuple(args, "O|s", &ob, &modifier))
        return NULL;
    if (Py_TYPE(ob) == &PyCFunction_Type)
        return pyside_cf_get___signature__(ob, modifier);
    if (Py_TYPE(ob) == PepStaticMethod_TypePtr)
        return pyside_sm_get___signature__(ob, modifier);
    if (Py_TYPE(ob) == PepMethodDescr_TypePtr)
        return pyside_md_get___signature__(ob, modifier);
    if (PyType_Check(ob))
        return pyside_tp_get___signature__(ob, modifier);
    if (Py_TYPE(ob) == &PyWrapperDescr_Type)
        return pyside_wd_get___signature__(ob, modifier);
    Py_RETURN_NONE;
}

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
    static int init_done = 0;

    if (!init_done) {
        Shiboken::AutoDecRef md(PyObject_GetAttrString((PyObject *)&PyString_Type, "split"));       // method-descriptor
        Shiboken::AutoDecRef wd(PyObject_GetAttrString((PyObject *)Py_TYPE(Py_True), "__add__"));   // wrapper-descriptor
        if (md.isNull() || wd.isNull()
            || PyType_Ready(Py_TYPE(md)) < 0
            || add_more_getsets(PepMethodDescr_TypePtr, new_PyMethodDescr_getsets) < 0
            || add_more_getsets(&PyCFunction_Type, new_PyCFunction_getsets) < 0
            || add_more_getsets(PepStaticMethod_TypePtr, new_PyStaticMethod_getsets) < 0
            || add_more_getsets(&PyType_Type, new_PyType_getsets) < 0
            || add_more_getsets(Py_TYPE(wd), new_PyWrapperDescr_getsets) < 0
            )
            return -1;
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

static void
init_module_1(void)
{
    static int init_done = 0;

    if (!init_done) {
        pyside_globals = init_phase_1();
        if (pyside_globals != nullptr)
            init_done = 1;
    }
}

static int
PySide_BuildSignatureArgs(PyObject *module, PyObject *type,
                          const char *signatures)
{
    PyObject *type_key, *arg_tup;

    init_module_1();
    arg_tup = Py_BuildValue("(Os)", type, signatures);
    if (arg_tup == NULL)
        return -1;
    /*
     * We either get a module name or the dict of an EnclosingObject.
     * We can ignore the EnclosingObject since we get full name info
     * from the type.
     */
    if (PyModule_Check(module)) {
        const char *name = PyModule_GetName(module);
        if (name == NULL)
            return -1;
        if (strcmp(name, "testbinding") == 0)
            return 0;
    }
    else
        assert(PyDict_Check(module));
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
    type_key = GetClassKey(type);
    if (type_key == nullptr)
        return -1;
    if (PyDict_SetItem(pyside_globals->arg_dict, type_key, arg_tup) < 0)
        return -1;
    /*
     * We record also a mapping from type key to type. This helps to lazily
     * initialize the Py_LIMITED_API in name_key_to_func().
     */

    if (PyDict_SetItem(pyside_globals->map_dict, type_key, type) < 0)
        return -1;
    return 0;
}

static PyMethodDef signature_methods[] = {
    {"get_signature", (PyCFunction)get_signature, METH_VARARGS,
        "get the __signature__, but pass an optional string parameter"},
    {NULL, NULL}
};

static void
init_module_2(void)
{
    static int init_done = 0, initializing = 0;

    if (!init_done) {
        if (initializing)
            Py_FatalError("Init 2 called recursively!");
        init_phase_2(pyside_globals, signature_methods);
        init_done = 1;
        initializing = 0;
    }
}

static PyObject *
PySide_BuildSignatureProps(PyObject *classmod)
{
    /*
     * Here is the second part of the function.
     * This part will be called on-demand when needed by some attribute.
     * We simply pick up the arguments that we stored here and replace
     * them by the function result.
     */
    init_module_2();
    Shiboken::AutoDecRef type_key(GetClassKey(classmod));
    if (type_key.isNull())
        return nullptr;
    PyObject *arg_tup = PyDict_GetItem(pyside_globals->arg_dict, type_key);
    if (arg_tup == nullptr)
        return nullptr;
    PyObject *dict = PyObject_CallObject(pyside_globals->sigparse_func, arg_tup);
    if (dict == nullptr)
        return nullptr;

    // We replace the arguments by the result dict.
    if (PyDict_SetItem(pyside_globals->arg_dict, type_key, dict) < 0)
        return nullptr;
    return dict;
}

int
SbkSpecial_Type_Ready(PyObject *module, PyTypeObject *type,
                      const char *signatures)
{
    int ret;
    if (PySideType_Ready(type) < 0)
        return -1;
    ret = PySide_BuildSignatureArgs(module, (PyObject *)type, signatures);
    if (ret < 0) {
        PyErr_Print();
        PyErr_SetNone(PyExc_ImportError);
    }
    return ret;
}

static int _finish_nested_classes(PyObject *dict);
static int _build_func_to_type(PyObject *obtype);

static int
PySide_FinishSignatures(PyObject *module, const char *signatures)
{
    /*
     * Initialization of module functions and resolving of static methods.
     */

    // CRUCIAL: Do not call this on "testbinding":
    // The module is different and should not get signatures, anyway.
    const char *name = PyModule_GetName(module);
    if (name == NULL)
        return -1;
    if (strcmp(name, "testbinding") == 0)
        return 0;

    // we abuse the call for types, since they both have a __name__ attribute.
    if (PySide_BuildSignatureArgs(module, module, signatures) < 0)
        return -1;

    /*
     * Note: This function crashed when called from PySide_BuildSignatureArgs.
     * Probably this was too early.
     *
     * Pep384: We need to switch this always on since we have no access
     * to the PyCFunction attributes. Therefore I simplified things
     * and always use our own mapping.
     */
    PyObject *key, *func, *obdict = PyModule_GetDict(module);
    Py_ssize_t pos = 0;

    while (PyDict_Next(obdict, &pos, &key, &func))
        if (PyCFunction_Check(func))
            if (PyDict_SetItem(pyside_globals->map_dict, func, module) < 0)
                return -1;
    if (_finish_nested_classes(obdict) < 0)
        return -1;
    return 0;
}

static int
_finish_nested_classes(PyObject *obdict)
{
    PyObject *key, *value, *obtype;
    PyTypeObject *subtype;
    Py_ssize_t pos = 0;

    if (obdict == NULL)
        return -1;
    while (PyDict_Next(obdict, &pos, &key, &value)) {
        if (PyType_Check(value)) {
            obtype = value;
            if (_build_func_to_type(obtype) < 0)
                return -1;
            // now continue with nested cases
            subtype = reinterpret_cast<PyTypeObject *>(obtype);
            if (_finish_nested_classes(subtype->tp_dict) < 0)
                return -1;
        }
    }
    return 0;
}

static int
_build_func_to_type(PyObject *obtype)
{
    /*
     * There is no general way to directly get the type of a static method.
     * On Python 3, the type is hidden in an unused pointer in the
     * PyCFunction structure, but the Limited API does not allow to access
     * this, either.
     *
     * In the end, it was easier to avoid such tricks and build an explicit
     * mapping from function to type.
     *
     * We walk through the method list of the type
     * and record the mapping from function to this type in a dict.
     */
    PyTypeObject *type = reinterpret_cast<PyTypeObject *>(obtype);
    PyObject *dict = type->tp_dict;
    PyMethodDef *meth = type->tp_methods;

    if (meth == 0)
        return 0;

    for (; meth->ml_name != NULL; meth++) {
        if (meth->ml_flags & METH_STATIC) {
            PyObject *descr = PyDict_GetItemString(dict, meth->ml_name);
            if (descr == NULL)
                return -1;
            Shiboken::AutoDecRef func(PyObject_GetAttrString(descr, "__func__"));
            if (func.isNull() ||
                PyDict_SetItem(pyside_globals->map_dict, func, obtype) < 0)
                return -1;
        }
    }
    return 0;
}

void
FinishSignatureInitialization(PyObject *module, const char *signatures)
{
    /*
     * This function is called at the very end of a module
     * initialization. SbkSpecial_Type_Ready has already been run
     * with all the types.
     * We now initialize module functions and resolve static methods.
     */
    if (PySide_FinishSignatures(module, signatures) < 0) {
        PyErr_Print();
        PyErr_SetNone(PyExc_ImportError);
    }
}

} //extern "C"
