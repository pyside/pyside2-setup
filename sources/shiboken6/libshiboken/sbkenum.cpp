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

#include "sbkenum.h"
#include "sbkstring.h"
#include "sbkstaticstrings.h"
#include "sbkstaticstrings_p.h"
#include "sbkconverter.h"
#include "basewrapper.h"
#include "sbkdbg.h"
#include "autodecref.h"
#include "sbkpython.h"

#include <string.h>
#include <cstring>
#include <vector>

#define SBK_ENUM(ENUM) reinterpret_cast<SbkEnumObject *>(ENUM)
#define SBK_TYPE_CHECK(o) (strcmp(Py_TYPE(Py_TYPE(o))->tp_name, "Shiboken.EnumType") == 0)
typedef PyObject *(*enum_func)(PyObject *, PyObject *);

extern "C"
{

struct SbkEnumTypePrivate
{
    SbkConverter **converterPtr;
    SbkConverter *converter;
    const char *cppName;
};

struct SbkEnumType
{
    PyTypeObject type;
};

struct SbkEnumObject
{
    PyObject_HEAD
    long ob_value;
    PyObject *ob_name;
};

static PyObject *SbkEnumObject_repr(PyObject *self)
{
    const SbkEnumObject *enumObj = SBK_ENUM(self);
    if (enumObj->ob_name)
        return Shiboken::String::fromFormat("%s.%s", (Py_TYPE(self))->tp_name, PyBytes_AS_STRING(enumObj->ob_name));
    else
        return Shiboken::String::fromFormat("%s(%ld)", (Py_TYPE(self))->tp_name, enumObj->ob_value);
}

static PyObject *SbkEnumObject_name(PyObject *self, void *)
{
    auto *enum_self = SBK_ENUM(self);

    if (enum_self->ob_name == nullptr)
        Py_RETURN_NONE;

    Py_INCREF(enum_self->ob_name);
    return enum_self->ob_name;
}

static PyObject *SbkEnum_tp_new(PyTypeObject *type, PyObject *args, PyObject *)
{
    long itemValue = 0;
    if (!PyArg_ParseTuple(args, "|l:__new__", &itemValue))
        return nullptr;

    SbkEnumObject *self = PyObject_New(SbkEnumObject, type);
    if (!self)
        return nullptr;
    self->ob_value = itemValue;
    Shiboken::AutoDecRef item(Shiboken::Enum::getEnumItemFromValue(type, itemValue));
    self->ob_name = item.object() ? SbkEnumObject_name(item, nullptr) : nullptr;
    return reinterpret_cast<PyObject *>(self);
}

void enum_object_dealloc(PyObject *ob)
{
    auto self = reinterpret_cast<SbkEnumObject *>(ob);
    Py_XDECREF(self->ob_name);
    Sbk_object_dealloc(ob);
}

static PyObject *enum_op(enum_func f, PyObject *a, PyObject *b) {
    PyObject *valA = a;
    PyObject *valB = b;
    PyObject *result = nullptr;
    bool enumA = false;
    bool enumB = false;

    // We are not allowing floats
    if (!PyFloat_Check(valA) && !PyFloat_Check(valB)) {
        // Check if both variables are SbkEnumObject
        if (SBK_TYPE_CHECK(valA)) {
            valA = PyLong_FromLong(SBK_ENUM(valA)->ob_value);
            enumA = true;
        }
        if (SBK_TYPE_CHECK(valB)) {
            valB = PyLong_FromLong(SBK_ENUM(valB)->ob_value);
            enumB = true;
        }
    }

    // Without an enum we are not supporting the operation
    if (!(enumA || enumB)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    } else {
        result = f(valA, valB);
    }

    // Decreasing the reference of the used variables a and b.
    if (enumA)
        Py_DECREF(valA);
    if (enumB)
        Py_DECREF(valB);

    return result;
}

/* Notes:
 *   On Py3k land we use long type when using integer numbers. However, on older
 *   versions of Python (version 2) we need to convert it to int type,
 *   respectively.
 *
 *   Thus calling PyInt_FromLong() will result in calling PyLong_FromLong in
 *   Py3k.
 */
static PyObject *enum_int(PyObject *v)
{
    return PyInt_FromLong(SBK_ENUM(v)->ob_value);
}

static PyObject *enum_and(PyObject *self, PyObject *b)
{
    return enum_op(PyNumber_And, self, b);
}

static PyObject *enum_or(PyObject *self, PyObject *b)
{
return enum_op(PyNumber_Or, self, b);
}

static PyObject *enum_xor(PyObject *self, PyObject *b)
{
    return enum_op(PyNumber_Xor, self, b);
}

static int enum_bool(PyObject *v)
{
    return (SBK_ENUM(v)->ob_value > 0);
}

static PyObject *enum_add(PyObject *self, PyObject *v)
{
    return enum_op(PyNumber_Add, self, v);
}

static PyObject *enum_subtract(PyObject *self, PyObject *v)
{
    return enum_op(PyNumber_Subtract, self, v);
}

static PyObject *enum_multiply(PyObject *self, PyObject *v)
{
return enum_op(PyNumber_Multiply, self, v);
}

static PyObject *enum_richcompare(PyObject *self, PyObject *other, int op)
{
    PyObject *valA = self;
    PyObject *valB = other;
    PyObject *result = nullptr;
    bool enumA = false;
    bool enumB = false;

    // We are not allowing floats
    if (!PyFloat_Check(valA) && !PyFloat_Check(valB)) {

        // Check if both variables are SbkEnumObject
        if (SBK_TYPE_CHECK(valA)) {
            valA = PyLong_FromLong(SBK_ENUM(valA)->ob_value);
            enumA = true;
        }
        if (SBK_TYPE_CHECK(valB)) {
            valB = PyLong_FromLong(SBK_ENUM(valB)->ob_value);
            enumB =true;
        }
    }

    // Without an enum we are not supporting the operation
    if (!(enumA || enumB)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    result = PyObject_RichCompare(valA, valB, op);

    // Decreasing the reference of the used variables a and b.
    if (enumA)
        Py_DECREF(valA);
    if (enumB)
        Py_DECREF(valB);

    return result;
}

static Py_hash_t enum_hash(PyObject *pyObj)
{
    Py_hash_t val = reinterpret_cast<SbkEnumObject *>(pyObj)->ob_value;
    if (val == -1)
        val = -2;
    return val;
}

static PyGetSetDef SbkEnumGetSetList[] = {
    {const_cast<char *>("name"), &SbkEnumObject_name, nullptr, nullptr, nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} // Sentinel
};

static void SbkEnumTypeDealloc(PyObject *pyObj);
static PyObject *SbkEnumTypeTpNew(PyTypeObject *metatype, PyObject *args, PyObject *kwds);

static PyType_Slot SbkEnumType_Type_slots[] = {
    {Py_tp_dealloc, (void *)SbkEnumTypeDealloc},
    {Py_nb_add, (void *)enum_add},
    {Py_nb_subtract, (void *)enum_subtract},
    {Py_nb_multiply, (void *)enum_multiply},
    {Py_nb_positive, (void *)enum_int},
    {Py_nb_bool, (void *)enum_bool},
    {Py_nb_and, (void *)enum_and},
    {Py_nb_xor, (void *)enum_xor},
    {Py_nb_or, (void *)enum_or},
    {Py_nb_int, (void *)enum_int},
    {Py_nb_index, (void *)enum_int},
    {Py_tp_base, (void *)&PyType_Type},
    {Py_tp_alloc, (void *)PyType_GenericAlloc},
    {Py_tp_new, (void *)SbkEnumTypeTpNew},
    {Py_tp_free, (void *)PyObject_GC_Del},
    {0, nullptr}
};
static PyType_Spec SbkEnumType_Type_spec = {
    "1:Shiboken.EnumType",
    0,    // filled in later
    sizeof(PyMemberDef),
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES,
    SbkEnumType_Type_slots,
};


PyTypeObject *SbkEnumType_TypeF(void)
{
    static PyTypeObject *type = nullptr;
    if (!type) {
        SbkEnumType_Type_spec.basicsize =
            PepHeapType_SIZE + sizeof(SbkEnumTypePrivate);
        type = reinterpret_cast<PyTypeObject *>(SbkType_FromSpec(&SbkEnumType_Type_spec));
    }
    return type;
}

void SbkEnumTypeDealloc(PyObject *pyObj)
{
    auto sbkType = reinterpret_cast<SbkEnumType *>(pyObj);

    PyObject_GC_UnTrack(pyObj);
#ifndef Py_LIMITED_API
    Py_TRASHCAN_SAFE_BEGIN(pyObj);
#endif
    if (PepType_SETP(sbkType)->converter) {
        Shiboken::Conversions::deleteConverter(PepType_SETP(sbkType)->converter);
    }
#ifndef Py_LIMITED_API
    Py_TRASHCAN_SAFE_END(pyObj);
#endif
    if (PepRuntime_38_flag) {
        // PYSIDE-939: Handling references correctly.
        // This was not needed before Python 3.8 (Python issue 35810)
        Py_DECREF(Py_TYPE(pyObj));
    }
}

PyObject *SbkEnumTypeTpNew(PyTypeObject *metatype, PyObject *args, PyObject *kwds)
{
    auto type_new = reinterpret_cast<newfunc>(PyType_GetSlot(&PyType_Type, Py_tp_new));
    auto newType = reinterpret_cast<SbkEnumType *>(type_new(metatype, args, kwds));
    if (!newType)
        return nullptr;
    return reinterpret_cast<PyObject *>(newType);
}

} // extern "C"

///////////////////////////////////////////////////////////////
//
// PYSIDE-15: Pickling Support for Qt Enum objects
//            This works very well and fixes the issue.
//
extern "C" {

static void init_enum();  // forward

static PyObject *enum_unpickler = nullptr;

// Pickling: reduce the Qt Enum object
static PyObject *enum___reduce__(PyObject *obj)
{
    init_enum();
    return Py_BuildValue("O(Ni)",
                         enum_unpickler,
                         Py_BuildValue("s", Py_TYPE(obj)->tp_name),
                         PyInt_AS_LONG(obj));
}

} // extern "C"

namespace Shiboken { namespace Enum {

// Unpickling: rebuild the Qt Enum object
PyObject *unpickleEnum(PyObject *enum_class_name, PyObject *value)
{
    Shiboken::AutoDecRef parts(PyObject_CallMethod(enum_class_name,
        const_cast<char *>("split"), const_cast<char *>("s"), "."));
    if (parts.isNull())
        return nullptr;
    PyObject *top_name = PyList_GetItem(parts, 0); // borrowed ref
    if (top_name == nullptr)
        return nullptr;
    PyObject *module = PyImport_GetModule(top_name);
    if (module == nullptr) {
        PyErr_Format(PyExc_ImportError, "could not import module %.200s",
            Shiboken::String::toCString(top_name));
        return nullptr;
    }
    Shiboken::AutoDecRef cur_thing(module);
    int len = PyList_Size(parts);
    for (int idx = 1; idx < len; ++idx) {
        PyObject *name = PyList_GetItem(parts, idx); // borrowed ref
        PyObject *thing = PyObject_GetAttr(cur_thing, name);
        if (thing == nullptr) {
            PyErr_Format(PyExc_ImportError, "could not import Qt Enum type %.200s",
                Shiboken::String::toCString(enum_class_name));
            return nullptr;
        }
        cur_thing.reset(thing);
    }
    PyObject *klass = cur_thing;
    return PyObject_CallFunctionObjArgs(klass, value, nullptr);
}

} // namespace Enum
} // namespace Shiboken

extern "C" {

// Initialization
static bool _init_enum()
{
    static PyObject *shiboken_name = Py_BuildValue("s", "shiboken6");
    if (shiboken_name == nullptr)
        return false;
    Shiboken::AutoDecRef shibo(PyImport_GetModule(shiboken_name));
    if (shibo.isNull())
        return false;
    Shiboken::AutoDecRef sub(PyObject_GetAttr(shibo, shiboken_name));
    PyObject *mod = sub.object();
    if (mod == nullptr) {
        // We are in the build dir and already in shiboken.
        PyErr_Clear();
        mod = shibo.object();
    }
    enum_unpickler = PyObject_GetAttrString(mod, "_unpickle_enum");
    if (enum_unpickler == nullptr)
        return false;
    return true;
}

static void init_enum()
{
    if (!(enum_unpickler || _init_enum()))
        Py_FatalError("could not load enum pickling helper function");
}

static PyMethodDef SbkEnumObject_Methods[] = {
    {const_cast<char *>("__reduce__"), reinterpret_cast<PyCFunction>(enum___reduce__),
        METH_NOARGS, nullptr},
    {nullptr, nullptr, 0, nullptr} // Sentinel
};

} // extern "C"

//
///////////////////////////////////////////////////////////////

namespace Shiboken {

class DeclaredEnumTypes
{
public:
    DeclaredEnumTypes(const DeclaredEnumTypes &) = delete;
    DeclaredEnumTypes(DeclaredEnumTypes &&) = delete;
    DeclaredEnumTypes &operator=(const DeclaredEnumTypes &) = delete;
    DeclaredEnumTypes &operator=(DeclaredEnumTypes &&) = delete;

    DeclaredEnumTypes();
    ~DeclaredEnumTypes();
    static DeclaredEnumTypes &instance();
    void addEnumType(PyTypeObject *type);

private:
    std::vector<PyTypeObject *> m_enumTypes;
};

namespace Enum {

bool check(PyObject *pyObj)
{
    return Py_TYPE(Py_TYPE(pyObj)) == SbkEnumType_TypeF();
}

PyObject *getEnumItemFromValue(PyTypeObject *enumType, long itemValue)
{
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    PyObject *values = PyDict_GetItem(enumType->tp_dict, Shiboken::PyName::values());

    while (PyDict_Next(values, &pos, &key, &value)) {
        auto *obj = reinterpret_cast<SbkEnumObject *>(value);
        if (obj->ob_value == itemValue) {
            Py_INCREF(value);
            return value;
        }
    }
    return nullptr;
}

static PyTypeObject *createEnum(const char *fullName, const char *cppName,
                                PyTypeObject *flagsType)
{
    PyTypeObject *enumType = newTypeWithName(fullName, cppName, flagsType);
    if (PyType_Ready(enumType) < 0) {
        Py_XDECREF(enumType);
        return nullptr;
    }
    return enumType;
}

PyTypeObject *createGlobalEnum(PyObject *module, const char *name, const char *fullName, const char *cppName, PyTypeObject *flagsType)
{
    PyTypeObject *enumType = createEnum(fullName, cppName, flagsType);
    if (enumType && PyModule_AddObject(module, name, reinterpret_cast<PyObject *>(enumType)) < 0) {
        Py_DECREF(enumType);
        return nullptr;
    }
    if (flagsType && PyModule_AddObject(module, PepType_GetNameStr(flagsType),
            reinterpret_cast<PyObject *>(flagsType)) < 0) {
        Py_DECREF(enumType);
        return nullptr;
    }
    return enumType;
}

PyTypeObject *createScopedEnum(SbkObjectType *scope, const char *name, const char *fullName, const char *cppName, PyTypeObject *flagsType)
{
    PyTypeObject *enumType = createEnum(fullName, cppName, flagsType);
    if (enumType && PyDict_SetItemString(reinterpret_cast<PyTypeObject *>(scope)->tp_dict, name,
            reinterpret_cast<PyObject *>(enumType)) < 0) {
        Py_DECREF(enumType);
        return nullptr;
    }
    if (flagsType && PyDict_SetItemString(reinterpret_cast<PyTypeObject *>(scope)->tp_dict,
            PepType_GetNameStr(flagsType),
            reinterpret_cast<PyObject *>(flagsType)) < 0) {
        Py_DECREF(enumType);
        return nullptr;
    }
    return enumType;
}

static PyObject *createEnumItem(PyTypeObject *enumType, const char *itemName, long itemValue)
{
    PyObject *enumItem = newItem(enumType, itemValue, itemName);
    if (PyDict_SetItemString(enumType->tp_dict, itemName, enumItem) < 0) {
        Py_DECREF(enumItem);
        return nullptr;
    }
    return enumItem;
}

bool createGlobalEnumItem(PyTypeObject *enumType, PyObject *module, const char *itemName, long itemValue)
{
    PyObject *enumItem = createEnumItem(enumType, itemName, itemValue);
    if (!enumItem)
        return false;
    int ok = PyModule_AddObject(module, itemName, enumItem);
    Py_DECREF(enumItem);
    return ok >= 0;
}

bool createScopedEnumItem(PyTypeObject *enumType, PyTypeObject *scope,
                          const char *itemName, long itemValue)
{
    PyObject *enumItem = createEnumItem(enumType, itemName, itemValue);
    if (!enumItem)
        return false;
    int ok = PyDict_SetItemString(reinterpret_cast<PyTypeObject *>(scope)->tp_dict, itemName, enumItem);
    Py_DECREF(enumItem);
    return ok >= 0;
}

bool createScopedEnumItem(PyTypeObject *enumType, SbkObjectType *scope, const char *itemName, long itemValue)
{
    return createScopedEnumItem(enumType, reinterpret_cast<PyTypeObject *>(scope), itemName, itemValue);
}

PyObject *
newItem(PyTypeObject *enumType, long itemValue, const char *itemName)
{
    bool newValue = true;
    SbkEnumObject *enumObj;
    if (!itemName) {
        enumObj = reinterpret_cast<SbkEnumObject *>(
                      getEnumItemFromValue(enumType, itemValue));
        if (enumObj)
            return reinterpret_cast<PyObject *>(enumObj);

        newValue = false;
    }

    enumObj = PyObject_New(SbkEnumObject, enumType);
    if (!enumObj)
        return nullptr;

    enumObj->ob_name = itemName ? PyBytes_FromString(itemName) : nullptr;
    enumObj->ob_value = itemValue;

    if (newValue) {
        auto dict = enumType->tp_dict;  // Note: 'values' is borrowed
        PyObject *values = PyDict_GetItemWithError(dict, Shiboken::PyName::values());
        if (values == nullptr) {
            if (PyErr_Occurred())
                return nullptr;
            Shiboken::AutoDecRef new_values(values = PyDict_New());
            if (values == nullptr)
                return nullptr;
            if (PyDict_SetItem(dict, Shiboken::PyName::values(), values) < 0)
                return nullptr;
        }
        PyDict_SetItemString(values, itemName, reinterpret_cast<PyObject *>(enumObj));
    }

    return reinterpret_cast<PyObject *>(enumObj);
}

static PyType_Slot SbkNewType_slots[] = {
    {Py_tp_repr, (void *)SbkEnumObject_repr},
    {Py_tp_str, (void *)SbkEnumObject_repr},
    {Py_tp_getset, (void *)SbkEnumGetSetList},
    {Py_tp_methods, (void *)SbkEnumObject_Methods},
    {Py_tp_new, (void *)SbkEnum_tp_new},
    {Py_nb_add, (void *)enum_add},
    {Py_nb_subtract, (void *)enum_subtract},
    {Py_nb_multiply, (void *)enum_multiply},
    {Py_nb_positive, (void *)enum_int},
    {Py_nb_bool, (void *)enum_bool},
    {Py_nb_and, (void *)enum_and},
    {Py_nb_xor, (void *)enum_xor},
    {Py_nb_or, (void *)enum_or},
    {Py_nb_int, (void *)enum_int},
    {Py_nb_index, (void *)enum_int},
    {Py_tp_richcompare, (void *)enum_richcompare},
    {Py_tp_hash, (void *)enum_hash},
    {Py_tp_dealloc, (void *)enum_object_dealloc},
    {0, nullptr}
};
static PyType_Spec SbkNewType_spec = {
    "missing Enum name", // to be inserted later
    sizeof(SbkEnumObject),
    0,
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_CHECKTYPES,
    SbkNewType_slots,
};

static void
copyNumberMethods(PyTypeObject *flagsType,
                  PyType_Slot number_slots[],
                  int *pidx)
{
    int idx = *pidx;
#define PUT_SLOT(name) \
    number_slots[idx].slot = (name);                                \
    number_slots[idx].pfunc = PyType_GetSlot(flagsType, (name));    \
    ++idx;

    PUT_SLOT(Py_nb_absolute);
    PUT_SLOT(Py_nb_add);
    PUT_SLOT(Py_nb_and);
    PUT_SLOT(Py_nb_bool);
    PUT_SLOT(Py_nb_divmod);
    PUT_SLOT(Py_nb_float);
    PUT_SLOT(Py_nb_floor_divide);
    PUT_SLOT(Py_nb_index);
    PUT_SLOT(Py_nb_inplace_add);
    PUT_SLOT(Py_nb_inplace_and);
    PUT_SLOT(Py_nb_inplace_floor_divide);
    PUT_SLOT(Py_nb_inplace_lshift);
    PUT_SLOT(Py_nb_inplace_multiply);
    PUT_SLOT(Py_nb_inplace_or);
    PUT_SLOT(Py_nb_inplace_power);
    PUT_SLOT(Py_nb_inplace_remainder);
    PUT_SLOT(Py_nb_inplace_rshift);
    PUT_SLOT(Py_nb_inplace_subtract);
    PUT_SLOT(Py_nb_inplace_true_divide);
    PUT_SLOT(Py_nb_inplace_xor);
    PUT_SLOT(Py_nb_int);
    PUT_SLOT(Py_nb_invert);
    PUT_SLOT(Py_nb_lshift);
    PUT_SLOT(Py_nb_multiply);
    PUT_SLOT(Py_nb_negative);
    PUT_SLOT(Py_nb_or);
    PUT_SLOT(Py_nb_positive);
    PUT_SLOT(Py_nb_power);
    PUT_SLOT(Py_nb_remainder);
    PUT_SLOT(Py_nb_rshift);
    PUT_SLOT(Py_nb_subtract);
    PUT_SLOT(Py_nb_true_divide);
    PUT_SLOT(Py_nb_xor);
#undef PUT_SLOT
    *pidx = idx;
}

PyTypeObject *
newTypeWithName(const char *name,
                const char *cppName,
                PyTypeObject *numbers_fromFlag)
{
    // Careful: SbkType_FromSpec does not allocate the string.
    PyType_Slot newslots[99] = {};  // enough but not too big for the stack
    PyType_Spec newspec;
    newspec.name = strdup(name);
    newspec.basicsize = SbkNewType_spec.basicsize;
    newspec.itemsize = SbkNewType_spec.itemsize;
    newspec.flags = SbkNewType_spec.flags;
    // we must append all the number methods, so rebuild everything:
    int idx = 0;
    while (SbkNewType_slots[idx].slot) {
        newslots[idx].slot = SbkNewType_slots[idx].slot;
        newslots[idx].pfunc = SbkNewType_slots[idx].pfunc;
        ++idx;
    }
    if (numbers_fromFlag)
        copyNumberMethods(numbers_fromFlag, newslots, &idx);
    newspec.slots = newslots;
    auto *type = reinterpret_cast<PyTypeObject *>(SbkType_FromSpec(&newspec));
    Py_TYPE(type) = SbkEnumType_TypeF();

    auto *enumType = reinterpret_cast<SbkEnumType *>(type);
    PepType_SETP(enumType)->cppName = cppName;
    PepType_SETP(enumType)->converterPtr = &PepType_SETP(enumType)->converter;
    DeclaredEnumTypes::instance().addEnumType(type);
    return type;
}

const char *getCppName(PyTypeObject *enumType)
{
    assert(Py_TYPE(enumType) == SbkEnumType_TypeF());
    return PepType_SETP(reinterpret_cast<SbkEnumType *>(enumType))->cppName;
}

long int getValue(PyObject *enumItem)
{
    assert(Shiboken::Enum::check(enumItem));
    return reinterpret_cast<SbkEnumObject *>(enumItem)->ob_value;
}

void setTypeConverter(PyTypeObject *enumType, SbkConverter *converter)
{
    //reinterpret_cast<SbkEnumType *>(enumType)->converter = converter;
    *PepType_SGTP(enumType)->converter = converter;
}

SbkConverter *getTypeConverter(PyTypeObject *enumType)
{
    //return reinterpret_cast<SbkEnumType *>(enumType)->converter;
    return *PepType_SGTP(enumType)->converter;
}

} // namespace Enum

DeclaredEnumTypes &DeclaredEnumTypes::instance()
{
    static DeclaredEnumTypes me;
    return me;
}

DeclaredEnumTypes::DeclaredEnumTypes() = default;

DeclaredEnumTypes::~DeclaredEnumTypes()
{
        /*
         * PYSIDE-595: This was "delete *it;" before introducing 'SbkType_FromSpec'.
         * XXX what should I do now?
         * Refcounts in tests are 30 or 0 at end.
         * When I add the default tp_dealloc, we get negative refcounts!
         * So right now I am doing nothing. Surely wrong but no crash.
         * See also the comment in function 'createGlobalEnumItem'.
         */
        // for (PyTypeObject *o : m_enumTypes)
        //     fprintf(stderr, "ttt %d %s\n", Py_REFCNT(o), o->tp_name);
    m_enumTypes.clear();
}

void DeclaredEnumTypes::addEnumType(PyTypeObject *type)
{
    m_enumTypes.push_back(type);
}

}
