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
#include "shibokenmacros.h"
#include "conversions.h"

namespace Shiboken
{

template<typename A, typename B>
inline PyObject* makeTuple(const A& a, const B& b)
{
    return PyTuple_Pack(2, Converter<A>::toPython(a), Converter<B>::toPython(b));
}

template<typename A, typename B, typename C>
inline PyObject* makeTuple(const A& a, const B& b, const C& c)
{
    return PyTuple_Pack(3, Converter<A>::toPython(a),
                           Converter<B>::toPython(b),
                           Converter<C>::toPython(c));
}

template<typename A, typename B, typename C, typename D>
inline PyObject* makeTuple(const A& a, const B& b, const C& c, const D& d)
{
    return PyTuple_Pack(4, Converter<A>::toPython(a),
                           Converter<B>::toPython(b),
                           Converter<C>::toPython(c),
                           Converter<D>::toPython(d));
}

template<typename A, typename B, typename C, typename D, typename E>
inline PyObject* makeTuple(const A& a, const B& b, const C& c, const D& d, const E& e)
{
    return PyTuple_Pack(5, Converter<A>::toPython(a),
                           Converter<B>::toPython(b),
                           Converter<C>::toPython(c),
                           Converter<D>::toPython(d),
                           Converter<E>::toPython(e));
}

/**
* It transforms a python sequence into two C variables, argc and argv.
* If the sequence is empty and defaultAppName was provided, argc will be 1 and
* argv will have a copy of defaultAppName.
*
* \note argc and argv *should* be deleted by the user.
* \returns True on sucess, false otherwise.
*/
LIBSHIBOKEN_API bool PySequenceToArgcArgv(PyObject* argList, int* argc, char*** argv, const char* defaultAppName = 0);

/**
 * Convert a python sequence into a heap-allocated array of ints.
 *
 * \returns The newly allocated array or NULL in case of error or empty sequence. Check with PyErr_Occurred
 *          if it was successfull.
 */
LIBSHIBOKEN_API int* sequenceToIntArray(PyObject* obj, bool zeroTerminated = false);

} // namespace Shiboken

#endif // HELPER_H

