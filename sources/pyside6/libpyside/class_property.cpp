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

#include "pyside.h"
#include "pysidestaticstrings.h"
#include "feature_select.h"
#include "class_property.h"

#include <shiboken.h>
#include <sbkstaticstrings.h>

extern "C" {

/*
 * A `classproperty` is the same as a `property` but the `__get__()` and `__set__()`
 * methods are modified to always use the object class instead of a concrete instance.
 *
 * Note: A "static property" as it is often called does not exist per se.
 * Static methods do not receive anything when created. Static methods which
 * should participate in a property must be turned into class methods, before.
 * See function `createProperty` in `feature_select.cpp`.
 */

// `class_property.__get__()`: Always pass the class instead of the instance.
static PyObject *PyClassProperty_get(PyObject *self, PyObject * /*ob*/, PyObject *cls)
{
    return PyProperty_Type.tp_descr_get(self, cls, cls);
}

// `class_property.__set__()`: Just like the above `__get__()`.
static int PyClassProperty_set(PyObject *self, PyObject *obj, PyObject *value)
{
    PyObject *cls = PyType_Check(obj) ? obj : reinterpret_cast<PyObject *>(Py_TYPE(obj));
    return PyProperty_Type.tp_descr_set(self, cls, value);
}

// The property `__doc__` default does not work for class properties
// because PyProperty_Type.tp_init thinks this is a subclass which needs PyObject_SetAttr.
// We call `__init__` while pretending to be a PyProperty_Type instance.
static int PyClassProperty_init(PyObject *self, PyObject *args, PyObject *kwargs)
{
    auto hold = Py_TYPE(self);
    Py_TYPE(self) = &PyProperty_Type;
    auto ret =  PyProperty_Type.tp_init(self, args, kwargs);
    Py_TYPE(self) = hold;
    return ret;
}

static PyType_Slot PyClassProperty_slots[] = {
    {Py_tp_getset,      nullptr},    // will be set below
    {Py_tp_base,        reinterpret_cast<void *>(&PyProperty_Type)},
    {Py_tp_descr_get,   reinterpret_cast<void *>(PyClassProperty_get)},
    {Py_tp_descr_set,   reinterpret_cast<void *>(PyClassProperty_set)},
    {Py_tp_init,        reinterpret_cast<void *>(PyClassProperty_init)},
    {0, 0}
};

static PyType_Spec PyClassProperty_spec = {
    "PySide6.PyClassProperty",
    sizeof(propertyobject),
    0,
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    PyClassProperty_slots,
};

PyTypeObject *PyClassPropertyTypeF()
{
    static PyTypeObject *type = nullptr;
    if (type == nullptr) {
        // Provide the same `tp_getset`, which is not inherited.
        PyClassProperty_slots[0].pfunc = PyProperty_Type.tp_getset;
        type = reinterpret_cast<PyTypeObject *>(
            PyType_FromSpec(&PyClassProperty_spec));
    }
    return type;
}

/*
 * Types with class properties need to handle `Type.class_prop = x` in a specific way.
 * By default, Python replaces the `class_property` itself, but for wrapped C++ types
 * we need to call `class_property.__set__()` in order to propagate the new value to
 * the underlying C++ data structure.
 */
static int SbkObjectType_meta_setattro(PyObject *obj, PyObject *name, PyObject *value)
{
    // Use `_PepType_Lookup()` instead of `PyObject_GetAttr()` in order to get the raw
    // descriptor (`property`) instead of calling `tp_descr_get` (`property.__get__()`).
    auto type = reinterpret_cast<PyTypeObject *>(obj);
    PyObject *descr = _PepType_Lookup(type, name);

    // The following assignment combinations are possible:
    //   1. `Type.class_prop = value`              --> descr_set: `Type.class_prop.__set__(value)`
    //   2. `Type.class_prop = other_class_prop`   --> setattro:  replace existing `class_prop`
    //   3. `Type.regular_attribute = value`       --> setattro:  regular attribute assignment
    const auto class_prop = reinterpret_cast<PyObject *>(PyClassPropertyTypeF());
    const auto call_descr_set = descr && PyObject_IsInstance(descr, class_prop)
                                && !PyObject_IsInstance(value, class_prop);
    if (call_descr_set) {
        // Call `class_property.__set__()` instead of replacing the `class_property`.
        return Py_TYPE(descr)->tp_descr_set(descr, obj, value);
    } else {
        // Replace existing attribute.
        return PyType_Type.tp_setattro(obj, name, value);
    }
}

} // extern "C"

/*
 * These functions are added to the SbkObjectType_TypeF() dynamically.
 */
namespace PySide { namespace ClassProperty {

void init()
{
    PyTypeObject *type = SbkObjectType_TypeF();
    type->tp_setattro = SbkObjectType_meta_setattro;
    Py_TYPE(PyClassPropertyTypeF()) = type;
}

} // namespace ClassProperty
} // namespace PySide
