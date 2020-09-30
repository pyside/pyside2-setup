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
// signature_helper.cpp
// --------------------
//
// This file contains assoerted helper functions that are needed,
// but it is not helpful to see them all the time.
//

#include "autodecref.h"
#include "sbkstring.h"
#include "sbkstaticstrings.h"
#include "sbkstaticstrings_p.h"

#include "signature_p.h"

using namespace Shiboken;

extern "C" {

// Helper for __qualname__ which might not always exist in Python 2 (type).
PyObject *_get_qualname(PyObject *ob)
{
    // We support __qualname__ for types, only.
    assert(PyType_Check(ob));
    PyObject *name = PyObject_GetAttr(ob, PyMagicName::qualname());
    if (name == nullptr) {
        PyErr_Clear();
        name = PyObject_GetAttr(ob, PyMagicName::name());
    }
    return name;
}

static int _fixup_getset(PyTypeObject *type, const char *name, PyGetSetDef *new_gsp)
{
    /*
     * This function pre-fills all fields of the new gsp. We then
     * insert the changed values.
     */
    PyGetSetDef *gsp = type->tp_getset;
    if (gsp != nullptr) {
        for (; gsp->name != nullptr; gsp++) {
            if (strcmp(gsp->name, name) == 0) {
                new_gsp->set = gsp->set;
                new_gsp->doc = gsp->doc;
                new_gsp->closure = gsp->closure;
                return 1;  // success
            }
        }
    }
    PyMemberDef *md = type->tp_members;
    if (md != nullptr)
        for (; md->name != nullptr; md++)
            if (strcmp(md->name, name) == 0)
                return 1;
    // staticmethod has just a `__doc__` in the class
    assert(strcmp(type->tp_name, "staticmethod") == 0 && strcmp(name, "__doc__") == 0);
    return 0;
}

int add_more_getsets(PyTypeObject *type, PyGetSetDef *gsp, PyObject **doc_descr)
{
    /*
     * This function is used to assign a new `__signature__` attribute,
     * and also to override a `__doc__` or `__name__` attribute.
     */
    assert(PyType_Check(type));
    PyType_Ready(type);
    PyObject *dict = type->tp_dict;
    for (; gsp->name != nullptr; gsp++) {
        PyObject *have_descr = PyDict_GetItemString(dict, gsp->name);
        if (have_descr != nullptr) {
            Py_INCREF(have_descr);
            if (strcmp(gsp->name, "__doc__") == 0)
                *doc_descr = have_descr;
            else
                assert(false);
            if (!_fixup_getset(type, gsp->name, gsp))
                continue;
        }
        AutoDecRef descr(PyDescr_NewGetSet(type, gsp));
        if (descr.isNull())
            return -1;
        if (PyDict_SetItemString(dict, gsp->name, descr) < 0)
            return -1;
    }
    PyType_Modified(type);
    return 0;
}

static PyObject *get_funcname(PyObject *ob)
{
    PyObject *func = ob;
    if (Py_TYPE(ob) == PepStaticMethod_TypePtr)
        func = PyObject_GetAttr(ob, PyMagicName::func());
    else
        Py_INCREF(func);
    PyObject *func_name = PyObject_GetAttr(func, PyMagicName::name());
    Py_DECREF(func);
    if (func_name == nullptr)
        Py_FatalError("unexpected name problem in compute_name_key");
    return func_name;
}

static PyObject *compute_name_key(PyObject *ob)
{
    if (PyType_Check(ob))
        return GetTypeKey(ob);
    AutoDecRef func_name(get_funcname(ob));
    AutoDecRef type_key(GetTypeKey(GetClassOrModOf(ob)));
    return Py_BuildValue("(OO)", type_key.object(), func_name.object());
}

static PyObject *_func_with_new_name(PyTypeObject *type,
                                     PyMethodDef *meth,
                                     const char *new_name)
{
    /*
     * Create a function with a lower case name.
     * Note: This is similar to feature_select's methodWithNewName,
     * but does not create a descriptor.
     * XXX Maybe we can get rid of this, completely?
     */
    auto obtype = reinterpret_cast<PyObject *>(type);
    int len = strlen(new_name);
    auto name = new char[len + 1];
    strcpy(name, new_name);
    auto new_meth = new PyMethodDef;
    new_meth->ml_name = name;
    new_meth->ml_meth = meth->ml_meth;
    new_meth->ml_flags = meth->ml_flags;
    new_meth->ml_doc = meth->ml_doc;
    return PyCFunction_NewEx(new_meth, obtype, nullptr);
}

static int build_name_key_to_func(PyObject *obtype)
{
    auto *type = reinterpret_cast<PyTypeObject *>(obtype);
    PyMethodDef *meth = type->tp_methods;

    if (meth == nullptr)
        return 0;

    AutoDecRef type_key(GetTypeKey(obtype));
    for (; meth->ml_name != nullptr; meth++) {
        AutoDecRef func(PyCFunction_NewEx(meth, obtype, nullptr));
        AutoDecRef func_name(get_funcname(func));
        AutoDecRef name_key(Py_BuildValue("(OO)", type_key.object(), func_name.object()));
        if (func.isNull() || name_key.isNull()
            || PyDict_SetItem(pyside_globals->map_dict, name_key, func) < 0)
            return -1;
    }
    // PYSIDE-1019: Now we repeat the same for snake case names.
    meth = type->tp_methods;
    for (; meth->ml_name != nullptr; meth++) {
        const char *name = String::toCString(String::getSnakeCaseName(meth->ml_name, true));
        AutoDecRef func(_func_with_new_name(type, meth, name));
        AutoDecRef func_name(get_funcname(func));
        AutoDecRef name_key(Py_BuildValue("(OO)", type_key.object(), func_name.object()));
        if (func.isNull() || name_key.isNull()
            || PyDict_SetItem(pyside_globals->map_dict, name_key, func) < 0)
            return -1;
    }
    return 0;
}

PyObject *name_key_to_func(PyObject *ob)
{
    /*
     * We build a mapping from name_key to function.
     * This could also be computed directly, but the Limited API
     * makes this impossible. So we always build our own mapping.
     */
    AutoDecRef name_key(compute_name_key(ob));
    if (name_key.isNull())
        Py_RETURN_NONE;

    PyObject *ret = PyDict_GetItem(pyside_globals->map_dict, name_key);
    if (ret == nullptr) {
        // do a lazy initialization
        AutoDecRef type_key(GetTypeKey(GetClassOrModOf(ob)));
        PyObject *type = PyDict_GetItem(pyside_globals->map_dict,
                                        type_key);
        if (type == nullptr)
            Py_RETURN_NONE;
        assert(PyType_Check(type));
        if (build_name_key_to_func(type) < 0)
            return nullptr;
        ret = PyDict_GetItem(pyside_globals->map_dict, name_key);
    }
    Py_XINCREF(ret);
    return ret;
}

static PyObject *_build_new_entry(PyObject *new_name, PyObject *value)
{
    PyObject *new_value = PyDict_Copy(value);
    PyObject *multi = PyDict_GetItem(value, PyName::multi());
    if (multi != nullptr && Py_TYPE(multi) == &PyList_Type) {
        ssize_t len = PyList_Size(multi);
        AutoDecRef list(PyList_New(len));
        if (list.isNull())
            return nullptr;
        for (int idx = 0; idx < len; ++idx) {
            auto multi_entry = PyList_GetItem(multi, idx);
            auto dup = PyDict_Copy(multi_entry);
            if (PyDict_SetItem(dup, PyName::name(), new_name) < 0)
                return nullptr;
            if (PyList_SetItem(list, idx, dup) < 0)
                return nullptr;
        }
        if (PyDict_SetItem(new_value, PyName::multi(), list) < 0)
            return nullptr;
    } else {
        if (PyDict_SetItem(new_value, PyName::name(), new_name) < 0)
            return nullptr;
    }
    return new_value;
}

int insert_snake_case_variants(PyObject *dict)
{
    AutoDecRef snake_dict(PyDict_New());
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) {
        AutoDecRef name(String::getSnakeCaseName(key, true));
        AutoDecRef new_value(_build_new_entry(name, value));
        if (PyDict_SetItem(snake_dict, name, new_value) < 0)
            return -1;
    }
    return PyDict_Merge(dict, snake_dict, 0);
}

