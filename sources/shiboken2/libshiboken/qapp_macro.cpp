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
// This variable is also able to destroy the app by deleting qApp.
//
static const char *mod_names[3] = {"PySide2.QtCore", "PySide2.QtGui", "PySide2.QtWidgets"};

static int
qApp_module_index(PyObject *module)
{
    const char *name = PyModule_GetName(module);
    for (int idx = 0; idx < 3; idx++)
        if (strcmp(name, mod_names[idx]) == 0)
            return idx + 1;
    return 0;
}

#define PYTHON_IS_PYTHON3               (PY_VERSION_HEX >= 0x03000000)
#define PYTHON_IS_PYTHON2               (!PYTHON_IS_PYTHON3)
#define Py_NONE_TYPE                    Py_TYPE(Py_None)

#if PYTHON_IS_PYTHON3
#  define BRACE_OPEN {
#  define BRACE_CLOSE }
#else
#  define BRACE_OPEN
#  define BRACE_CLOSE
#endif

static SbkObject _Py_ChameleonQAppWrapper_Struct = {
    BRACE_OPEN
        _PyObject_EXTRA_INIT
        1, Py_NONE_TYPE
    BRACE_CLOSE
};

static PyObject *qApp_var = nullptr;
static PyObject *qApp_content = reinterpret_cast<PyObject *>(&_Py_ChameleonQAppWrapper_Struct);
static PyObject *qApp_moduledicts[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

static int
reset_qApp_var(void)
{
    PyObject **mod_ptr;

    for (mod_ptr = qApp_moduledicts; *mod_ptr != nullptr; mod_ptr++) {
        // We respect whatever the user may have set.
        PyObject *existing = PyDict_GetItem(*mod_ptr, qApp_var);
        if (existing == nullptr || Py_TYPE(existing) == Py_NONE_TYPE) {
            if (PyDict_SetItem(*mod_ptr, qApp_var, qApp_content) < 0)
                return -1;
        }
    }
    return 0;
}

static bool app_created = false;

PyObject *
MakeSingletonQAppWrapper(PyTypeObject *type)
{
    if (type == nullptr)
        type = Py_NONE_TYPE;
    if (!(type == Py_NONE_TYPE || Py_TYPE(qApp_content) == Py_NONE_TYPE)) {
        const char *res_name = PepType_GetNameStr(Py_TYPE(qApp_content));
        const char *type_name = PepType_GetNameStr(type);
        PyErr_Format(PyExc_RuntimeError, "Please destroy the %s singleton before"
            " creating a new %s instance.", res_name, type_name);
        return nullptr;
    }
    if (reset_qApp_var() < 0)
        return nullptr;
    if (type == Py_NONE_TYPE) {
        // PYSIDE-1093: Ignore None when no instance has ever been created.
        if (!app_created)
            Py_RETURN_NONE;
        Py_TYPE(qApp_content) = Py_NONE_TYPE;
    } else {
        PyObject_Init(qApp_content, type);
        app_created = true;
    }
    Py_INCREF(qApp_content);
    return qApp_content;
}

// PYSIDE-1158: Be clear that the QApp none has the type of None but is a
// different thing.

static PyObject *
none_repr(PyObject *op)
{
    if (op == qApp_content)
        return PyUnicode_FromString("noApp");
    return PyUnicode_FromString("None");
}

static void
none_dealloc(PyObject *ignore)
{
    if (ignore == qApp_content)
        return;
    /* This should never get called, but we also don't want to SEGV if
     * we accidentally decref None out of existence.
     */
    Py_FatalError("deallocating None");
}

#if PYTHON_IS_PYTHON2

// Install support in Py_NONE_TYPE for Python 2: 'bool(qApp) == False'.
static int
none_bool(PyObject *v)
{
    return 0;
}

static PyNumberMethods none_as_number = {
    nullptr,                                            /* nb_add */
    nullptr,                                            /* nb_subtract */
    nullptr,                                            /* nb_multiply */
    nullptr,                                            /* nb_divide */
    nullptr,                                            /* nb_remainder */
    nullptr,                                            /* nb_divmod */
    nullptr,                                            /* nb_power */
    nullptr,                                            /* nb_negative */
    nullptr,                                            /* nb_positive */
    nullptr,                                            /* nb_absolute */
    reinterpret_cast<inquiry>(none_bool),               /* nb_nonzero */
};

#endif

static int
setup_qApp_var(PyObject *module)
{
    int module_index;
    static int init_done = 0;

    if (!init_done) {
        Py_NONE_TYPE->tp_repr = &none_repr;
        Py_NONE_TYPE->tp_dealloc = &none_dealloc;
#if PYTHON_IS_PYTHON2
        Py_NONE_TYPE->tp_as_number = &none_as_number;
#endif
        qApp_var = Py_BuildValue("s", "qApp");
        if (qApp_var == nullptr)
            return -1;
        // This is a borrowed reference
        qApp_moduledicts[0] = PyEval_GetBuiltins();
        Py_INCREF(qApp_moduledicts[0]);
        init_done = 1;
    }

    // Initialize qApp. We insert it into __dict__ for "import *" and also
    // into __builtins__, to let it appear like a real macro.
    module_index = qApp_module_index(module);
    if (module_index) {
        // This line gets a borrowed reference
        qApp_moduledicts[module_index] = PyModule_GetDict(module);
        Py_INCREF(qApp_moduledicts[module_index]);
        if (reset_qApp_var() < 0)
            return -1;
    }
    return 0;
}

void
NotifyModuleForQApp(PyObject *module, void *qApp)
{
    /*
     * PYSIDE-571: Check if an QApplication instance exists before the import.
     * This happens in scriptableapplication and application_test.py .
     *
     * Crucial Observation
     * ===================
     *
     * A Q*Application object from C++ does not have a wrapper or constructor
     * like instances created by Python. It makes no sense to support
     * deletion or special features like qApp resurrection.
     *
     * Therefore, the implementation is very simple and just redirects the
     * qApp_contents variable and assigns the instance, instead of vice-versa.
     */

    // PYSIDE-1135: Make sure that at least QtCore gets imported.
    // That problem exists when a derived instance is created in C++.
    // PYSIDE-1164: Use the highest Q*Application module possible,
    // because in embedded mode the instance() seems to be sticky.

    // PYSIDE-1135 again:
    // The problem of late initialization is not worth the effort.
    // We simply don't create the qApp variable when we are embedded.
    if (qApp == nullptr)
        setup_qApp_var(module);
}


} //extern "C"

// end of module
