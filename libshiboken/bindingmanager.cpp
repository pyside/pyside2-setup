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
#include "bindingmanager.h"
#include "google/dense_hash_map"

namespace Shiboken
{

typedef google::dense_hash_map<const void*, PyObject*> WrapperMap;

struct BindingManager::BindingManagerPrivate {
    WrapperMap wrapperMapper;
    void releaseWrapper(void* cptr);
};

void BindingManager::BindingManagerPrivate::releaseWrapper(void* cptr)
{
    WrapperMap::iterator iter = wrapperMapper.find(cptr);
    if (iter != wrapperMapper.end())
        wrapperMapper.erase(iter);
}

BindingManager::BindingManager()
{
    m_d = new BindingManager::BindingManagerPrivate;
    m_d->wrapperMapper.set_empty_key((WrapperMap::key_type)0);
    m_d->wrapperMapper.set_deleted_key((WrapperMap::key_type)1);
}

BindingManager::~BindingManager()
{
    delete m_d;
}

BindingManager& BindingManager::instance() {
    static BindingManager singleton;
    return singleton;
}

bool BindingManager::hasWrapper(const void* cptr)
{
    return m_d->wrapperMapper.count(cptr);
}

void BindingManager::assignWrapper(PyObject* wrapper, const void* cptr)
{
    WrapperMap::iterator iter = m_d->wrapperMapper.find(cptr);
    if (iter == m_d->wrapperMapper.end())
        m_d->wrapperMapper.insert(std::make_pair(cptr, wrapper));
    else
        iter->second = wrapper;
}

void BindingManager::releaseWrapper(PyObject* wrapper)
{
    void* cptr = PyBaseWrapper_cptr(wrapper);
    m_d->releaseWrapper(cptr);
    if (((ShiboTypeObject*) wrapper->ob_type)->mi_offsets) {
        int* offset = ((ShiboTypeObject*) wrapper->ob_type)->mi_offsets;
        while (*offset != -1) {
            if (*offset > 0)
                m_d->releaseWrapper((void*) ((size_t) cptr + (*offset)));
            offset++;
        }
    }
}

PyObject* BindingManager::retrieveWrapper(const void* cptr)
{
    WrapperMap::iterator iter = m_d->wrapperMapper.find(cptr);
    if (iter == m_d->wrapperMapper.end())
        return 0;
    return iter->second;
}

PyObject* BindingManager::getOverride(const void* cptr, const char* methodName)
{
    PyObject* wrapper = retrieveWrapper(cptr);
    if (!wrapper)
        return 0;

    PyObject* pyMethodName = PyString_FromString(methodName);
    PyObject* method = PyObject_GetAttr(wrapper, pyMethodName);

    if (method && PyMethod_Check(method) && reinterpret_cast<PyMethodObject*>(method)->im_self == wrapper) {
        PyObject* defaultMethod;
        PyObject* mro = wrapper->ob_type->tp_mro;

        // The first class in the mro (index 0) is the class being checked and it should not be tested.
        // The last class in the mro (size - 1) is the base Python object class which should not be tested also.
        for (int i = 1; i < PyTuple_GET_SIZE(mro) - 1; i++) {
            PyTypeObject* parent = reinterpret_cast<PyTypeObject*>(PyTuple_GET_ITEM(mro, i));
            if (parent->tp_dict) {
                defaultMethod = PyDict_GetItem(parent->tp_dict, pyMethodName);
                if (defaultMethod && reinterpret_cast<PyMethodObject*>(method)->im_func != defaultMethod) {
                    Py_DECREF(pyMethodName);
                    return method;
                }
            }
        }
    }

    Py_XDECREF(method);
    Py_DECREF(pyMethodName);
    return 0;
}

void BindingManager::invalidateWrapper(PyBaseWrapper* wrapper)
{
    if (!PyBaseWrapper_validCppObject(wrapper))
        return;
    PyBaseWrapper_setValidCppObject(wrapper, false);
    PyBaseWrapper_setOwnership(wrapper, false);
    // If it is a parent invalidate all children.
    if (PyBaseWrapper_hasParentInfo(wrapper)) {
        ShiboChildrenList::iterator it = wrapper->parentInfo->children.begin();
        for (; it != wrapper->parentInfo->children.end(); ++it)
            invalidateWrapper(*it);
    }
    releaseWrapper(reinterpret_cast<PyObject*>(wrapper));
}

void BindingManager::invalidateWrapper(const void* cptr)
{
    WrapperMap::iterator iter = m_d->wrapperMapper.find(cptr);
    if (iter != m_d->wrapperMapper.end())
        invalidateWrapper(iter->second);
}

void BindingManager::transferOwnershipToCpp(PyBaseWrapper* wrapper)
{
    if (wrapper->parentInfo)
        Shiboken::removeParent(wrapper);

    if (PyBaseWrapper_containsCppWrapper(wrapper))
        PyBaseWrapper_setOwnership(wrapper, false);
    else
        invalidateWrapper(wrapper);
}

} // namespace Shiboken