PyObject *_get_class_of_cf(PyObject *ob_cf)
{
    PyObject *selftype = PyCFunction_GET_SELF(ob_cf);
    if (selftype == nullptr) {
        selftype = PyDict_GetItem(pyside_globals->map_dict, ob_cf);
        if (selftype == nullptr) {
            // This must be an overloaded function that we handled special.
            AutoDecRef special(Py_BuildValue("(OO)", ob_cf, PyName::overload()));
            selftype = PyDict_GetItem(pyside_globals->map_dict, special);
            if (selftype == nullptr) {
                // This is probably a module function. We will return type(None).
                selftype = Py_None;
            }
        }
    }

    PyObject *obtype_mod = (PyType_Check(selftype) || PyModule_Check(selftype))
                           ? selftype
                           : reinterpret_cast<PyObject *>(Py_TYPE(selftype));
    Py_INCREF(obtype_mod);
    return obtype_mod;
}

PyObject *_get_class_of_sm(PyObject *ob_sm)
{
    AutoDecRef func(PyObject_GetAttr(ob_sm, PyMagicName::func()));
    return _get_class_of_cf(func);
}

PyObject *_get_class_of_descr(PyObject *ob)
{
    return PyObject_GetAttr(ob, PyMagicName::objclass());
}

PyObject *_address_to_stringlist(PyObject *numkey)
{
    /*
     * This is a tiny optimization that saves initialization time.
     * Instead of creating all Python strings during the call to
     * `PySide_BuildSignatureArgs`, we store the address of the stringlist.
     * When needed in `PySide_BuildSignatureProps`, the strings are
     * finally materialized.
     */
    ssize_t address = PyNumber_AsSsize_t(numkey, PyExc_ValueError);
    if (address == -1 && PyErr_Occurred())
        return nullptr;
    char **sig_strings = reinterpret_cast<char **>(address);
    PyObject *res_list = PyList_New(0);
    if (res_list == nullptr)
        return nullptr;
    for (; *sig_strings != nullptr; ++sig_strings) {
        char *sig_str = *sig_strings;
        AutoDecRef pystr(Py_BuildValue("s", sig_str));
        if (pystr.isNull() || PyList_Append(res_list, pystr) < 0)
            return nullptr;
    }
    return res_list;
}

