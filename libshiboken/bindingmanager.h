/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef BINDINGMANAGER_H
#define BINDINGMANAGER_H

#include <Python.h>
#include "shibokenmacros.h"

namespace Shiboken
{

struct SbkBaseWrapper;
struct SbkBaseWrapperType;

class LIBSHIBOKEN_API BindingManager
{
public:
    static BindingManager& instance();

    bool hasWrapper(const void *cptr);

    void registerWrapper( Shiboken::SbkBaseWrapper* pyobj, void* cptr);
    void releaseWrapper(PyObject* wrapper);
    PyObject* retrieveWrapper(const void* cptr);
    PyObject* getOverride(const void* cptr, const char* methodName);

    /// Invalidate the Python wrapper and removes the relations from C++ pointers to the Python wrapper.
    void invalidateWrapper(SbkBaseWrapper* wrapper);
    /// Convenience method to call invalidateWrapper with a properly cast SbkBaseWrapper.
    inline void invalidateWrapper(PyObject* wrapper)
    {
        invalidateWrapper(reinterpret_cast<SbkBaseWrapper*>(wrapper));
    }
    /// Convenience method to invalidate the Python wrapper for a C++ wrapped object. Do nothing if C++ pointer has no Python wrapper.
    void invalidateWrapper(const void* cptr);

    /// Transfers the ownership of a Python wrapper to C++.
    void transferOwnershipToCpp(SbkBaseWrapper* wrapper);
    /// Convenience method to call transferOwnershipToCpp with a properly cast SbkBaseWrapper.
    inline void transferOwnershipToCpp(PyObject* wrapper)
    {
        transferOwnershipToCpp(reinterpret_cast<SbkBaseWrapper*>(wrapper));
    }

    void addClassInheritance(SbkBaseWrapperType* parent, SbkBaseWrapperType* child);
    SbkBaseWrapperType* resolveType(void* cptr, SbkBaseWrapperType* type);

    /// Called by wrapper destructor
    void destroyWrapper(const void* cptr);
    void destroyWrapper(SbkBaseWrapper* wrapper);
private:
    ~BindingManager();
    // disable copy
    BindingManager();
    BindingManager(const BindingManager&);
    BindingManager& operator=(const BindingManager&);

    struct BindingManagerPrivate;
    BindingManagerPrivate* m_d;
};

} // namespace Shiboken

#endif // BINDINGMANAGER_H

