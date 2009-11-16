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

namespace Shiboken
{

PyObject*
PyBaseWrapper_New(PyTypeObject* instanceType, ShiboTypeObject* baseWrapperType, const void* cptr, uint hasOwnership)
{
    if (!cptr)
        return 0;

    PyObject* self = ((ShiboTypeObject*) instanceType)->pytype.tp_alloc((PyTypeObject*) instanceType, 0);
    ((PyBaseWrapper*)self)->baseWrapperType = baseWrapperType;
    ((PyBaseWrapper*)self)->cptr = const_cast<void*>(cptr);
    ((PyBaseWrapper*)self)->hasOwnership = hasOwnership;
    ((PyBaseWrapper*)self)->validCppObject = 1;
    if (((ShiboTypeObject*) instanceType)->mi_init && !((ShiboTypeObject*) instanceType)->mi_offsets)
        ((ShiboTypeObject*) instanceType)->mi_offsets = ((ShiboTypeObject*) instanceType)->mi_init(cptr);
    BindingManager::instance().assignWrapper(self, cptr);
    if (((ShiboTypeObject*) instanceType)->mi_offsets) {
        int* offset = ((ShiboTypeObject*) instanceType)->mi_offsets;
        while (*offset != -1) {
            if (*offset > 0)
                BindingManager::instance().assignWrapper(self, (void*) ((size_t) cptr + (*offset)));
            offset++;
        }
    }
    return self;
}

void
PyBaseWrapper_Dealloc_PrivateDtor(PyObject* self)
{
    BindingManager::instance().releaseWrapper(self);
    Py_TYPE(((PyBaseWrapper*)self))->tp_free((PyObject*)self);
}

} // namespace Shiboken
