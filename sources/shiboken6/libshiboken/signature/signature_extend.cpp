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
// signature_extend.cpp
// --------------------
//
// This file contains the additions and changes to the following
// Python types:
//
//     PyMethodDescr_Type
//     PyCFunction_Type
//     PyStaticMethod_Type
//     PyType_Type
//     PyWrapperDescr_Type
//
// Their `tp_getset` fields are modified so support the `__signature__`
// attribute and additions to the `__doc__` attribute.
//

#include "autodecref.h"
#include "sbkstring.h"
#include "sbkstaticstrings.h"
#include "sbkstaticstrings_p.h"

#include "signature_p.h"

using namespace Shiboken;

extern "C" {

typedef PyObject *(*signaturefunc)(PyObject *, PyObject *);

static PyObject *_get_written_signature(signaturefunc sf, PyObject *ob, PyObject *modifier)
{
    /*
     * Be a writable Attribute, but have a computed value.
     *
     * If a signature has not been written, call the signature function.
     * If it has been written, return the written value.
     * After __del__ was called, the function value re-appears.
     *
     * Note: This serves also for the new version that does not allow any
     * assignment if we have a computed value. We only need to check if
     * a computed value exists and then forbid writing.
     * See pyside_set___signature
     */
    PyObject *ret = PyDict_GetItem(pyside_globals->value_dict, ob);
    if (ret == nullptr)
        return ob == nullptr ? nullptr : sf(ob, modifier);
    Py_INCREF(ret);
    return ret;
}

PyObject *pyside_cf_get___signature__(PyObject *func, PyObject *modifier)
{
    init_module_2();
    return _get_written_signature(GetSignature_Function, func, modifier);
}

PyObject *pyside_sm_get___signature__(PyObject *sm, PyObject *modifier)
{
    init_module_2();
    AutoDecRef func(PyObject_GetAttr(sm, PyMagicName::func()));
    if (Py_TYPE(func) == PepFunction_TypePtr)
        return PyObject_GetAttr(func, PyMagicName::signature());
    return _get_written_signature(GetSignature_Function, func, modifier);
}

PyObject *pyside_md_get___signature__(PyObject *ob_md, PyObject *modifier)
{
    init_module_2();
    AutoDecRef func(name_key_to_func(ob_md));
    if (func.object() == Py_None)
        return Py_None;
    if (func.isNull())
        Py_FatalError("missing mapping in MethodDescriptor");
    return pyside_cf_get___signature__(func, modifier);
}

PyObject *pyside_wd_get___signature__(PyObject *ob, PyObject *modifier)
{
    init_module_2();
    return _get_written_signature(GetSignature_Wrapper, ob, modifier);
}

PyObject *pyside_tp_get___signature__(PyObject *obtype_mod, PyObject *modifier)
{
    init_module_2();
    return _get_written_signature(GetSignature_TypeMod, obtype_mod, modifier);
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
// Addendum 2019-01-12: We now also compute a docstring from the signature.
//

// keep the original __doc__ functions
static PyObject *old_cf_doc_descr = nullptr;
static PyObject *old_sm_doc_descr = nullptr;
static PyObject *old_md_doc_descr = nullptr;
static PyObject *old_tp_doc_descr = nullptr;
static PyObject *old_wd_doc_descr = nullptr;

static int handle_doc_in_progress = 0;

static PyObject *handle_doc(PyObject *ob, PyObject *old_descr)
{
    init_module_1();
    init_module_2();
    AutoDecRef ob_type_mod(GetClassOrModOf(ob));
    const char *name;
    if (PyModule_Check(ob_type_mod))
        name = PyModule_GetName(ob_type_mod);
    else
        name = reinterpret_cast<PyTypeObject *>(ob_type_mod.object())->tp_name;
    if (handle_doc_in_progress || name == nullptr
        || strncmp(name, "PySide2.", 8) != 0)
        return PyObject_CallMethodObjArgs(old_descr,
                                          PyMagicName::get(),
                                          ob, nullptr);
    handle_doc_in_progress++;
    PyObject *res = PyObject_CallFunction(
                        pyside_globals->make_helptext_func,
                        const_cast<char *>("(O)"), ob);
    handle_doc_in_progress--;
    if (res == nullptr) {
        PyErr_Print();
        Py_FatalError("handle_doc did not receive a result");
    }
    return res;
}

static PyObject *pyside_cf_get___doc__(PyObject *cf)
{
    return handle_doc(cf, old_cf_doc_descr);
}

static PyObject *pyside_sm_get___doc__(PyObject *sm)
{
    return handle_doc(sm, old_sm_doc_descr);
}

static PyObject *pyside_md_get___doc__(PyObject *md)
{
    return handle_doc(md, old_md_doc_descr);
}

PyObject *pyside_tp_get___doc__(PyObject *tp)
{
    return handle_doc(tp, old_tp_doc_descr);
}

static PyObject *pyside_wd_get___doc__(PyObject *wd)
{
    return handle_doc(wd, old_wd_doc_descr);
}

// the default setter for all objects
static int pyside_set___signature__(PyObject *op, PyObject *value)
{
    // By this additional check, this function refuses write access.
    // We consider both nullptr and Py_None as not been written.
    AutoDecRef has_val(get_signature_intern(op, nullptr));
    if (!(has_val.isNull() || has_val == Py_None)) {
        PyErr_Format(PyExc_AttributeError,
                     "Attribute '__signature__' of '%.50s' object is not writable",
                     Py_TYPE(op)->tp_name);
        return -1;
    }
    int ret = value == nullptr ? PyDict_DelItem(pyside_globals->value_dict, op)
                               : PyDict_SetItem(pyside_globals->value_dict, op, value);
    Py_XINCREF(value);
    return ret;
}

static PyGetSetDef new_PyCFunction_getsets[] = {
    {const_cast<char *>("__doc__"), (getter)pyside_cf_get___doc__},
    {const_cast<char *>("__signature__"), (getter)pyside_cf_get___signature__,
                                          (setter)pyside_set___signature__},
    {nullptr}
};

static PyGetSetDef new_PyStaticMethod_getsets[] = {
    {const_cast<char *>("__doc__"), (getter)pyside_sm_get___doc__},
    {const_cast<char *>("__signature__"), (getter)pyside_sm_get___signature__,
                                          (setter)pyside_set___signature__},
    {nullptr}
};

static PyGetSetDef new_PyMethodDescr_getsets[] = {
    {const_cast<char *>("__doc__"), (getter)pyside_md_get___doc__},
    {const_cast<char *>("__signature__"), (getter)pyside_md_get___signature__,
                                          (setter)pyside_set___signature__},
    {nullptr}
};

static PyGetSetDef new_PyType_getsets[] = {
    {const_cast<char *>("__doc__"), (getter)pyside_tp_get___doc__},
    {const_cast<char *>("__signature__"), (getter)pyside_tp_get___signature__,
                                          (setter)pyside_set___signature__},
    {nullptr}
};

static PyGetSetDef new_PyWrapperDescr_getsets[] = {
    {const_cast<char *>("__doc__"), (getter)pyside_wd_get___doc__},
    {const_cast<char *>("__signature__"), (getter)pyside_wd_get___signature__,
                                          (setter)pyside_set___signature__},
    {nullptr}
};

int PySide_PatchTypes(void)
{
    static int init_done = 0;

    if (!init_done) {
        AutoDecRef meth_descr(PyObject_GetAttrString(
                                    reinterpret_cast<PyObject *>(&PyString_Type), "split"));
        AutoDecRef wrap_descr(PyObject_GetAttrString(
                                    reinterpret_cast<PyObject *>(Py_TYPE(Py_True)), "__add__"));
        // abbreviations for readability
        auto md_gs = new_PyMethodDescr_getsets;
        auto md_doc = &old_md_doc_descr;
        auto cf_gs = new_PyCFunction_getsets;
        auto cf_doc = &old_cf_doc_descr;
        auto sm_gs = new_PyStaticMethod_getsets;
        auto sm_doc = &old_sm_doc_descr;
        auto tp_gs = new_PyType_getsets;
        auto tp_doc = &old_tp_doc_descr;
        auto wd_gs = new_PyWrapperDescr_getsets;
        auto wd_doc = &old_wd_doc_descr;

        if (meth_descr.isNull() || wrap_descr.isNull()
            || PyType_Ready(Py_TYPE(meth_descr)) < 0
            || add_more_getsets(PepMethodDescr_TypePtr,  md_gs, md_doc) < 0
            || add_more_getsets(&PyCFunction_Type,       cf_gs, cf_doc) < 0
            || add_more_getsets(PepStaticMethod_TypePtr, sm_gs, sm_doc) < 0
            || add_more_getsets(&PyType_Type,            tp_gs, tp_doc) < 0
            || add_more_getsets(Py_TYPE(wrap_descr),     wd_gs, wd_doc) < 0
            )
            return -1;
        init_done = 1;
    }
    return 0;
}

} // extern "C"
