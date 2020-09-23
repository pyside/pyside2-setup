/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

////////////////////////////////////////////////////////////////////////////
//
// signature_global.cpp
//
// This file contains the global data structures and init code.
//

#include "autodecref.h"
#include "sbkstring.h"
#include "sbkstaticstrings.h"
#include "sbkstaticstrings_p.h"

#include "signature_p.h"

using namespace Shiboken;

extern "C" {

static const char *PySide_CompressedSignaturePackage[] = {
#include "embed/signature_inc.h"
    };

static const unsigned char PySide_SignatureLoader[] = {
#include "embed/signature_bootstrap_inc.h"
    };

static PyObject *_init_pyside_extension(PyObject * /* self */, PyObject * /* args */)
{
    init_module_1();
    init_module_2();
    Py_RETURN_NONE;
}

// This function will be inserted into __builtins__.
static PyMethodDef init_methods[] = {
    {"_init_pyside_extension", (PyCFunction)_init_pyside_extension, METH_NOARGS},
    {nullptr, nullptr}
};

static safe_globals_struc *init_phase_1(PyMethodDef *init_meth)
{
    {
        auto *p = reinterpret_cast<safe_globals_struc *>
                                    (malloc(sizeof(safe_globals_struc)));
        if (p == nullptr)
            goto error;
        /*
         * Initializing module signature_bootstrap.
         * Since we now have an embedding script, we can do this without any
         * Python strings in the C code.
         */
#ifdef Py_LIMITED_API
        // We must work for multiple versions, so use source code.
#else
        AutoDecRef marshal_module(PyImport_Import(PyName::marshal()));
        if (marshal_module.isNull())
            goto error;
        AutoDecRef loads(PyObject_GetAttr(marshal_module, PyName::loads()));
        if (loads.isNull())
            goto error;
#endif
        char *bytes_cast = reinterpret_cast<char *>(
                                       const_cast<unsigned char *>(PySide_SignatureLoader));
        AutoDecRef bytes(PyBytes_FromStringAndSize(bytes_cast, sizeof(PySide_SignatureLoader)));
        if (bytes.isNull())
            goto error;
#ifdef Py_LIMITED_API
        PyObject *builtins = PyEval_GetBuiltins();
        PyObject *compile = PyDict_GetItem(builtins, PyName::compile());
        if (compile == nullptr)
            goto error;
        AutoDecRef code_obj(PyObject_CallFunction(compile, "Oss",
                                bytes.object(), "(builtin)", "exec"));
#else
        AutoDecRef code_obj(PyObject_CallFunctionObjArgs(
                                loads, bytes.object(), nullptr));
#endif
        if (code_obj.isNull())
            goto error;
        p->helper_module = PyImport_ExecCodeModule(const_cast<char *>
                                                       ("signature_bootstrap"), code_obj);
        if (p->helper_module == nullptr)
            goto error;
        // Initialize the module
        PyObject *mdict = PyModule_GetDict(p->helper_module);
        if (PyDict_SetItem(mdict, PyMagicName::builtins(), PyEval_GetBuiltins()) < 0)
            goto error;
        /*
         * Unpack an embedded ZIP file with more signature modules.
         * They will be loaded later with the zipimporter.
         * Due to MSVC's limitation to 64k strings, we need to assemble pieces.
         */
        const char **block_ptr = (const char **)PySide_CompressedSignaturePackage;
        int npieces = 0;
        PyObject *piece, *zipped_string_sequence = PyList_New(0);
        if (zipped_string_sequence == nullptr)
            return nullptr;
        for (; **block_ptr != 0; ++block_ptr) {
            npieces++;
            // we avoid the string/unicode dilemma by not using PyString_XXX:
            piece = Py_BuildValue("s", *block_ptr);
            if (piece == nullptr || PyList_Append(zipped_string_sequence, piece) < 0)
                goto error;
        }
        if (PyDict_SetItemString(mdict, "zipstring_sequence", zipped_string_sequence) < 0)
            goto error;
        Py_DECREF(zipped_string_sequence);

        // build a dict for diverse mappings
        p->map_dict = PyDict_New();
        if (p->map_dict == nullptr)
            goto error;

        // build a dict for the prepared arguments
        p->arg_dict = PyDict_New();
        if (p->arg_dict == nullptr
            || PyObject_SetAttrString(p->helper_module, "pyside_arg_dict", p->arg_dict) < 0)
            goto error;

        // build a dict for assigned signature values
        p->value_dict = PyDict_New();
        if (p->value_dict == nullptr)
            goto error;

        // PYSIDE-1019: build a __feature__ dict
        p->feature_dict = PyDict_New();
        if (p->feature_dict == nullptr
            || PyObject_SetAttrString(p->helper_module, "pyside_feature_dict", p->feature_dict) < 0)
            goto error;

        // This function will be disabled until phase 2 is done.
        p->finish_import_func = nullptr;

        // Initialize the explicit init function.
        AutoDecRef init(PyCFunction_NewEx(init_meth, nullptr, nullptr));
        if (init.isNull()
            || PyDict_SetItemString(PyEval_GetBuiltins(), init_meth->ml_name, init) != 0)
            goto error;

        return p;
    }
error:
    PyErr_Print();
    Py_FatalError("could not initialize part 1");
    return nullptr;
}

static int init_phase_2(safe_globals_struc *p, PyMethodDef *methods)
{
    {
        PyMethodDef *ml;

        // The single function to be called, but maybe more to come.
        for (ml = methods; ml->ml_name != nullptr; ml++) {
            PyObject *v = PyCFunction_NewEx(ml, nullptr, nullptr);
            if (v == nullptr
                || PyObject_SetAttrString(p->helper_module, ml->ml_name, v) != 0)
                goto error;
            Py_DECREF(v);
        }
        PyObject *bootstrap_func = PyObject_GetAttrString(p->helper_module, "bootstrap");
        if (bootstrap_func == nullptr)
            goto error;
        // The return value of the bootstrap function is the loader module.
        PyObject *loader = PyObject_CallFunction(bootstrap_func, const_cast<char *>("()"));
        if (loader == nullptr)
            goto error;
        // now the loader should be initialized
        p->pyside_type_init_func = PyObject_GetAttrString(loader, "pyside_type_init");
        if (p->pyside_type_init_func == nullptr)
            goto error;
        p->create_signature_func = PyObject_GetAttrString(loader, "create_signature");
        if (p->create_signature_func == nullptr)
            goto error;
        p->seterror_argument_func = PyObject_GetAttrString(loader, "seterror_argument");
        if (p->seterror_argument_func == nullptr)
            goto error;
        p->make_helptext_func = PyObject_GetAttrString(loader, "make_helptext");
        if (p->make_helptext_func == nullptr)
            goto error;
        p->finish_import_func = PyObject_GetAttrString(loader, "finish_import");
        if (p->finish_import_func == nullptr)
            goto error;
        return 0;
    }
error:
    PyErr_Print();
    Py_FatalError("could not initialize part 2");
    return -1;
}

#ifndef _WIN32
////////////////////////////////////////////////////////////////////////////
// a stack trace for linux-like platforms
#include <stdio.h>
#if defined(__GLIBC__)
#  include <execinfo.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static void handler(int sig) {
#if defined(__GLIBC__)
    void *array[30];
    size_t size;

    // get void *'s for all entries on the stack
    size = backtrace(array, 30);

    // print out all the frames to stderr
#endif
    fprintf(stderr, "Error: signal %d:\n", sig);
#if defined(__GLIBC__)
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
    exit(1);
}

////////////////////////////////////////////////////////////////////////////
#endif // _WIN32

safe_globals pyside_globals = nullptr;

void init_module_1(void)
{
    static int init_done = 0;

    if (!init_done) {
        pyside_globals = init_phase_1(init_methods);
        if (pyside_globals != nullptr)
            init_done = 1;

#ifndef _WIN32
        // We enable the stack trace in CI, only.
        const char *testEnv = getenv("QTEST_ENVIRONMENT");
        if (testEnv && strstr(testEnv, "ci"))
            signal(SIGSEGV, handler);   // install our handler
#endif // _WIN32

    }
}

void init_module_2(void)
{
    static int init_done = 0;

    if (!init_done) {
        // Phase 2 will call __init__.py which touches a signature, itself.
        // Therefore we set init_done prior to init_phase_2().
        init_done = 1;
        init_phase_2(pyside_globals, signature_methods);
    }
}

} // extern "C"
