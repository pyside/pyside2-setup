/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef HELPER_H
#define HELPER_H

#include <Python.h>

namespace Shiboken
{

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define SHIBOKEN_HELPER_DLL_IMPORT __declspec(dllimport)
#define SHIBOKEN_HELPER_DLL_EXPORT __declspec(dllexport)
#define SHIBOKEN_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define SHIBOKEN_HELPER_DLL_IMPORT __attribute__ ((visibility("default")))
#define SHIBOKEN_HELPER_DLL_EXPORT __attribute__ ((visibility("default")))
#define SHIBOKEN_HELPER_DLL_LOCAL  __attribute__ ((visibility("internal")))
#else
#define SHIBOKEN_HELPER_DLL_IMPORT
#define SHIBOKEN_HELPER_DLL_EXPORT
#define SHIBOKEN_HELPER_DLL_LOCAL
#endif
#endif

// Now we use the generic helper definitions above to define SHIBOKEN_API and SHIBOKEN_LOCAL.
// SHIBOKEN_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// SHIBOKEN_LOCAL is used for non-api symbols.

#define SHIBOKEN_API SHIBOKEN_HELPER_DLL_EXPORT
#define SHIBOKEN_LOCAL SHIBOKEN_HELPER_DLL_LOCAL

int PySequence_to_argc_argv(PyObject* argList, char** argv[]);

} // namespace Shiboken

#endif // HELPER_H

