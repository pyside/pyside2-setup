/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
#include "sbkconverter.h"
#include "basewrapper.h"
#include "sbkdbg.h"
#include "autodecref.h"
#include "sbkpython.h"

#include <string.h>
#include <cstring>
#include <list>

#define SBK_ENUM(ENUM) reinterpret_cast<SbkEnumObject*>(ENUM)

extern "C"
{

struct SbkEnumType
{
    PyHeapTypeObject super;
    SbkConverter** converterPtr;
    SbkConverter* converter;
    const char* cppName;
};

struct SbkEnumObject
{
    PyObject_HEAD
    long ob_value;
    PyObject* ob_name;
};

static PyObject* SbkEnumObject_repr(PyObject* self)
{
    const SbkEnumObject *enumObj = reinterpret_cast<SbkEnumObject *>(self);
    if (enumObj->ob_name)
        return Shiboken::String::fromFormat("%s.%s", self->ob_type->tp_name, PyBytes_AS_STRING(enumObj->ob_name));
    else
        return Shiboken::String::fromFormat("%s(%ld)", self->ob_type->tp_name, enumObj->ob_value);
}

static int SbkEnumObject_print(PyObject* self, FILE* fp, int)
{
    Py_BEGIN_ALLOW_THREADS
    const SbkEnumObject *enumObj = reinterpret_cast<SbkEnumObject *>(self);
    if (enumObj->ob_name)
        fprintf(fp, "%s.%s", self->ob_type->tp_name, PyBytes_AS_STRING(enumObj->ob_name));
    else
        fprintf(fp, "%s(%ld)", self->ob_type->tp_name, enumObj->ob_value);
    Py_END_ALLOW_THREADS
    return 0;
}

static PyObject* SbkEnumObject_name(PyObject* self, void*)
{
    SbkEnumObject *enum_self = reinterpret_cast<SbkEnumObject *>(self);

    if (enum_self->ob_name == NULL)
        Py_RETURN_NONE;

    Py_INCREF(enum_self->ob_name);
    return enum_self->ob_name;
}

static PyObject* SbkEnum_tp_new(PyTypeObject *type, PyObject *args, PyObject *)
{
    long itemValue = 0;
    if (!PyArg_ParseTuple(args, "|l:__new__", &itemValue))
        return 0;

    SbkEnumObject* self = PyObject_New(SbkEnumObject, type);
    if (!self)
        return 0;
    self->ob_value = itemValue;
    PyObject* item = Shiboken::Enum::getEnumItemFromValue(type, itemValue);
    if (item) {
        self->ob_name = SbkEnumObject_name(item, 0);
        Py_XDECREF(item);
    } else {
        self->ob_name = 0;
    }
    return reinterpret_cast<PyObject*>(self);
}

/* Notes:
 *   On Py3k land we use long type when using integer numbers. However, on older
 *   versions of Python (version 2) we need to convert it to int type,
 *   respectively.
 *
 *   Thus calling PyInt_FromLong() will result in calling PyLong_FromLong in
 *   Py3k.
 */
static PyObject* enum_int(PyObject* v)
{
    return PyInt_FromLong(SBK_ENUM(v)->ob_value);
}

static long getNumberValue(PyObject* v)
{
    PyObject* number = PyNumber_Long(v);
    long result = PyLong_AsLong(number);
    Py_XDECREF(number);
    return result;
}

static PyObject* enum_and(PyObject* self, PyObject* b)
{
    if (!PyNumber_Check(b)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(b);
    return PyInt_FromLong(valA & valB);
}

static PyObject* enum_or(PyObject* self, PyObject* b)
{
    if (!PyNumber_Check(b)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(b);
    return PyInt_FromLong(valA | valB);
}

static PyObject* enum_xor(PyObject* self, PyObject* b)
{
    if (!PyNumber_Check(b)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(b);
    return PyInt_FromLong(valA ^ valB);
}

static int enum_bool(PyObject* v)
{
    return (SBK_ENUM(v)->ob_value > 0);
}

static PyObject* enum_add(PyObject* self, PyObject* v)
{
    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(v);
    return PyInt_FromLong(valA + valB);
}

static PyObject* enum_subtract(PyObject* self, PyObject* v)
{
    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(v);
    return PyInt_FromLong(valA - valB);
}

static PyObject* enum_multiply(PyObject* self, PyObject* v)
{
    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(v);
    return PyInt_FromLong(valA * valB);
}

#ifndef IS_PY3K
static PyObject* enum_divide(PyObject* self, PyObject* v)
{
    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(v);
    return PyLong_FromLong(valA / valB);
}
#endif

static PyObject* enum_richcompare(PyObject* self, PyObject* other, int op)
{
    int result = 0;
    if (!PyNumber_Check(other)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    long valA = SBK_ENUM(self)->ob_value;
    long valB = getNumberValue(other);

    switch (op) {
    case Py_EQ:
        result = (valA == valB);
        break;
    case Py_NE:
        result = (valA != valB);
        break;
    case Py_LE:
        result = (valA <= valB);
        break;
    case Py_GE:
        result = (valA >= valB);
        break;
    case Py_LT:
        result = (valA < valB);
        break;
    case Py_GT:
        result = (valA > valB);
        break;
    default:
        PyErr_BadArgument();
        return NULL;
    }
    if (result)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

static Py_hash_t enum_hash(PyObject* pyObj)
{
    Py_hash_t val = reinterpret_cast<SbkEnumObject*>(pyObj)->ob_value;
    if (val == -1)
        val = -2;
    return val;
}

static PyGetSetDef SbkEnumGetSetList[] = {
    {const_cast<char*>("name"), &SbkEnumObject_name, 0, 0, 0},
    {0, 0, 0, 0, 0} // Sentinel
};

static PyNumberMethods enum_as_number = {
     /* nb_add */                   enum_add,
     /* nb_subtract */              enum_subtract,
     /* nb_multiply */              enum_multiply,
#ifndef IS_PY3K
     /* nb_divide */                enum_divide,
#endif
     /* nb_remainder */             0,
     /* nb_divmod */                0,
     /* nb_power */                 0,
     /* nb_negative */              0,
     /* nb_positive */              enum_int,
     /* nb_absolute */              0,
     /* nb_bool/nb_nonzero */       enum_bool,
     /* nb_invert */                0,
     /* nb_lshift */                0,
     /* nb_rshift */                0,
     /* nb_and */                   enum_and,
     /* nb_xor */                   enum_xor,
     /* nb_or */                    enum_or,
#ifndef IS_PY3K
     /* nb_coerce */                0,
#endif
     /* nb_int */                   enum_int,
#ifdef IS_PY3K
     /* nb_reserved */              0,
     /* nb_float */                 0,
#else
     /* nb_long */                  enum_int,
     /* nb_float */                 0,
     /* nb_oct */                   0,
     /* nb_hex */                   0,
#endif

     /* nb_inplace_add */           0,
     /* nb_inplace_subtract */      0,
     /* nb_inplace_multiply */      0,
#ifndef IS_PY3K
     /* nb_inplace_div */           0,
#endif
     /* nb_inplace_remainder */     0,
     /* nb_inplace_power */         0,
     /* nb_inplace_lshift */        0,
     /* nb_inplace_rshift */        0,
     /* nb_inplace_and */           0,
     /* nb_inplace_xor */           0,
     /* nb_inplace_or */            0,

     /* nb_floor_divide */          0,
     /* nb_true_divide */           0,
     /* nb_inplace_floor_divide */  0,
     /* nb_inplace_true_divide */   0,

     /* nb_index */                 enum_int
};

static void SbkEnumTypeDealloc(PyObject* pyObj);
static PyObject* SbkEnumTypeTpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds);

static PyTypeObject SbkEnumType_Type = {
    PyVarObject_HEAD_INIT(0, 0)
    /*tp_name*/             "Shiboken.EnumType",
    /*tp_basicsize*/        sizeof(SbkEnumType),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          SbkEnumTypeDealloc,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        &enum_as_number,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       0,
    /*tp_hash*/             0,
    /*tp_call*/             0,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT/*|Py_TPFLAGS_HEAPTYPE*/|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES,
    /*tp_doc*/              0,
    /*tp_traverse*/         0,
    /*tp_clear*/            0,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          0,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             &PyType_Type,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             0,
    /*tp_alloc*/            PyType_GenericAlloc,
    /*tp_new*/              SbkEnumTypeTpNew,
    /*tp_free*/             PyObject_GC_Del,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0,
    /*tp_del*/              0,
    /*tp_version_tag*/      0
};

PyTypeObject *SbkEnumType_TypeP = &SbkEnumType_Type;

void SbkEnumTypeDealloc(PyObject* pyObj)
{
    SbkEnumType* sbkType = reinterpret_cast<SbkEnumType*>(pyObj);

    PyObject_GC_UnTrack(pyObj);
    Py_TRASHCAN_SAFE_BEGIN(pyObj);
    if (sbkType->converter) {
        Shiboken::Conversions::deleteConverter(sbkType->converter);
    }
    Py_TRASHCAN_SAFE_END(pyObj);
}

PyObject* SbkEnumTypeTpNew(PyTypeObject* metatype, PyObject* args, PyObject* kwds)
{
    SbkEnumType* newType = reinterpret_cast<SbkEnumType*>(PyType_Type.tp_new(metatype, args, kwds));
    if (!newType)
        return 0;
    return reinterpret_cast<PyObject*>(newType);
}

} // extern "C"

namespace Shiboken {

class DeclaredEnumTypes
{
public:
    DeclaredEnumTypes();
    ~DeclaredEnumTypes();
    static DeclaredEnumTypes& instance();
    void addEnumType(PyTypeObject* type);

private:
    DeclaredEnumTypes(const DeclaredEnumTypes&);
    DeclaredEnumTypes& operator=(const DeclaredEnumTypes&);
    std::list<PyTypeObject*> m_enumTypes;
};

namespace Enum {

bool check(PyObject* pyObj)
{
    return Py_TYPE(pyObj->ob_type) == SbkEnumType_TypeP;
}

PyObject* getEnumItemFromValue(PyTypeObject* enumType, long itemValue)
{
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    PyObject* values = PyDict_GetItemString(enumType->tp_dict, const_cast<char*>("values"));

    while (PyDict_Next(values, &pos, &key, &value)) {
        SbkEnumObject *obj = reinterpret_cast<SbkEnumObject *>(value);
        if (obj->ob_value == itemValue) {
            Py_INCREF(obj);
            return value;
        }
    }
    return 0;
}

static PyTypeObject* createEnum(const char* fullName, const char* cppName, const char* shortName, PyTypeObject* flagsType)
{
    PyTypeObject* enumType = newTypeWithName(fullName, cppName);
    if (flagsType)
        enumType->tp_as_number = flagsType->tp_as_number;
    if (PyType_Ready(enumType) < 0)
        return 0;
    return enumType;
}

PyTypeObject* createGlobalEnum(PyObject* module, const char* name, const char* fullName, const char* cppName, PyTypeObject* flagsType)
{
    PyTypeObject* enumType = createEnum(fullName, cppName, name, flagsType);
    if (enumType && PyModule_AddObject(module, name, reinterpret_cast<PyObject *>(enumType)) < 0)
        return 0;
    if (flagsType && PyModule_AddObject(module, flagsType->tp_name, reinterpret_cast<PyObject *>(flagsType)) < 0)
        return 0;
    return enumType;
}

PyTypeObject* createScopedEnum(SbkObjectType* scope, const char* name, const char* fullName, const char* cppName, PyTypeObject* flagsType)
{
    PyTypeObject* enumType = createEnum(fullName, cppName, name, flagsType);
    if (enumType && PyDict_SetItemString(scope->super.ht_type.tp_dict, name, reinterpret_cast<PyObject *>(enumType)) < 0)
       return 0;
    if (flagsType && PyDict_SetItemString(scope->super.ht_type.tp_dict, flagsType->tp_name, reinterpret_cast<PyObject *>(flagsType)) < 0)
       return 0;
    return enumType;
}

static PyObject* createEnumItem(PyTypeObject* enumType, const char* itemName, long itemValue)
{
    PyObject* enumItem = newItem(enumType, itemValue, itemName);
    if (PyDict_SetItemString(enumType->tp_dict, itemName, enumItem) < 0)
        return 0;
    Py_DECREF(enumItem);
    return enumItem;
}

bool createGlobalEnumItem(PyTypeObject* enumType, PyObject* module, const char* itemName, long itemValue)
{
    PyObject* enumItem = createEnumItem(enumType, itemName, itemValue);
    if (enumItem) {
        if (PyModule_AddObject(module, itemName, enumItem) < 0)
            return false;
        // @TODO This Py_DECREF causes crashes on exit with a debug Python interpreter, essentially
        // causing a use-after-free in the GC. This is now commented out to cause a memory leak
        // instead of a crash. Proper memory management of Enum types and items should be
        // implemented. See PYSIDE-488. This will require proper allocation and deallocation of
        // the underlying Enum PyHeapType, which is currently just deallocated at application exit.
        // Py_DECREF(enumItem);
        return true;
    }
    return false;
}

bool createScopedEnumItem(PyTypeObject *enumType, PyTypeObject *scope,
                          const char *itemName, long itemValue)
{
    if (PyObject *enumItem = createEnumItem(enumType, itemName, itemValue)) {
        if (PyDict_SetItemString(scope->tp_dict, itemName, enumItem) < 0)
            return false;
        Py_DECREF(enumItem);
        return true;
    }
    return false;
}

bool createScopedEnumItem(PyTypeObject* enumType, SbkObjectType* scope, const char* itemName, long itemValue)
{
    return createScopedEnumItem(enumType, &scope->super.ht_type, itemName, itemValue);
}

PyObject* newItem(PyTypeObject* enumType, long itemValue, const char* itemName)
{
    bool newValue = true;
    SbkEnumObject* enumObj;
    if (!itemName) {
        enumObj = reinterpret_cast<SbkEnumObject*>(getEnumItemFromValue(enumType, itemValue));
        if (enumObj)
            return reinterpret_cast<PyObject*>(enumObj);

        newValue = false;
    }

    enumObj = PyObject_New(SbkEnumObject, enumType);
    if (!enumObj)
        return 0;

    enumObj->ob_name = itemName ? PyBytes_FromString(itemName) : 0;
    enumObj->ob_value = itemValue;

    if (newValue) {
        PyObject* values = PyDict_GetItemString(enumType->tp_dict, const_cast<char*>("values"));
        if (!values) {
            values = PyDict_New();
            PyDict_SetItemString(enumType->tp_dict, const_cast<char*>("values"), values);
            Py_DECREF(values); // ^ values still alive, because setitemstring incref it
        }
        PyDict_SetItemString(values, itemName, reinterpret_cast<PyObject*>(enumObj));
    }

    return reinterpret_cast<PyObject*>(enumObj);
}

PyTypeObject* newType(const char* name)
{
    return newTypeWithName(name, "");
}

PyTypeObject* newTypeWithName(const char* name, const char* cppName)
{
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(new SbkEnumType);
    ::memset(type, 0, sizeof(SbkEnumType));
    Py_TYPE(type) = SbkEnumType_TypeP;
    type->tp_basicsize = sizeof(SbkEnumObject);
    type->tp_print = &SbkEnumObject_print;
    type->tp_repr = &SbkEnumObject_repr;
    type->tp_str = &SbkEnumObject_repr;
    type->tp_flags = Py_TPFLAGS_DEFAULT/*|Py_TPFLAGS_HEAPTYPE*/|Py_TPFLAGS_CHECKTYPES;
    type->tp_name = name;
    type->tp_getset = SbkEnumGetSetList;
    type->tp_new = SbkEnum_tp_new;
    type->tp_as_number = &enum_as_number;
    type->tp_richcompare = &enum_richcompare;
    type->tp_hash = &enum_hash;

    SbkEnumType* enumType = reinterpret_cast<SbkEnumType*>(type);
    enumType->cppName = cppName;
    enumType->converterPtr = &enumType->converter;
    DeclaredEnumTypes::instance().addEnumType(type);
    return type;
}

const char* getCppName(PyTypeObject* enumType)
{
    assert(Py_TYPE(enumType) == SbkEnumType_TypeP);
    return reinterpret_cast<SbkEnumType*>(enumType)->cppName;;
}

long int getValue(PyObject* enumItem)
{
    assert(Shiboken::Enum::check(enumItem));
    return reinterpret_cast<SbkEnumObject*>(enumItem)->ob_value;
}

void setTypeConverter(PyTypeObject* enumType, SbkConverter* converter)
{
    //reinterpret_cast<SbkEnumType*>(enumType)->converter = converter;
    SBK_CONVERTER(enumType) = converter;
}

SbkConverter* getTypeConverter(PyTypeObject* enumType)
{
    //return reinterpret_cast<SbkEnumType*>(enumType)->converter;
    return SBK_CONVERTER(enumType);
}

} // namespace Enum

DeclaredEnumTypes& DeclaredEnumTypes::instance()
{
    static DeclaredEnumTypes me;
    return me;
}

DeclaredEnumTypes::DeclaredEnumTypes()
{
}

DeclaredEnumTypes::~DeclaredEnumTypes()
{
    std::list<PyTypeObject*>::const_iterator it = m_enumTypes.begin();
    for (; it != m_enumTypes.end(); ++it)
        delete *it;
    m_enumTypes.clear();
}

void DeclaredEnumTypes::addEnumType(PyTypeObject* type)
{
    m_enumTypes.push_back(type);
}

}
