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

#ifndef SBKPYTHON_H
#define SBKPYTHON_H

#include "Python.h"
#include "python25compat.h"

#if PY_MAJOR_VERSION >= 3
    #define IS_PY3K

    #define PyInt_Type PyLong_Type
    #define PyInt_Check PyLong_Check
    #define PyInt_CheckExact PyLong_CheckExact
    #define PyInt_FromString PyLong_FromString
    #define PyInt_FromSsize_t PyLong_FromSsize_t
    #define PyInt_FromSize_t PyLong_FromSize_t
    #define PyInt_AS_LONG PyLong_AS_LONG
    #define PyInt_AsUnsignedLongLongMask PyLong_AsLongLong
    #define PyInt_FromLong PyLong_FromLong
    #define PyInt_AsLong PyLong_AsLong
    #define SbkNumber_Check PyNumber_Check
    #define Py_TPFLAGS_CHECKTYPES  0

    #define SBK_NB_BOOL(x) (x).nb_bool
    #define SBK_PyMethod_New PyMethod_New
    #define PyInt_AsSsize_t(x)  PyLong_AsSsize_t(x)
    #define PyString_Type PyUnicode_Type

#else
    // Note: if there wasn't for the old-style classes, only a PyNumber_Check would suffice.
    #define SbkNumber_Check(X) \
            (PyNumber_Check(X) && (!PyInstance_Check(X) || PyObject_HasAttrString(X, "__trunc__")))
    #define SBK_NB_BOOL(x) (x).nb_nonzero
    #define SBK_STR_NAME "str"
    #define SBK_PyMethod_New(X, Y) PyMethod_New(X, Y, reinterpret_cast<PyObject *>(Py_TYPE(Y)))

    #define Py_hash_t long
#endif

#endif
