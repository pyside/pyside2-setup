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

#ifndef AUTODECREF_H
#define AUTODECREF_H

#include "sbkpython.h"
#include "basewrapper.h"

#ifdef _MSC_VER
__pragma(warning(push))
__pragma(warning(disable:4522)) // warning: C4522: 'Shiboken::AutoDecRef': multiple assignment operators specified
#endif

struct SbkObject;
namespace Shiboken
{

/**
 *  AutoDecRef holds a PyObject pointer and decrement its reference counter when destroyed.
 */
struct LIBSHIBOKEN_API AutoDecRef
{
public:
    /**
     * AutoDecRef constructor.
     * \param pyobj A borrowed reference to a Python object
     */
    explicit AutoDecRef(PyObject* pyObj) : m_pyObj(pyObj) {}
    /**
     * AutoDecRef constructor.
     * \param pyobj A borrowed reference to a Python object
     */
    explicit AutoDecRef(SbkObject* pyObj) : m_pyObj(reinterpret_cast<PyObject*>(pyObj)) {}

    /// Decref the borrowed python reference
    ~AutoDecRef()
    {
        Py_XDECREF(m_pyObj);
    }

    inline bool isNull() const { return m_pyObj == 0; }
    /// Returns the pointer of the Python object being held.
    inline PyObject* object() { return m_pyObj; }
    inline operator PyObject*() { return m_pyObj; }
#ifndef Py_LIMITED_API
    inline operator PyTupleObject*() { return reinterpret_cast<PyTupleObject*>(m_pyObj); }
#endif
    inline operator bool() const { return m_pyObj != 0; }
    inline PyObject* operator->() { return m_pyObj; }

    template<typename T>
    T cast()
    {
        return reinterpret_cast<T>(m_pyObj);
    }

    /**
     * Decref the current borrowed python reference and take the reference
     * borrowed by \p other, so other.isNull() will return true.
     */
    void operator=(AutoDecRef& other)
    {
        Py_XDECREF(m_pyObj);
        m_pyObj = other.m_pyObj;
        other.m_pyObj = 0;
    }

    /**
     * Decref the current borrowed python reference and borrow \p other.
     */
    void operator=(PyObject* other)
    {
        Py_XDECREF(m_pyObj);
        m_pyObj = other;
    }
private:
    PyObject* m_pyObj;
    AutoDecRef(const AutoDecRef&);
    AutoDecRef& operator=(const AutoDecRef&);
};

} // namespace Shiboken

#ifdef _MSC_VER
__pragma(warning(pop))
#endif

#endif // AUTODECREF_H

