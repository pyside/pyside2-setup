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
static int
qApp_module_index(PyObject *module)
{
    const char *name = PyModule_GetName(module);
    int ret = 0;

    if (strcmp(name, "PySide2.QtCore") == 0)
        ret = 1;
    else if (strcmp(name, "PySide2.QtGui") == 0)
        ret = 2;
    else if (strcmp(name, "PySide2.QtWidgets") == 0)
        ret = 3;
    return ret;
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
static int qApp_var_ref = 0;
static int qApp_content_ref = 0;

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

/*
 * Note:
 * The PYSIDE-585 problem was that shutdown is called one more often
 * than Q*Application is created. We could special-case that last
 * shutdown or add a refcount, initially, but actually it was easier
 * and more intuitive in that context to make the refcount of
 * qApp_content equal to the refcount of Py_None.
 */
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
    // always know the max of the refs
    if (Py_REFCNT(qApp_var) > qApp_var_ref)
        qApp_var_ref = Py_REFCNT(qApp_var);
    if (Py_REFCNT(qApp_content) > qApp_content_ref)
        qApp_content_ref = Py_REFCNT(qApp_content);

    if (Py_TYPE(qApp_content) != Py_NONE_TYPE) {
        // Remove the "_" variable which might hold a reference to qApp.
        Shiboken::AutoDecRef pymain(PyImport_ImportModule("__main__"));
        if (pymain.object() && PyObject_HasAttrString(pymain.object(), "_"))
            PyObject_DelAttrString(pymain.object(), "_");
        Py_REFCNT(qApp_var) = 1; // fuse is armed...
    }
    if (type == Py_NONE_TYPE) {
        // Debug mode showed that we need to do more than just remove the
        // reference. To keep everything in the right order, it is easiest
        // to do a full shutdown, using QtCore.__moduleShutdown().
        // restore the "None-state"
        PyObject *__moduleShutdown = PyDict_GetItemString(qApp_moduledicts[1],
                                                          "__moduleShutdown");
        // PYSIDE-585: It was crucial to update the refcounts *before*
        // calling the shutdown.
        Py_TYPE(qApp_content) = Py_NONE_TYPE;
        Py_REFCNT(qApp_var) = qApp_var_ref;
        Py_REFCNT(qApp_content) = Py_REFCNT(Py_None);
        if (__moduleShutdown != nullptr)
            Py_XDECREF(PyObject_CallFunction(__moduleShutdown, const_cast<char *>("()")));
    }
    else
        (void)PyObject_INIT(qApp_content, type);
    Py_INCREF(qApp_content);
    return qApp_content;
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
    setup_qApp_var(module);
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
    if (qApp != nullptr) {
        Shiboken::AutoDecRef pycore(PyImport_ImportModule("PySide2.QtCore"));
        Shiboken::AutoDecRef coreapp(PyObject_GetAttrString(pycore, "QCoreApplication"));
        qApp_content = PyObject_CallMethod(coreapp, "instance", "");
        reset_qApp_var();
    }
}


} //extern "C"

// end of module
