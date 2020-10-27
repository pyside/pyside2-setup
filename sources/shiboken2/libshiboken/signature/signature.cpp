/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

////////////////////////////////////////////////////////////////////////////
//
// signature.cpp
// -------------
//
// This is the main file of the signature module.
// It contains the most important functions and avoids confusion
// by moving many helper functions elsewhere.
//
// General documentation can be found in `signature_doc.rst`.
//

#include "basewrapper.h"
#include "autodecref.h"
#include "sbkstring.h"
#include "sbkstaticstrings.h"
#include "sbkstaticstrings_p.h"

#include "signature_p.h"
#include <structmember.h>

using namespace Shiboken;

extern "C"
{

static PyObject *CreateSignature(PyObject *props, PyObject *key)
{
    /*
     * Here is the new function to create all signatures. It simply calls
     * into Python and creates a signature object directly.
     * This is so much simpler than using all the attributes explicitly
     * to support '_signature_is_functionlike()'.
     */
    return PyObject_CallFunction(pyside_globals->create_signature_func,
                                 const_cast<char *>("(OO)"), props, key);
}

PyObject *GetClassOrModOf(PyObject *ob)
{
    /*
     * Return the type or module of a function or type.
     * The purpose is finally to use the name of the object.
     */
    if (PyType_Check(ob)) {
        // PySide-928: The type case must do refcounting like the others as well.
        Py_INCREF(ob);
        return ob;
    }
    if (PyType_IsSubtype(Py_TYPE(ob), &PyCFunction_Type))
        return _get_class_of_cf(ob);
    if (Py_TYPE(ob) == PepStaticMethod_TypePtr)
        return _get_class_of_sm(ob);
    if (Py_TYPE(ob) == PepMethodDescr_TypePtr)
        return _get_class_of_descr(ob);
    if (Py_TYPE(ob) == &PyWrapperDescr_Type)
        return _get_class_of_descr(ob);
    Py_FatalError("unexpected type in GetClassOrModOf");
    return nullptr;
}

PyObject *GetTypeKey(PyObject *ob)
{
    assert(PyType_Check(ob) || PyModule_Check(ob));
    /*
     * Obtain a unique key using the module name and the type name.
     *
     * PYSIDE-1286: We use correct __module__ and __qualname__, now.
     */
    AutoDecRef module_name(PyObject_GetAttr(ob, PyMagicName::module()));
    if (module_name.isNull()) {
        // We have no module_name because this is a module ;-)
        PyErr_Clear();
        module_name.reset(PyObject_GetAttr(ob, PyMagicName::name()));
        return Py_BuildValue("O", module_name.object());
    }
    AutoDecRef class_name(_get_qualname(ob));
    if (class_name.isNull()) {
        Py_FatalError("Signature: missing class name in GetTypeKey");
        return nullptr;
    }
    return Py_BuildValue("(OO)", module_name.object(), class_name.object());
}

static PyObject *empty_dict = nullptr;

PyObject *TypeKey_to_PropsDict(PyObject *type_key, PyObject *obtype)
{
    PyObject *dict = PyDict_GetItem(pyside_globals->arg_dict, type_key);
    if (dict == nullptr) {
        if (empty_dict == nullptr)
            empty_dict = PyDict_New();
        dict = empty_dict;
    }
    if (!PyDict_Check(dict))
        dict = PySide_BuildSignatureProps(type_key);
    return dict;
}

static PyObject *_GetSignature_Cached(PyObject *props, PyObject *func_kind, PyObject *modifier)
{
    // Special case: We want to know the func_kind.
    if (modifier) {
        PyUnicode_InternInPlace(&modifier);
        if (modifier == PyMagicName::func_kind())
            return Py_BuildValue("O", func_kind);
    }

    AutoDecRef key(modifier == nullptr ? Py_BuildValue("O", func_kind)
                                       : Py_BuildValue("(OO)", func_kind, modifier));
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

PyObject *GetSignature_Function(PyObject *obfunc, PyObject *modifier)
{
    // make sure that we look into PyCFunction, only...
    if (Py_TYPE(obfunc) == PepFunction_TypePtr)
        Py_RETURN_NONE;
    AutoDecRef obtype_mod(GetClassOrModOf(obfunc));
    AutoDecRef type_key(GetTypeKey(obtype_mod));
    if (type_key.isNull())
        Py_RETURN_NONE;
    PyObject *dict = TypeKey_to_PropsDict(type_key, obtype_mod);
    if (dict == nullptr)
        return nullptr;
    AutoDecRef func_name(PyObject_GetAttr(obfunc, PyMagicName::name()));
    PyObject *props = !func_name.isNull() ? PyDict_GetItem(dict, func_name) : nullptr;
    if (props == nullptr)
        Py_RETURN_NONE;

    int flags = PyCFunction_GET_FLAGS(obfunc);
    PyObject *func_kind;
    if (PyModule_Check(obtype_mod))
        func_kind = PyName::function();
    else if (flags & METH_CLASS)
        func_kind = PyName::classmethod();
    else if (flags & METH_STATIC)
        func_kind = PyName::staticmethod();
    else
        func_kind = PyName::method();
    return _GetSignature_Cached(props, func_kind, modifier);
}

PyObject *GetSignature_Wrapper(PyObject *ob, PyObject *modifier)
{
    AutoDecRef func_name(PyObject_GetAttr(ob, PyMagicName::name()));
    AutoDecRef objclass(PyObject_GetAttr(ob, PyMagicName::objclass()));
    AutoDecRef class_key(GetTypeKey(objclass));
    if (func_name.isNull() || objclass.isNull() || class_key.isNull())
        return nullptr;
    PyObject *dict = TypeKey_to_PropsDict(class_key, objclass);
    if (dict == nullptr)
        return nullptr;
    PyObject *props = PyDict_GetItem(dict, func_name);
    if (props == nullptr)
        Py_RETURN_NONE;
    return _GetSignature_Cached(props, PyName::method(), modifier);
}

PyObject *GetSignature_TypeMod(PyObject *ob, PyObject *modifier)
{
    AutoDecRef ob_name(PyObject_GetAttr(ob, PyMagicName::name()));
    AutoDecRef ob_key(GetTypeKey(ob));

    PyObject *dict = TypeKey_to_PropsDict(ob_key, ob);
    if (dict == nullptr)
        return nullptr;
    PyObject *props = PyDict_GetItem(dict, ob_name);
    if (props == nullptr)
        Py_RETURN_NONE;
    return _GetSignature_Cached(props, PyName::method(), modifier);
}

////////////////////////////////////////////////////////////////////////////
//
// get_signature  --  providing a superior interface
//
// Additional to the interface via `__signature__`, we also provide
// a general function, which allows for different signature layouts.
// The `modifier` argument is a string that is passed in from `loader.py`.
// Configuration what the modifiers mean is completely in Python.
//

PyObject *get_signature_intern(PyObject *ob, PyObject *modifier)
{
    if (PyType_IsSubtype(Py_TYPE(ob), &PyCFunction_Type))
        return pyside_cf_get___signature__(ob, modifier);
    if (Py_TYPE(ob) == PepStaticMethod_TypePtr)
        return pyside_sm_get___signature__(ob, modifier);
    if (Py_TYPE(ob) == PepMethodDescr_TypePtr)
        return pyside_md_get___signature__(ob, modifier);
    if (PyType_Check(ob))
        return pyside_tp_get___signature__(ob, modifier);
    if (Py_TYPE(ob) == &PyWrapperDescr_Type)
        return pyside_wd_get___signature__(ob, modifier);
    return nullptr;
}

static PyObject *get_signature(PyObject * /* self */, PyObject *args)
{
    PyObject *ob;
    PyObject *modifier = nullptr;

    init_module_1();

    if (!PyArg_ParseTuple(args, "O|O", &ob, &modifier))
        return nullptr;
    if (Py_TYPE(ob) == PepFunction_TypePtr)
        Py_RETURN_NONE;
    PyObject *ret = get_signature_intern(ob, modifier);
    if (ret != nullptr)
        return ret;
    Py_RETURN_NONE;
}

PyMethodDef signature_methods[] = {
    {"get_signature", (PyCFunction)get_signature, METH_VARARGS,
        "get the __signature__, but pass an optional string parameter"},
    {nullptr, nullptr}
};

////////////////////////////////////////////////////////////////////////////
//
// Argument Handling
// -----------------
//
// * PySide_BuildSignatureArgs
//
// Called during class or module initialization.
// The signature strings from the C modules are stored in a dict for
// later use.
//
// * PySide_BuildSignatureProps
//
// Called on demand during signature retieval. This function calls all the way
// through `parser.py` and prepares all properties for the functions of the class.
// The parsed properties can then be used to create signature objects.
//

static int PySide_BuildSignatureArgs(PyObject *obtype_mod, const char *signatures[])
{
    init_module_1();
    AutoDecRef type_key(GetTypeKey(obtype_mod));
    /*
     * PYSIDE-996: Avoid string overflow in MSVC, which has a limit of
     * 2**15 unicode characters (64 K memory).
     * Instead of one huge string, we take a ssize_t that is the
     * address of a string array. It will not be turned into a real
     * string list until really used by Python. This is quite optimal.
     */
    AutoDecRef numkey(Py_BuildValue("n", signatures));
    if (type_key.isNull() || numkey.isNull()
        || PyDict_SetItem(pyside_globals->arg_dict, type_key, numkey) < 0)
        return -1;
    /*
     * We record also a mapping from type key to type/module. This helps to
     * lazily initialize the Py_LIMITED_API in name_key_to_func().
     */
    return PyDict_SetItem(pyside_globals->map_dict, type_key, obtype_mod) == 0 ? 0 : -1;
}

PyObject *PySide_BuildSignatureProps(PyObject *type_key)
{
    /*
     * Here is the second part of the function.
     * This part will be called on-demand when needed by some attribute.
     * We simply pick up the arguments that we stored here and replace
     * them by the function result.
     */
    init_module_2();
    if (type_key == nullptr)
        return nullptr;
    PyObject *numkey = PyDict_GetItem(pyside_globals->arg_dict, type_key);
    AutoDecRef strings(_address_to_stringlist(numkey));
    if (strings.isNull())
        return nullptr;
    AutoDecRef arg_tup(Py_BuildValue("(OO)", type_key, strings.object()));
    if (arg_tup.isNull())
        return nullptr;
    PyObject *dict = PyObject_CallObject(pyside_globals->pyside_type_init_func, arg_tup);
    if (dict == nullptr) {
        if (PyErr_Occurred())
            return nullptr;
        // No error: return an empty dict.
        if (empty_dict == nullptr)
            empty_dict = PyDict_New();
        return empty_dict;
    }
    // PYSIDE-1019: Build snake case versions of the functions.
    if (insert_snake_case_variants(dict) < 0)
        return nullptr;
    // We replace the arguments by the result dict.
    if (PyDict_SetItem(pyside_globals->arg_dict, type_key, dict) < 0)
        return nullptr;
    return dict;
}
//
////////////////////////////////////////////////////////////////////////////

static int PySide_FinishSignatures(PyObject *module, const char *signatures[])
{
    /*
     * Initialization of module functions and resolving of static methods.
     */
    const char *name = PyModule_GetName(module);
    if (name == nullptr)
        return -1;

    // we abuse the call for types, since they both have a __name__ attribute.
    if (PySide_BuildSignatureArgs(module, signatures) < 0)
        return -1;

    /*
     * Note: This function crashed when called from PySide_BuildSignatureArgs.
     * Probably this was an import timing problem.
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
    // The finish_import function will not work the first time since phase 2
    // was not yet run. But that is ok, because the first import is always for
    // the shiboken module (or a test module).
    if (pyside_globals->finish_import_func == nullptr) {
        assert(strncmp(name, "PySide2.", 8) != 0);
        return 0;
    }
    AutoDecRef ret(PyObject_CallFunction(
        pyside_globals->finish_import_func, const_cast<char *>("(O)"), module));
    return ret.isNull() ? -1 : 0;
}

////////////////////////////////////////////////////////////////////////////
//
// External functions interface
//
// These are exactly the supported functions from `signature.h`.
//

int InitSignatureStrings(PyTypeObject *type, const char *signatures[])
{
    auto *ob_type = reinterpret_cast<PyObject *>(type);
    int ret = PySide_BuildSignatureArgs(ob_type, signatures);
    if (ret < 0) {
        PyErr_Print();
        PyErr_SetNone(PyExc_ImportError);
    }
    return ret;
}

void FinishSignatureInitialization(PyObject *module, const char *signatures[])
{
    /*
     * This function is called at the very end of a module initialization.
     * We now patch certain types to support the __signature__ attribute,
     * initialize module functions and resolve static methods.
     *
     * Still, it is not possible to call init phase 2 from here,
     * because the import is still running. Do it from Python!
     */
    if (   PySide_PatchTypes() < 0
        || PySide_FinishSignatures(module, signatures) < 0) {
        PyErr_Print();
        PyErr_SetNone(PyExc_ImportError);
    }
}

void SetError_Argument(PyObject *args, const char *func_name)
{
    /*
     * This function replaces the type error construction with extra
     * overloads parameter in favor of using the signature module.
     * Error messages are rare, so we do it completely in Python.
     */
    init_module_1();
    init_module_2();
    AutoDecRef res(PyObject_CallFunction(pyside_globals->seterror_argument_func,
                                         const_cast<char *>("(Os)"), args, func_name));
    if (res.isNull()) {
        PyErr_Print();
        Py_FatalError("seterror_argument did not receive a result");
    }
    PyObject *err, *msg;
    if (!PyArg_UnpackTuple(res, func_name, 2, 2, &err, &msg)) {
        PyErr_Print();
        Py_FatalError("unexpected failure in seterror_argument");
    }
    PyErr_SetObject(err, msg);
}

/*
 * Support for the metatype SbkObjectType_Type's tp_getset.
 *
 * This was not necessary for __signature__, because PyType_Type inherited it.
 * But the __doc__ attribute existed already by inheritance, and calling
 * PyType_Modified() is not supported. So we added the getsets explicitly
 * to the metatype.
 */

PyObject *Sbk_TypeGet___signature__(PyObject *ob, PyObject *modifier)
{
    return pyside_tp_get___signature__(ob, modifier);
}

PyObject *Sbk_TypeGet___doc__(PyObject *ob)
{
    return pyside_tp_get___doc__(ob);
}

PyObject *GetFeatureDict()
{
    init_module_1();
    return pyside_globals->feature_dict;
}

} //extern "C"
