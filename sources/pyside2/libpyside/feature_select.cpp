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
// basewrapper.cpp file.
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
This way, we can replace the methods of the dict in no time.

The crucial point to understand is how the `tp_dict` is actually accessed:
When you type "QObject.__dict__", the descriptor of SbkObjectType_Type
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

This is everything that the following code does.

*****************************************************************************/


namespace PySide { namespace FeatureSelector {

using namespace Shiboken;

static PyObject *getFeatureSelectID()
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

static PyTypeObject *old_dict_type = Py_TYPE(PyType_Type.tp_dict);
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
    assert(Py_TYPE(dict) != old_dict_type);
    AutoDecRef next_dict(PyObject_GetAttr(dict, PyName::dict_ring()));
    return next_dict;
}

static inline void setNextDict(PyObject *dict, PyObject *next_dict)
{
    assert(Py_TYPE(dict) != old_dict_type);
    PyObject_SetAttr(dict, PyName::dict_ring(), next_dict);
}

static inline void setSelectId(PyObject *dict, PyObject *select_id)
{
    assert(Py_TYPE(dict) != old_dict_type);
    PyObject_SetAttr(dict, PyName::select_id(), select_id);
}

static inline PyObject *getSelectId(PyObject *dict)
{
    assert(Py_TYPE(dict) != old_dict_type);
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
    int id = PyInt_AsSsize_t(select_id);
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

static PyObject *SelectFeatureSet(PyTypeObject *type)
{
    /*
     *  This is the main function of the module.
     *  It just makes no sense to make the function public, because
     *  Shiboken will assign it via a public hook of `basewrapper.cpp`.
     */
    if (Py_TYPE(type->tp_dict) == old_dict_type) {
        // PYSIDE-1019: On first touch, we initialize the dynamic naming.
        // The dict type will be replaced after the first call.
        if (!replaceClassDict(type))
            return nullptr;
    }
    PyObject *select_id = getFeatureSelectID();     // borrowed
    AutoDecRef current_id(getSelectId(type->tp_dict));
    if (select_id != current_id) {
        if (!moveToFeatureSet(type, select_id))
            if (!createNewFeatureSet(type, select_id)) {
                Py_FatalError("failed to create a new feature set!");
                return nullptr;
            }
    }
    return type->tp_dict;
}

static bool feature_01_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_02_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_04_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_08_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_10_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_20_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_40_addDummyNames(PyTypeObject *type, PyObject *prev_dict);
static bool feature_80_addDummyNames(PyTypeObject *type, PyObject *prev_dict);

static FeatureProc featureProcArray[] = {
    feature_01_addDummyNames,
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
//     Feature 0x01..0x80: A fake switchable option for testing
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

SIMILAR_FEATURE(01)
SIMILAR_FEATURE(02)
SIMILAR_FEATURE(04)
SIMILAR_FEATURE(08)
SIMILAR_FEATURE(10)
SIMILAR_FEATURE(20)
SIMILAR_FEATURE(40)
SIMILAR_FEATURE(80)

} // namespace PySide
} // namespace FeatureSelector
