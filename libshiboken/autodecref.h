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

#ifndef AUTODECREF_H
#define AUTODECREF_H

#include <Python.h>
#include "shibokenmacros.h"

namespace Shiboken
{

/**
 *  AutoDecRef holds a PyObject pointer and decrement its reference counter when destroyed.
 */
class LIBSHIBOKEN_API AutoDecRef
{
public:
    /**
     * AutoDecRef constructor.
     * /param pyobj A borrowed reference to a Python object
     */
    AutoDecRef(PyObject* pyobj) : m_pyobj(pyobj) {}
    ~AutoDecRef() {
        Py_XDECREF(m_pyobj);
    }

    /// Returns the pointer of the Python object being held.
    PyObject* object() { return m_pyobj; }
private:
    PyObject* m_pyobj;
};


} // namespace Shiboken

#endif // AUTODECREF_H

