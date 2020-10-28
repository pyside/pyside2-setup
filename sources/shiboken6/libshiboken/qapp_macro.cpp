/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "basewrapper.h"
#include "autodecref.h"

extern "C"
{

#include "qapp_macro.h"

////////////////////////////////////////////////////////////////////////////
//
// Support for the qApp macro.
//
// qApp is a macro in Qt5. In Python, we simulate that a little by a
// variable that monitors Q*Application.instance().
// This variable is also able to destroy the app by qApp.shutdown().
//

static PyObject *qApp_var = nullptr;
static PyObject *qApp_content = nullptr;

static PyObject *
monitor_qApp_var(PyObject *qApp)
{
    static bool init_done;
    static PyObject *builtins = PyEval_GetBuiltins();

    if (!init_done) {
        qApp_var = Py_BuildValue("s", "qApp");
        if (qApp_var == nullptr)
            return nullptr;
        // This is a borrowed reference
        Py_INCREF(builtins);
        init_done = true;
    }

    if (PyDict_SetItem(builtins, qApp_var, qApp) < 0)
        return nullptr;
    qApp_content = qApp;
    Py_INCREF(qApp);
    return qApp;
}

PyObject *
MakeQAppWrapper(PyTypeObject *type)
{
    if (type == nullptr)
        type = Py_TYPE(Py_None);
    if (!(type == Py_TYPE(Py_None) || Py_TYPE(qApp_content) == Py_TYPE(Py_None))) {
        const char *res_name = PepType_GetNameStr(Py_TYPE(qApp_content));
        const char *type_name = PepType_GetNameStr(type);
        PyErr_Format(PyExc_RuntimeError, "Please destroy the %s singleton before"
            " creating a new %s instance.", res_name, type_name);
        return nullptr;
    }
    PyObject *self = type != Py_TYPE(Py_None) ? PyObject_New(PyObject, type) : Py_None;
    return monitor_qApp_var(self);
}

} //extern "C"

// end of module