static int _build_func_to_type(PyObject *obtype)
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
     * and record the mapping from static method to this type in a dict.
     * We also check for hidden methods, see below.
     */
    auto *type = reinterpret_cast<PyTypeObject *>(obtype);
    PyObject *dict = type->tp_dict;
    PyMethodDef *meth = type->tp_methods;

    if (meth == nullptr)
        return 0;

    for (; meth->ml_name != nullptr; meth++) {
        /*
         * It is possible that a method is overwritten by another
         * attribute with the same name. This case was obviously provoked
         * explicitly in "testbinding.TestObject.staticMethodDouble",
         * where instead of the method a "PySide2.QtCore.Signal" object
         * was in the dict.
         * This overlap is also found in regular PySide under
         * "PySide2.QtCore.QProcess.error" where again a signal object is
         * returned. These hidden methods will be opened for the
         * signature module by adding them under the name
         * "{name}.overload".
         */
        PyObject *descr = PyDict_GetItemString(dict, meth->ml_name);
        PyObject *look_attr = meth->ml_flags & METH_STATIC ? PyMagicName::func()
                                                           : PyMagicName::name();
        int check_name = meth->ml_flags & METH_STATIC ? 0 : 1;
        if (descr == nullptr)
            return -1;

        // We first check all methods if one is hidden by something else.
        AutoDecRef look(PyObject_GetAttr(descr, look_attr));
        AutoDecRef given(Py_BuildValue("s", meth->ml_name));
        if (look.isNull()
            || (check_name && PyObject_RichCompareBool(look, given, Py_EQ) != 1)) {
            PyErr_Clear();
            AutoDecRef cfunc(PyCFunction_NewEx(
                                meth, reinterpret_cast<PyObject *>(type), nullptr));
            if (cfunc.isNull())
                return -1;
            if (meth->ml_flags & METH_STATIC)
                descr = PyStaticMethod_New(cfunc);
            else
                descr = PyDescr_NewMethod(type, meth);
            if (descr == nullptr)
                return -1;
            char mangled_name[200];
            strcpy(mangled_name, meth->ml_name);
            strcat(mangled_name, ".overload");
            if (PyDict_SetItemString(dict, mangled_name, descr) < 0)
                return -1;
            if (meth->ml_flags & METH_STATIC) {
                // This is the special case where a static method is hidden.
                AutoDecRef special(Py_BuildValue("(Os)", cfunc.object(), "overload"));
                if (PyDict_SetItem(pyside_globals->map_dict, special, obtype) < 0)
                    return -1;
            }
            if (PyDict_SetItemString(pyside_globals->map_dict, mangled_name, obtype) < 0)
                return -1;
            continue;
        }
        // Then we insert the mapping for static methods.
        if (meth->ml_flags & METH_STATIC) {
            if (PyDict_SetItem(pyside_globals->map_dict, look, obtype) < 0)
                return -1;
        }
    }
    return 0;
}

int _finish_nested_classes(PyObject *obdict)
{
    PyObject *key, *value, *obtype;
    PyTypeObject *subtype;
    Py_ssize_t pos = 0;

    if (obdict == nullptr)
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

} // extern "C"
