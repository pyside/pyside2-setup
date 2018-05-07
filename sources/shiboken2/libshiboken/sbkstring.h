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

#ifndef SBKSTRING_H
#define SBKSTRING_H

#include "sbkpython.h"
#include "shibokenmacros.h"

#if PY_MAJOR_VERSION >= 3
    #define SBK_STR_NAME "unicode"
#else
    #define SBK_STR_NAME "str"
#endif

namespace Shiboken
{
namespace String
{
    LIBSHIBOKEN_API bool check(PyObject* obj);
    LIBSHIBOKEN_API bool checkType(PyTypeObject* obj);
    LIBSHIBOKEN_API bool checkChar(PyObject* obj);
    LIBSHIBOKEN_API bool isConvertible(PyObject* obj);
    LIBSHIBOKEN_API PyObject* fromCString(const char* value);
    LIBSHIBOKEN_API PyObject* fromCString(const char* value, int len);
    LIBSHIBOKEN_API const char* toCString(PyObject* str, Py_ssize_t* len = 0);
    LIBSHIBOKEN_API bool concat(PyObject** val1, PyObject* val2);
    LIBSHIBOKEN_API PyObject* fromFormat(const char* format, ...);
    LIBSHIBOKEN_API PyObject* fromStringAndSize(const char* str, Py_ssize_t size);
    LIBSHIBOKEN_API int compare(PyObject* val1, const char* val2);
    LIBSHIBOKEN_API Py_ssize_t len(PyObject* str);

} // namespace String
} // namespace Shiboken


#endif


