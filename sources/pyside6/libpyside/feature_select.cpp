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
#include "pyside.h"
#include "pysidestaticstrings.h"
#include "class_property.h"

#include <shiboken.h>
#include <sbkstaticstrings.h>

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

typedef bool(*FeatureProc)(PyTypeObject *type, PyObject *prev_dict, int id);

static FeatureProc *featurePointer = nullptr;

static PyObject *cached_globals = nullptr;
static PyObject *last_select_id = nullptr;

static PyObject *_fast_id_array[1 + 256] = {};
// this will point to element 1 to allow indexing from -1
static PyObject **fast_id_array;

static inline PyObject *getFeatureSelectId()
{
    static PyObject *undef = fast_id_array[-1];
    static PyObject *feature_dict = GetFeatureDict();
    // these things are all borrowed
    PyObject *globals = PyEval_GetGlobals();
    if (   globals == nullptr
        || globals == cached_globals)
        return last_select_id;

    PyObject *modname = PyDict_GetItem(globals, PyMagicName::name());
    if (modname == nullptr)
        return last_select_id;

    PyObject *select_id = PyDict_GetItem(feature_dict, modname);
    if (   select_id == nullptr
        || !PyInt_Check(select_id)  // int/long cheating
        || select_id == undef)
        return last_select_id;

    cached_globals = globals;
    last_select_id = select_id;
    assert(PyInt_AsSsize_t(select_id) >= 0);
    return select_id;
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
            Py_FatalError("PySide6: Problem creating ChameleonDict");
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

static inline void setCurrentSelectId(PyTypeObject *type, PyObject *select_id)
{
    SbkObjectType_SetReserved(type, PyInt_AsSsize_t(select_id));    // int/long cheating
}

static inline void setCurrentSelectId(PyTypeObject *type, int id)
{
    SbkObjectType_SetReserved(type, id);
}

static inline PyObject *getCurrentSelectId(PyTypeObject *type)
{
    int id = SbkObjectType_GetReserved(type);
    // This can be too early.
    if (id < 0)
        id = 0;
    return fast_id_array[id];
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
            setCurrentSelectId(type, select_id);
            return true;
        }
    } while (dict != initial_dict);
    type->tp_dict = initial_dict;
    return false;
}

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

    static auto zero = fast_id_array[0];
    bool ok = moveToFeatureSet(type, zero);
    Q_UNUSED(ok);
    assert(ok);

    AutoDecRef prev_dict(type->tp_dict);
    Py_INCREF(prev_dict);   // keep the first ref unchanged
    if (!addNewDict(type, select_id))
        return false;
    auto id = PyInt_AsSsize_t(select_id);   // int/long cheating
    if (id == -1)
        return false;
    setCurrentSelectId(type, id);
    FeatureProc *proc = featurePointer;
    for (int idx = id; *proc != nullptr; ++proc, idx >>= 1) {
        if (idx & 1) {
            // clear the tp_dict that will get new content
            PyDict_Clear(type->tp_dict);
            // let the proc re-fill the tp_dict
            if (!(*proc)(type, prev_dict, id))
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
    PyObject *select_id = getFeatureSelectId();         // borrowed
    PyObject *current_id = getCurrentSelectId(type);    // borrowed
    static PyObject *undef = fast_id_array[-1];

    // PYSIDE-1019: During import PepType_SOTP is still zero.
    if (current_id == undef)
        current_id = select_id = fast_id_array[0];

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
    if (featurePointer == nullptr)
        return;
    auto type = Py_TYPE(obj);
    type->tp_dict = SelectFeatureSet(type);
}

PyObject *Select(PyTypeObject *type)
{
    if (featurePointer != nullptr)
        type->tp_dict = SelectFeatureSet(type);
    return type->tp_dict;
}

static bool feature_01_addLowerNames(PyTypeObject *type, PyObject *prev_dict, int id);
static bool feature_02_true_property(PyTypeObject *type, PyObject *prev_dict, int id);
static bool feature_04_addDummyNames(PyTypeObject *type, PyObject *prev_dict, int id);
static bool feature_08_addDummyNames(PyTypeObject *type, PyObject *prev_dict, int id);
static bool feature_10_addDummyNames(PyTypeObject *type, PyObject *prev_dict, int id);
static bool feature_20_addDummyNames(PyTypeObject *type, PyObject *prev_dict, int id);
static bool feature_40_addDummyNames(PyTypeObject *type, PyObject *prev_dict, int id);
static bool feature_80_addDummyNames(PyTypeObject *type, PyObject *prev_dict, int id);

static FeatureProc featureProcArray[] = {
    feature_01_addLowerNames,
    feature_02_true_property,
    feature_04_addDummyNames,
    feature_08_addDummyNames,
    feature_10_addDummyNames,
    feature_20_addDummyNames,
    feature_40_addDummyNames,
    feature_80_addDummyNames,
    nullptr
};

void finalize()
{
    for (int idx = -1; idx < 256; ++idx)
        Py_DECREF(fast_id_array[idx]);
}

static bool patch_property_impl();

void init()
{
    // This function can be called multiple times.
    static bool is_initialized = false;
    if (!is_initialized) {
        fast_id_array = &_fast_id_array[1];
        for (int idx = -1; idx < 256; ++idx)
            fast_id_array[idx] = PyInt_FromLong(idx);
        last_select_id = fast_id_array[0];
        featurePointer = featureProcArray;
        initSelectableFeature(SelectFeatureSet);
        registerCleanupFunction(finalize);
        patch_property_impl();
        PySide::ClassProperty::init();
        is_initialized = true;
    }
    // Reset the cache. This is called at any "from __feature__ import".
    cached_globals = nullptr;
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

static bool feature_01_addLowerNames(PyTypeObject *type, PyObject *prev_dict, int id)
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
    if (!meth)
        return true;

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
//     Feature 0x02: Use true properties instead of getters and setters
//

// This is the Python 2 version for inspection of m_ml, only.
// The actual Python 3 version is larget.

typedef struct {
    PyObject_HEAD
    PyMethodDef *m_ml; /* Description of the C function to call */
    PyObject    *m_self; /* Passed as 'self' arg to the C func, can be NULL */
    PyObject    *m_module; /* The __module__ attribute, can be anything */
} PyCFunctionObject;

static PyObject *modifyStaticToClassMethod(PyTypeObject *type, PyObject *sm)
{
    AutoDecRef func_ob(PyObject_GetAttr(sm, PyMagicName::func()));
    if (func_ob.isNull())
        return nullptr;
    auto func = reinterpret_cast<PyCFunctionObject *>(func_ob.object());
    auto new_func = new PyMethodDef;
    new_func->ml_name = func->m_ml->ml_name;
    new_func->ml_meth = func->m_ml->ml_meth;
    new_func->ml_flags = (func->m_ml->ml_flags & ~METH_STATIC) | METH_CLASS;
    new_func->ml_doc = func->m_ml->ml_doc;
    auto cfunc = PyCFunction_NewEx(new_func, nullptr, nullptr);
    cfunc = PyDescr_NewClassMethod(type, new_func);
    return cfunc;
}

static PyObject *createProperty(PyTypeObject *type, PyObject *getter, PyObject *setter)
{
    bool chassprop = false;
    assert(getter != nullptr);
    if (setter == nullptr)
        setter = Py_None;
    auto ptype = &PyProperty_Type;
    if (Py_TYPE(getter) == PepStaticMethod_TypePtr) {
        ptype = PyClassPropertyTypeF();
        chassprop = true;
        getter = modifyStaticToClassMethod(type, getter);
        if (setter != Py_None)
            setter = modifyStaticToClassMethod(type, setter);
    }
    auto obtype = reinterpret_cast<PyObject *>(ptype);
    PyObject *prop = PyObject_CallFunctionObjArgs(obtype, getter, setter, nullptr);
    return prop;
}

static QStringList parseFields(const char *propstr)
{
    /*
     * Break the string into subfields at ':' and add defaults.
     */
    QString s = QString(QLatin1String(propstr));
    auto list = s.split(QLatin1Char(':'));
    assert(list.size() == 2 || list.size() == 3);
    auto name = list[0];
    auto read = list[1];
    if (read.size() == 0)
        list[1] = name;
    if (list.size() == 2)
        return list;
    auto write = list[2];
    if (write.size() == 0) {
        list[2] = QLatin1String("set") + name;
        list[2][3] = list[2][3].toUpper();
    }
    return list;
}

static PyObject *make_snake_case(QString s, bool lower)
{
    if (s.isNull())
        return nullptr;
    return String::getSnakeCaseName(s.toLatin1().data(), lower);
}

static bool feature_02_true_property(PyTypeObject *type, PyObject *prev_dict, int id)
{
    /*
     * Use the property info to create true Python property objects.
     */

    // The empty `tp_dict` gets populated by the previous dict.
    PyObject *prop_dict = type->tp_dict;
    if (PyDict_Update(prop_dict, prev_dict) < 0)
        return false;

    // We then replace methods by properties.
    bool lower = (id & 0x01) != 0;
    auto props = SbkObjectType_GetPropertyStrings(type);
    if (props == nullptr || *props == nullptr)
        return true;
    for (; *props != nullptr; ++props) {
        auto propstr = *props;
        auto fields = parseFields(propstr);
        bool haveWrite = fields.size() == 3;
        PyObject *name = make_snake_case(fields[0], lower);
        PyObject *read = make_snake_case(fields[1], lower);
        PyObject *write = haveWrite ? make_snake_case(fields[2], lower) : nullptr;
        PyObject *getter = PyDict_GetItem(prev_dict, read);
        if (getter == nullptr || !(Py_TYPE(getter) == PepMethodDescr_TypePtr ||
                                   Py_TYPE(getter) == PepStaticMethod_TypePtr))
            continue;
        PyObject *setter = haveWrite ? PyDict_GetItem(prev_dict, write) : nullptr;

        AutoDecRef PyProperty(createProperty(type, getter, setter));
        if (PyProperty.isNull())
            return false;
        if (PyDict_SetItem(prop_dict, name, PyProperty) < 0)
            return false;
        if (fields[0] != fields[1] && PyDict_GetItem(prop_dict, read))
            if (PyDict_DelItem(prop_dict, read) < 0)
                return false;
        // Theoretically, we need to check for multiple signatures to be exact.
        // But we don't do so intentionally because it would be confusing.
        if (haveWrite && PyDict_GetItem(prop_dict, write))
            if (PyDict_DelItem(prop_dict, write) < 0)
                return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// These are a number of patches to make Python's property object better
// suitable for us.
// We turn `__doc__` into a lazy attribute saving signature initialization.
//
// There is now also a class property implementation which inherits
// from this one.
//

static PyObject *property_doc_get(PyObject *self, void *)
{
    auto po = reinterpret_cast<propertyobject *>(self);

    if (po->prop_doc != nullptr && po->prop_doc != Py_None) {
        Py_INCREF(po->prop_doc);
        return po->prop_doc;
    }
    if (po->prop_get) {
        // PYSIDE-1019: Fetch the default `__doc__` from fget. We do it late.
        auto txt = PyObject_GetAttr(po->prop_get, PyMagicName::doc());
        if (txt != nullptr) {
            Py_INCREF(txt);
            po->prop_doc = txt;
            Py_INCREF(txt);
            return txt;
        }
        PyErr_Clear();
    }
    Py_RETURN_NONE;
}

static int property_doc_set(PyObject *self, PyObject *value, void *)
{
    auto po = reinterpret_cast<propertyobject *>(self);

    Py_INCREF(value);
    po->prop_doc = value;
    return 0;
}

static PyGetSetDef property_getset[] = {
    // This gets added to the existing getsets
    {const_cast<char *>("__doc__"), property_doc_get, property_doc_set, nullptr, nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr}
};

static bool patch_property_impl()
{
    // Turn `__doc__` into a computed attribute without changing writability.
    auto gsp = property_getset;
    auto type = &PyProperty_Type;
    auto dict = type->tp_dict;
    AutoDecRef descr(PyDescr_NewGetSet(type, gsp));
    if (descr.isNull())
        return false;
    if (PyDict_SetItemString(dict, gsp->name, descr) < 0)
        return false;
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// PYSIDE-1019: Support switchable extensions
//
//     Feature 0x04..0x40: A fake switchable option for testing
//

#define SIMILAR_FEATURE(xx)  \
static bool feature_##xx##_addDummyNames(PyTypeObject *type, PyObject *prev_dict, int id) \
{ \
    PyObject *dict = type->tp_dict; \
    if (PyDict_Update(dict, prev_dict) < 0) \
        return false; \
    if (PyDict_SetItemString(dict, "fake_feature_" #xx, Py_None) < 0) \
        return false; \
    return true; \
}

SIMILAR_FEATURE(04)
SIMILAR_FEATURE(08)
SIMILAR_FEATURE(10)
SIMILAR_FEATURE(20)
SIMILAR_FEATURE(40)
SIMILAR_FEATURE(80)

} // namespace PySide
} // namespace Feature
