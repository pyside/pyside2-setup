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

#ifndef BINDINGMANAGER_H
#define BINDINGMANAGER_H

#include <Python.h>
#include "shibokenmacros.h"

namespace Shiboken
{

struct PyBaseWrapper;

class LIBSHIBOKEN_API BindingManager
{
public:
    static BindingManager& instance();

    bool hasWrapper(const void *cptr);
    void assignWrapper(PyObject* wrapper, const void* cptr);
    void releaseWrapper(PyObject* wrapper);
    PyObject* retrieveWrapper(const void* cptr);
    PyObject* getOverride(const void* cptr, const char* methodName);

    /// Invalidate the Python wrapper and removes the relations from C++ pointers to the Python wrapper.
    void invalidateWrapper(PyBaseWrapper* wrapper);
    /// Convenience method to call invalidateWrapper with a properly cast PyBaseWrapper.
    inline void invalidateWrapper(PyObject* wrapper)
    {
        invalidateWrapper(reinterpret_cast<PyBaseWrapper*>(wrapper));
    }
    /// Convenience method to invalidate the Python wrapper for a C++ wrapped object. Do nothing if C++ pointer has no Python wrapper.
    void invalidateWrapper(const void* cptr);

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
