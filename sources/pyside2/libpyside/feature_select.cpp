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

#include "feature_select.h"

#include <shiboken.h>
#include <sbkstaticstrings.h>

#include <QtCore/QtGlobal>

//////////////////////////////////////////////////////////////////////////////
//
// PYSIDE-1019: Support switchable extensions
//
// This functionality is no longer implemented in the signature module, since
// the PyCFunction getsets do not have to be modified any longer.
// Instead, we simply exchange the complete class dicts. This is done in the
// basewrapper.cpp file and in every generated `tp_(get|set)attro`.
//
// This is the general framework of the switchable extensions.
// A maximum of eight features is planned so far. This seems to be enough.
// More features are possible, but then we must somehow register the
// extra `select_id`s above 255.
//

/*****************************************************************************

    How Does This Feature Selection Work?
    -------------------------------------

The basic idea is to replace the `tp_dict` of a QObject derived type.
This way, we can replace the methods of the class in no time.

The crucial point to understand is how the `tp_dict` is actually accessed:
When you type "QObject.__dict__", the descriptor of `SbkObjectType_Type`
is called. This descriptor is per default unassigned, so the base class
PyType_Type provides the tp_getset method `type_dict`:

    static PyObject *
    type_dict(PyTypeObject *type, void *context)
    {
        if (type->tp_dict == NULL) {
            Py_RETURN_NONE;
        }
        return PyDictProxy_New(type->tp_dict);
    }

In order to change that, we need to insert our own version into SbkObjectType:

    static PyObject *Sbk_TypeGet___dict__(PyTypeObject *type, void *context)
    {
        auto dict = type->tp_dict;
        if (dict == NULL)
            Py_RETURN_NONE;
        if (SelectFeatureSet != nullptr)
            dict = SelectFeatureSet(type);
        return PyDictProxy_New(dict);
    }

This way, the Python function `type_ready()` does not fill in the default,
but uses our modified version. It a similar way, we overwrite type_getattro
with our own version, again in SbkObjectType, replacing the default of
PyType_Type.

Now we can exchange the dict with a customized version.
We have our own derived type `ChameleonDict` with additional attributes.
These allow us to create a ring of dicts which can be rotated to the actual
needed dict version:

Every dict has a field `select_id` which is selected by the `from __feature__`
import. The dicts are cyclic connected by the `dict_ring` field.

When a class dict is required, now always `SelectFeatureSet` is called, which
looks into the `__name__` attribute of the active module and decides which
version of `tp_dict` is needed. Then the right dict is searched in the ring
and created if not already there.

Furthermore, we need to overwrite every `tp_(get|set)attro`  with a version
that switches dicts right before looking up methods.
The dict changing must walk the whole `tp_mro` in order to change all names.

This is everything that the following code does.

*****************************************************************************/


