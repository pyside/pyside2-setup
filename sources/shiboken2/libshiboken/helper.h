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

#ifndef HELPER_H
#define HELPER_H

#include "sbkpython.h"
#include "shibokenmacros.h"
#include "autodecref.h"

#define SBK_UNUSED(x)   (void)x;

namespace Shiboken
{

/**
* It transforms a python sequence into two C variables, argc and argv.
* This function tries to find the application (script) name and put it into argv[0], if
* the application name can't be guessed, defaultAppName will be used.
*
* No memory is allocated is an error occur.
*
* \note argc must be a valid address.
* \note The argv array is allocated using new operator and each item is allocated using malloc.
* \returns True on sucess, false otherwise.
*/
LIBSHIBOKEN_API bool listToArgcArgv(PyObject* argList, int* argc, char*** argv, const char* defaultAppName = 0);

/**
 * Convert a python sequence into a heap-allocated array of ints.
 *
 * \returns The newly allocated array or NULL in case of error or empty sequence. Check with PyErr_Occurred
 *          if it was successfull.
 */
LIBSHIBOKEN_API int* sequenceToIntArray(PyObject* obj, bool zeroTerminated = false);

/**
 *  Creates and automatically deallocates C++ arrays.
 */
template<class T>
class AutoArrayPointer
{
    public:
        AutoArrayPointer(int size) { data = new T[size]; }
        T& operator[](int pos) { return data[pos]; }
        operator T*() const { return data; }
        ~AutoArrayPointer() { delete[] data; }
    private:
        T* data;
};

/**
 * An utility function used to call PyErr_WarnEx with a formatted message.
 */
LIBSHIBOKEN_API int warning(PyObject* category, int stacklevel, const char* format, ...);

} // namespace Shiboken

#endif // HELPER_H
