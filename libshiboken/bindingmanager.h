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
#include <set>
#include "shibokenmacros.h"

struct SbkObject;
struct SbkBaseType;

namespace Shiboken
{

class LIBSHIBOKEN_API BindingManager
{
public:
    static BindingManager& instance();

    bool hasWrapper(const void *cptr);

    void registerWrapper(SbkObject* pyObj, void* cptr);
    void releaseWrapper(SbkObject* wrapper);

    SbkObject* retrieveWrapper(const void* cptr);
    PyObject* getOverride(const void* cptr, const char* methodName);

    void addClassInheritance(SbkBaseType* parent, SbkBaseType* child);
    SbkBaseType* resolveType(void* cptr, SbkBaseType* type);

    std::set<SbkObject*> getAllPyObjects();
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