namespace PySide { namespace Feature {

using namespace Shiboken;

static inline PyObject *getFeatureSelectID()
{
    static PyObject *zero = PyInt_FromLong(0);
    static PyObject *feature_dict = GetFeatureDict();
    // these things are all borrowed
    PyObject *globals = PyEval_GetGlobals();
    if (globals == nullptr)
        return zero;
    PyObject *modname = PyDict_GetItem(globals, PyMagicName::name());
    if (modname == nullptr)
        return zero;
    PyObject *flag = PyDict_GetItem(feature_dict, modname);
    if (flag == nullptr || !PyInt_Check(flag))  // int/long cheating
        return zero;
    return flag;
}

// Create a derived dict class
static PyTypeObject *
createDerivedDictType()
{
    // It is not easy to create a compatible dict object with the
    // limited API. Easier is to use Python to create a derived
    // type and to modify that a bit from the C code.
    PyObject *ChameleonDict = PepRun_GetResult(R"CPP(if True:

        class ChameleonDict(dict):
            __slots__ = ("dict_ring", "select_id")

        result = ChameleonDict

        )CPP");
    return reinterpret_cast<PyTypeObject *>(ChameleonDict);
}

static PyTypeObject *new_dict_type = nullptr;

static void ensureNewDictType()
{
    if (new_dict_type == nullptr) {
        new_dict_type = createDerivedDictType();
        if (new_dict_type == nullptr)
            Py_FatalError("PySide2: Problem creating ChameleonDict");
    }
}

static inline PyObject *nextInCircle(PyObject *dict)
{
    // returns a borrowed ref
    AutoDecRef next_dict(PyObject_GetAttr(dict, PyName::dict_ring()));
    return next_dict;
}

static inline void setNextDict(PyObject *dict, PyObject *next_dict)
{
    PyObject_SetAttr(dict, PyName::dict_ring(), next_dict);
}

static inline void setSelectId(PyObject *dict, PyObject *select_id)
{
    PyObject_SetAttr(dict, PyName::select_id(), select_id);
}

static inline PyObject *getSelectId(PyObject *dict)
{
    auto select_id = PyObject_GetAttr(dict, PyName::select_id());
    return select_id;
}

static bool replaceClassDict(PyTypeObject *type)
{
    /*
     * Replace the type dict by the derived ChameleonDict.
     * This is mandatory for all type dicts when they are touched.
     */
    ensureNewDictType();
    PyObject *dict = type->tp_dict;
    auto ob_ndt = reinterpret_cast<PyObject *>(new_dict_type);
    PyObject *new_dict = PyObject_CallObject(ob_ndt, nullptr);
    if (new_dict == nullptr || PyDict_Update(new_dict, dict) < 0)
        return false;
    // Insert the default id. Cannot fail for small numbers.
    AutoDecRef select_id(PyInt_FromLong(0));
    setSelectId(new_dict, select_id);
    // insert the dict into itself as ring
    setNextDict(new_dict, new_dict);
    // We have now an exact copy of the dict with a new type.
    // Replace `__dict__` which usually has refcount 1 (but see cyclic_test.py)
    Py_DECREF(type->tp_dict);
    type->tp_dict = new_dict;
    return true;
}

static bool addNewDict(PyTypeObject *type, PyObject *select_id)
{
    /*
     * Add a new dict to the ring and set it as `type->tp_dict`.
     * A 'false' return is fatal.
     */
    auto dict = type->tp_dict;
    auto ob_ndt = reinterpret_cast<PyObject *>(new_dict_type);
    auto new_dict = PyObject_CallObject(ob_ndt, nullptr);
    if (new_dict == nullptr)
        return false;
    setSelectId(new_dict, select_id);
    // insert the dict into the ring
    auto next_dict = nextInCircle(dict);
    setNextDict(dict, new_dict);
    setNextDict(new_dict, next_dict);
    type->tp_dict = new_dict;
    return true;
}

static bool moveToFeatureSet(PyTypeObject *type, PyObject *select_id)
{
    /*
     * Rotate the ring to the given `select_id` and return `true`.
     * If not found, stay at the current position and return `false`.
     */
    auto initial_dict = type->tp_dict;
    auto dict = initial_dict;
    do {
        dict = nextInCircle(dict);
        AutoDecRef current_id(getSelectId(dict));
        // This works because small numbers are singleton objects.
        if (current_id == select_id) {
            type->tp_dict = dict;
            return true;
        }
    } while (dict != initial_dict);
    type->tp_dict = initial_dict;
    return false;
}

typedef bool(*FeatureProc)(PyTypeObject *type, PyObject *prev_dict);

static FeatureProc *featurePointer = nullptr;

static bool createNewFeatureSet(PyTypeObject *type, PyObject *select_id)
{
    /*
     * Create a new feature set.
     * A `false` return value is a fatal error.
     *
     * A FeatureProc sees an empty `type->tp_dict` and the previous dict
     * content in `prev_dict`. It is responsible of filling `type->tp_dict`
     * with modified content.
     */
    static auto small_1 = PyInt_FromLong(255);
    Q_UNUSED(small_1);
    static auto small_2 = PyInt_FromLong(255);
    Q_UNUSED(small_2);
    // make sure that small integers are cached
    assert(small_1 != nullptr && small_1 == small_2);

    static auto zero = PyInt_FromLong(0);
    bool ok = moveToFeatureSet(type, zero);
    Q_UNUSED(ok);
    assert(ok);

    AutoDecRef prev_dict(type->tp_dict);
    Py_INCREF(prev_dict);
    if (!addNewDict(type, select_id))
        return false;
    auto id = PyInt_AsSsize_t(select_id);
    if (id == -1)
        return false;
    FeatureProc *proc = featurePointer;
    for (int idx = id; *proc != nullptr; ++proc, idx >>= 1) {
        if (idx & 1) {
            // clear the tp_dict that will get new content
            PyDict_Clear(type->tp_dict);
            // let the proc re-fill the tp_dict
            if (!(*proc)(type, prev_dict))
                return false;
            // if there is still a step, prepare `prev_dict`
            if (idx >> 1) {
                prev_dict.reset(PyDict_Copy(type->tp_dict));
                if (prev_dict.isNull())
                    return false;
            }
        }
    }
    return true;
}

static bool SelectFeatureSetSubtype(PyTypeObject *type, PyObject *select_id)
{
    /*
     * This is the selector for one sublass. We need to call this for
     * every subclass until no more subclasses or reaching the wanted id.
     */
    if (Py_TYPE(type->tp_dict) == Py_TYPE(PyType_Type.tp_dict)) {
        // On first touch, we initialize the dynamic naming.
        // The dict type will be replaced after the first call.
        if (!replaceClassDict(type)) {
            Py_FatalError("failed to replace class dict!");
            return false;
        }
    }
    if (!moveToFeatureSet(type, select_id)) {
        if (!createNewFeatureSet(type, select_id)) {
            Py_FatalError("failed to create a new feature set!");
            return false;
        }
    }
    return true;
}

static inline PyObject *SelectFeatureSet(PyTypeObject *type)
{
    /*
     * This is the main function of the module.
     * The purpose of this function is to switch the dict of a class right
     * before a (get|set)attro call is performed.
     *
     * Generated functions call this directly.
     * Shiboken will assign it via a public hook of `basewrapper.cpp`.
     */
    if (Py_TYPE(type->tp_dict) == Py_TYPE(PyType_Type.tp_dict)) {
        // We initialize the dynamic features by using our own dict type.
        if (!replaceClassDict(type))
            return nullptr;
    }
    PyObject *select_id = getFeatureSelectID();     // borrowed
    AutoDecRef current_id(getSelectId(type->tp_dict));
    if (select_id != current_id) {
        PyObject *mro = type->tp_mro;
        Py_ssize_t idx, n = PyTuple_GET_SIZE(mro);
        // We leave 'Shiboken.Object' and 'object' alone, therefore "n - 2".
        for (idx = 0; idx < n - 2; idx++) {
            auto *sub_type = reinterpret_cast<PyTypeObject *>(PyTuple_GET_ITEM(mro, idx));
            // When any subtype is already resolved (false), we can stop.
            if (!SelectFeatureSetSubtype(sub_type, select_id))
                break;
        }
    }
    return type->tp_dict;
}

// For cppgenerator:
void Select(PyObject *obj)
{
    auto type = Py_TYPE(obj);
    type->tp_dict = SelectFeatureSet(type);
}

static bool feature_01_addLowerNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_02_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_04_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_08_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_10_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_20_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_40_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_80_addDummyNames(PyTypeObject *type, PyObject *prev_dict);

static FeatureProc featureProcArray[] = {
    feature_01_addLowerNames,
    feature_02_addDummyNames,
    feature_04_addDummyNames,
    feature_08_addDummyNames,
    feature_10_addDummyNames,
    feature_20_addDummyNames,
    feature_40_addDummyNames,
    feature_80_addDummyNames,
    nullptr
};

void init()
{
    featurePointer = featureProcArray;
    initSelectableFeature(SelectFeatureSet);
}

//////////////////////////////////////////////////////////////////////////////
//
// PYSIDE-1019: Support switchable extensions
//
//     Feature 0x01: Allow snake_case instead of camelCase
//
// This functionality is no longer implemented in the signature module, since
// the PyCFunction getsets do not have to be modified any longer.
// Instead, we simply exchange the complete class dicts. This is done in the
// basewrapper.cpp file.
//

static PyObject *methodWithNewName(PyTypeObject *type,
                                     PyMethodDef *meth,
                                     const char *new_name)
{
    /*
     * Create a method with a lower case name.
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
    PyObject *descr = nullptr;
    if (new_meth->ml_flags & METH_STATIC) {
        AutoDecRef cfunc(PyCFunction_NewEx(new_meth, obtype, nullptr));
        if (cfunc.isNull())
            return nullptr;
        descr = PyStaticMethod_New(cfunc);
    }
    else {
        descr = PyDescr_NewMethod(type, new_meth);
    }
    return descr;
}

static bool feature_01_addLowerNames(PyTypeObject *type, PyObject *prev_dict)
{
    /*
     * Add objects with lower names to `type->tp_dict` from 'prev_dict`.
     */
    PyObject *lower_dict = type->tp_dict;
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    // We first copy the things over which will not be changed:
    while (PyDict_Next(prev_dict, &pos, &key, &value)) {
        if (   Py_TYPE(value) != PepMethodDescr_TypePtr
            && Py_TYPE(value) != PepStaticMethod_TypePtr) {
            if (PyDict_SetItem(lower_dict, key, value))
                return false;
            continue;
        }
    }
    // Then we walk over the tp_methods to get all methods and insert
    // them with changed names.
    PyMethodDef *meth = type->tp_methods;
    for (; meth != nullptr && meth->ml_name != nullptr; ++meth) {
        const char *name = String::toCString(String::getSnakeCaseName(meth->ml_name, true));
        AutoDecRef new_method(methodWithNewName(type, meth, name));
        if (new_method.isNull())
            return false;
        if (PyDict_SetItemString(lower_dict, name, new_method) < 0)
            return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// PYSIDE-1019: Support switchable extensions
//
//     Feature 0x02..0x80: A fake switchable option for testing
//

#define SIMILAR_FEATURE(xx)  \
static bool feature_##xx##_addDummyNames(PyTypeObject *type, PyObject *prev_dict) \
{ \
    PyObject *dict = type->tp_dict; \
    if (PyDict_Update(dict, prev_dict) < 0) \
        return false; \
    Py_INCREF(Py_None); \
    if (PyDict_SetItemString(dict, "fake_feature_" #xx, Py_None) < 0) \
        return false; \
    return true; \
}

SIMILAR_FEATURE(02)
SIMILAR_FEATURE(04)
SIMILAR_FEATURE(08)
SIMILAR_FEATURE(10)
SIMILAR_FEATURE(20)
SIMILAR_FEATURE(40)
SIMILAR_FEATURE(80)

} // namespace PySide
} // namespace Feature
