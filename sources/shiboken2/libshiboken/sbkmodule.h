/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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

#ifndef SBK_MODULE_H
#define SBK_MODULE_H

#include "sbkpython.h"
#include "shibokenmacros.h"

#if PY_MAJOR_VERSION >= 3
    #define SBK_MODULE_INIT_ERROR 0
    #define SBK_MODULE_INIT_FUNCTION_BEGIN(ModuleName) \
        extern "C" SBK_EXPORT_MODULE PyObject* PyInit_##ModuleName() {

    #define SBK_MODULE_INIT_FUNCTION_END \
        return module; }
#else
    #define SBK_MODULE_INIT_ERROR
    #define SBK_MODULE_INIT_FUNCTION_BEGIN(ModuleName) \
        extern "C" SBK_EXPORT_MODULE void init##ModuleName() {

    #define SBK_MODULE_INIT_FUNCTION_END \
        }
#endif

extern "C"
{
struct SbkConverter;
}

namespace Shiboken {
namespace Module {

/**
 *  Imports and returns the module named \p moduleName, or a NULL pointer in case of failure.
 *  If the module is already imported, it increments its reference count before returning it.
 *  \returns the module specified in \p moduleName or NULL if an error occurs.
 */
LIBSHIBOKEN_API PyObject* import(const char* moduleName);

/**
 *  Creates a new Python module named \p moduleName using the information passed in \p moduleData.
 *  In fact, \p moduleData expects a "PyMethodDef*" object, but that's for Python 2. A void*
 *  was preferred to make this work with future Python 3 support.
 *  \returns a newly created module.
 */
LIBSHIBOKEN_API PyObject* create(const char* moduleName, void* moduleData);

/**
 *  Registers the list of types created by \p module.
 *  \param module   Module where the types were created.
 *  \param types    Array of PyTypeObject* objects representing the types created on \p module.
 */
LIBSHIBOKEN_API void registerTypes(PyObject* module, PyTypeObject** types);

/**
 *  Retrieves the array of types.
 *  \param module   Module where the types were created.
 *  \returns        A pointer to the PyTypeObject* array of types.
 */
LIBSHIBOKEN_API PyTypeObject** getTypes(PyObject* module);

/**
 *  Registers the list of converters created by \p module for non-wrapper types.
 *  \param module       Module where the converters were created.
 *  \param converters   Array of SbkConverter* objects representing the converters created on \p module.
 */
LIBSHIBOKEN_API void registerTypeConverters(PyObject* module, SbkConverter** converters);

/**
 *  Retrieves the array of converters.
 *  \param module   Module where the converters were created.
 *  \returns        A pointer to the SbkConverter* array of converters.
 */
LIBSHIBOKEN_API SbkConverter** getTypeConverters(PyObject* module);

} } // namespace Shiboken::Module

#endif // SBK_MODULE_H
