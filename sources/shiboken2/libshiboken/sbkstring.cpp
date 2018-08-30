/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "sbkstring.h"
#include "autodecref.h"

namespace Shiboken
{

namespace String
{

bool checkType(PyTypeObject* type)
{
    return type == &PyUnicode_Type
#if PY_MAJOR_VERSION < 3
            || type == &PyString_Type
#endif
    ;
}

bool check(PyObject* obj)
{
    return obj == Py_None ||
#if PY_MAJOR_VERSION < 3
        PyString_Check(obj) ||
#endif
        PyUnicode_Check(obj);
}

bool checkChar(PyObject* pyobj)
{
    return check(pyobj) && (len(pyobj) == 1);
}

bool isConvertible(PyObject* obj)
{
    return check(obj);
}

PyObject* fromCString(const char* value)
{
#ifdef IS_PY3K
    return PyUnicode_FromString(value);
#else
    return PyBytes_FromString(value);
#endif
}

PyObject* fromCString(const char* value, int len)
{
#ifdef IS_PY3K
    return PyUnicode_FromStringAndSize(value, len);
#else
    return PyBytes_FromStringAndSize(value, len);
#endif
}

const char* toCString(PyObject* str, Py_ssize_t* len)
{
    if (str == Py_None)
        return NULL;
    if (PyUnicode_Check(str)) {
        if (len) {
            // We need to encode the unicode string into utf8 to know the size of returned char*.
            Shiboken::AutoDecRef uniStr(PyUnicode_AsUTF8String(str));
            *len = PyBytes_GET_SIZE(uniStr.object());
        }
#ifdef IS_PY3K
        // Return unicode from str instead of uniStr, because the lifetime of the returned pointer
        // depends on the lifetime of str.
        return _PepUnicode_AsString(str);
#else
        str = PyUnicode_AsUTF8String(str);
        if (str == NULL) {
            return NULL;
        }
        return PyString_AsString(str);
#endif
    }
    if (PyBytes_Check(str)) {
        if (len)
            *len = PyBytes_GET_SIZE(str);
        return PyBytes_AS_STRING(str);
    }
    return 0;
}

bool concat(PyObject** val1, PyObject* val2)
{
    if (PyUnicode_Check(*val1) && PyUnicode_Check(val2)) {
        PyObject* result = PyUnicode_Concat(*val1, val2);
        Py_DECREF(*val1);
        *val1 = result;
        return true;
    }

    if (PyBytes_Check(*val1) && PyBytes_Check(val2)) {
        PyBytes_Concat(val1, val2);
        return true;
    }

#if PY_MAJOR_VERSION < 3
    if (PyString_Check(*val1) && PyString_Check(val2)) {
        PyString_Concat(val1, val2);
        return true;
    }
#endif
    return false;
}

PyObject* fromFormat(const char* format, ...)
{
    va_list argp;
    va_start(argp, format);
    PyObject* result = 0;
#ifdef IS_PY3K
    result = PyUnicode_FromFormatV(format, argp);
#else
    result = PyString_FromFormatV(format, argp);
#endif
    va_end(argp);
    return result;
}

PyObject* fromStringAndSize(const char* str, Py_ssize_t size)
{
#ifdef IS_PY3K
    return PyUnicode_FromStringAndSize(str, size);
#else
    return PyString_FromStringAndSize(str, size);
#endif
}

int compare(PyObject* val1, const char* val2)
{
    if (PyUnicode_Check(val1))
#ifdef IS_PY3K
       return PyUnicode_CompareWithASCIIString(val1, val2);
#else
    {
        PyObject* uVal2 = PyUnicode_FromString(val2);
        bool result =  PyUnicode_Compare(val1, uVal2);
        Py_XDECREF(uVal2);
        return result;
    }
    if (PyString_Check(val1))
        return strcmp(PyString_AS_STRING(val1), val2);
#endif
    return 0;

}

Py_ssize_t len(PyObject* str)
{
    if (str == Py_None)
        return 0;

    if (PyUnicode_Check(str))
        return PyUnicode_GET_SIZE(str);

    if (PyBytes_Check(str))
        return PyBytes_GET_SIZE(str);
    return 0;
}

} // namespace String

} // namespace Shiboken
