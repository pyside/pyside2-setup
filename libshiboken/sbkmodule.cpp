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

#include "sbkmodule.h"
#include "basewrapper.h"
#include "bindingmanager.h"
#include <unordered_map>

/// This hash maps module objects to arrays of Python types.
typedef std::unordered_map<PyObject *, PyTypeObject **> ModuleTypesMap;

/// This hash maps module objects to arrays of converters.
typedef std::unordered_map<PyObject *, SbkConverter **> ModuleConvertersMap;

/// All types produced in imported modules are mapped here.
static ModuleTypesMap moduleTypes;
static ModuleConvertersMap moduleConverters;

namespace Shiboken
{
namespace Module
{

PyObject* import(const char* moduleName)
{
    PyObject* sysModules = PyImport_GetModuleDict();
    PyObject* module = PyDict_GetItemString(sysModules, moduleName);
    if (module)
        Py_INCREF(module);
    else
        module = PyImport_ImportModule(moduleName);

    if (!module)
        PyErr_Format(PyExc_ImportError,"could not import module '%s'", moduleName);

    return module;
}

PyObject* create(const char* moduleName, void* moduleData)
{
    Shiboken::init();
#ifndef IS_PY3K
    return Py_InitModule(moduleName, reinterpret_cast<PyMethodDef *>(moduleData));
#else
    return PyModule_Create(reinterpret_cast<PyModuleDef*>(moduleData));
#endif
}

void registerTypes(PyObject* module, PyTypeObject** types)
{
    ModuleTypesMap::iterator iter = moduleTypes.find(module);
    if (iter == moduleTypes.end())
        moduleTypes.insert(std::make_pair(module, types));
}

PyTypeObject** getTypes(PyObject* module)
{
    ModuleTypesMap::iterator iter = moduleTypes.find(module);
    return (iter == moduleTypes.end()) ? 0 : iter->second;
}

void registerTypeConverters(PyObject* module, SbkConverter** converters)
{
    ModuleConvertersMap::iterator iter = moduleConverters.find(module);
    if (iter == moduleConverters.end())
        moduleConverters.insert(std::make_pair(module, converters));
}

SbkConverter** getTypeConverters(PyObject* module)
{
    ModuleConvertersMap::iterator iter = moduleConverters.find(module);
    return (iter == moduleConverters.end()) ? 0 : iter->second;
}

} } // namespace Shiboken::Module
