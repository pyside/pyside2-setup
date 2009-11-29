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

namespace Shiboken
{

void removeParent(PyBaseWrapper* child)
{
    if (child->parentInfo->parent) {
        ShiboChildrenList& oldBrothers = child->parentInfo->parent->parentInfo->children;
        oldBrothers.remove(child);
        child->parentInfo->parent = 0;
        Py_DECREF(child);
    }
}

void setParent(PyObject* parent, PyObject* child)
{
    if (!child || child == Py_None || child == parent)
        return;

    bool parentIsNull = !parent || parent == Py_None;

    PyBaseWrapper* parent_ = reinterpret_cast<PyBaseWrapper*>(parent);
    PyBaseWrapper* child_ = reinterpret_cast<PyBaseWrapper*>(child);
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

static void _destroyParentInfo(PyBaseWrapper* obj, bool removeFromParent)
{
    if (removeFromParent && obj->parentInfo->parent)
        removeParent(obj);
    ShiboChildrenList::iterator it = obj->parentInfo->children.begin();
    for (; it != obj->parentInfo->children.end(); ++it) {
        PyBaseWrapper*& child = *it;
        _destroyParentInfo(child, false);
        Py_DECREF(child);
    }
    delete obj->parentInfo;
    obj->parentInfo = 0;
}

void destroyParentInfo(PyBaseWrapper* obj, bool removeFromParent)
{
    BindingManager::instance().invalidateWrapper(obj);
    _destroyParentInfo(obj, removeFromParent);
}

PyObject* PyBaseWrapper_New(PyTypeObject* instanceType,
                            ShiboTypeObject* baseWrapperType,
                            const void* cptr,
                            unsigned int hasOwnership,
                            unsigned int containsCppWrapper)
{
    if (!cptr)
        return 0;

    ShiboTypeObject* const& instanceType_ = reinterpret_cast<ShiboTypeObject*>(instanceType);
    PyBaseWrapper* self = (PyBaseWrapper*)instanceType_->pytype.tp_alloc((PyTypeObject*) instanceType, 0);

    self->baseWrapperType = baseWrapperType;
    self->cptr = const_cast<void*>(cptr);
    self->hasOwnership = hasOwnership;
    self->containsCppWrapper = containsCppWrapper;
    self->validCppObject = 1;
    self->parentInfo = 0;

    if (instanceType_->mi_init && !instanceType_->mi_offsets)
        instanceType_->mi_offsets = instanceType_->mi_init(cptr);
    BindingManager::instance().assignWrapper(reinterpret_cast<PyObject*>(self), cptr);
    if (instanceType_->mi_offsets) {
        int* offset = instanceType_->mi_offsets;
        while (*offset != -1) {
            if (*offset > 0) {
                BindingManager::instance().assignWrapper(reinterpret_cast<PyObject*>(self),
                                                         reinterpret_cast<void*>((std::size_t) cptr + (*offset)));
            }
            offset++;
        }
    }
    return reinterpret_cast<PyObject*>(self);
}

bool cppObjectIsInvalid(PyObject* wrapper)
{
    if (((Shiboken::PyBaseWrapper*)wrapper)->validCppObject)
        return false;
    PyErr_SetString(PyExc_RuntimeError, "internal C++ object already deleted.");
    return true;
}

void PyBaseWrapper_Dealloc_PrivateDtor(PyObject* self)
{
    BindingManager::instance().releaseWrapper(self);
    Py_TYPE(((PyBaseWrapper*)self))->tp_free((PyObject*)self);
}

} // namespace Shiboken

