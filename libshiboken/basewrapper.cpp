/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "basewrapper.h"
#include <cstddef>
#include <algorithm>
#include "autodecref.h"

namespace Shiboken
{

void removeParent(SbkBaseWrapper* child)
{
    if (!child->parentInfo->parent)
        return;

    ShiboChildrenList& oldBrothers = child->parentInfo->parent->parentInfo->children;
    oldBrothers.remove(child);
    child->parentInfo->parent = 0;
    Py_DECREF(child);
}

void setParent(PyObject* parent, PyObject* child)
{
    if (!child || child == Py_None || child == parent)
        return;

    bool parentIsNull = !parent || parent == Py_None;

    SbkBaseWrapper* parent_ = reinterpret_cast<SbkBaseWrapper*>(parent);
    SbkBaseWrapper* child_ = reinterpret_cast<SbkBaseWrapper*>(child);
    if (!child_->parentInfo)
        child_->parentInfo = new ShiboParentInfo;

    if (!parentIsNull) {
        if (!parent_->parentInfo)
            parent_->parentInfo = new ShiboParentInfo;
        // do not re-add a child
        ShiboChildrenList& children = parent_->parentInfo->children;
        if (std::find(children.begin(), children.end(), child_) != children.end())
            return;
    }

    bool hasAnotherParent = child_->parentInfo->parent && child_->parentInfo->parent != parent_;

    // check if we need to remove this child from the old parent
    if (parentIsNull || hasAnotherParent)
        removeParent(child_);

    // Add the child to the new parent
    if (!parentIsNull) {
        child_->parentInfo->parent = parent_;
        parent_->parentInfo->children.push_back(child_);
        Py_INCREF(child_);
    }
}

static void _destroyParentInfo(SbkBaseWrapper* obj, bool removeFromParent)
{
    if (removeFromParent && obj->parentInfo->parent)
        removeParent(obj);
    ShiboChildrenList::iterator it = obj->parentInfo->children.begin();
    for (; it != obj->parentInfo->children.end(); ++it) {
        SbkBaseWrapper*& child = *it;
        _destroyParentInfo(child, false);
        Py_DECREF(child);
    }
    delete obj->parentInfo;
    obj->parentInfo = 0;
}

void destroyParentInfo(SbkBaseWrapper* obj, bool removeFromParent)
{
    BindingManager::instance().invalidateWrapper(obj);
    _destroyParentInfo(obj, removeFromParent);
}

PyObject* SbkBaseWrapper_New(SbkBaseWrapperType* instanceType,
                             const void* cptr,
                             bool hasOwnership,
                             bool containsCppWrapper)
{
    SbkBaseWrapper* self = reinterpret_cast<SbkBaseWrapper*>(SbkBaseWrapper_TpNew(reinterpret_cast<PyTypeObject*>(instanceType), 0, 0));
    self->cptr = const_cast<void*>(cptr);
    self->hasOwnership = hasOwnership;
    self->containsCppWrapper = containsCppWrapper;
    self->validCppObject = 1;
    self->parentInfo = 0;
    BindingManager::instance().registerWrapper(self);
    return reinterpret_cast<PyObject*>(self);
}

PyObject* SbkBaseWrapper_TpNew(PyTypeObject* subtype, PyObject*, PyObject*)
{
    Shiboken::AutoDecRef emptyTuple(PyTuple_New(0));
    SbkBaseWrapper* self = reinterpret_cast<SbkBaseWrapper*>(PyBaseObject_Type.tp_new(subtype, emptyTuple, 0));
    self->cptr = 0;
    self->hasOwnership = 1;
    self->containsCppWrapper = 0;
    self->validCppObject = 0;
    self->parentInfo = 0;
    return reinterpret_cast<PyObject*>(self);
}

bool cppObjectIsInvalid(PyObject* wrapper)
{
    if (wrapper == Py_None || ((Shiboken::SbkBaseWrapper*)wrapper)->validCppObject)
        return false;
    PyErr_SetString(PyExc_RuntimeError, "internal C++ object already deleted.");
    return true;
}

void SbkBaseWrapper_Dealloc_PrivateDtor(PyObject* self)
{
    BindingManager::instance().releaseWrapper(self);
    Py_TYPE(reinterpret_cast<SbkBaseWrapper*>(self))->tp_free(self);
}

// Wrapper metatype and base type ----------------------------------------------------------

extern "C"
{

struct SbkBaseWrapperType_Type;

PyTypeObject SbkBaseWrapperType_Type = {
    PyObject_HEAD_INIT(0)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.BaseWrapperType",
    /*tp_basicsize*/        sizeof(SbkBaseWrapperType),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          0,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       0,
    /*tp_hash*/             0,
    /*tp_call*/             0,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
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
    /*tp_base*/             0,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             0,
    /*tp_alloc*/            0,
    /*tp_new*/              0,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0
};

SbkBaseWrapperType SbkBaseWrapper_Type = { { {
    PyObject_HEAD_INIT(&SbkBaseWrapperType_Type)
    /*ob_size*/             0,
    /*tp_name*/             "Shiboken.BaseWrapper",
    /*tp_basicsize*/        sizeof(SbkBaseWrapper),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          0,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       0,
    /*tp_hash*/             0,
    /*tp_call*/             0,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
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
    /*tp_base*/             0,
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             0,
    /*tp_alloc*/            0,
    /*tp_new*/              0,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0
}, },
/*mi_offsets*/          0,
/*mi_init*/             0,
/*mi_specialcast*/      0
};

PyAPI_FUNC(void) init_shiboken()
{
    static bool shibokenAlreadInitialised = false;
    if (shibokenAlreadInitialised)
        return;

    SbkBaseWrapperType_Type.tp_base = &PyType_Type;

    if (PyType_Ready(&SbkBaseWrapperType_Type) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.BaseWrapperType metatype.");

    if (PyType_Ready((PyTypeObject *)&SbkBaseWrapper_Type) < 0)
        Py_FatalError("[libshiboken] Failed to initialise Shiboken.BaseWrapper type.");

    shibokenAlreadInitialised = true;
}

} // extern "C"

} // namespace Shiboken



