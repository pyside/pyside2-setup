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

#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include <Python.h>
#include <basewrapper.h>
#include <bindingmanager.h>

namespace Shiboken
{

template <typename T>
struct ValueHolder
{
    explicit ValueHolder(T val) : value(val) {}
    T value;
};

template <typename T>
struct Converter
{
    static PyObject* toPython(ValueHolder<T> cppobj);
    static T toCpp(PyObject* pyobj);
};

template <typename T>
struct Converter<T &> : Converter<T> {};

template <typename T>
struct Converter<const T &> : Converter<T> {};

// Object Types ---------------------------------------------------------------
template <>
struct Converter<void*>
{
    static PyObject* toPython(ValueHolder<void*> cppobj)
    {
        PyObject* obj = BindingManager::instance().retrieveWrapper(cppobj.value);
        Py_XINCREF(obj);
        return obj;
    }
    static void* toCpp(PyObject* pyobj)
    {
        return ((Shiboken::PyBaseWrapper*) pyobj)->cptr;
    }
};

// Primitive Types ------------------------------------------------------------
template <>
struct Converter<bool>
{
    static PyObject* toPython(ValueHolder<bool> holder)
    {
        return PyBool_FromLong(holder.value);
    }
    static bool toCpp(PyObject* pyobj)
    {
        return pyobj == Py_True;
    }
};

template <typename PyIntEquiv>
struct Converter_PyInt
{
    static PyObject* toPython(ValueHolder<PyIntEquiv> holder)
    {
        return PyInt_FromLong((long) holder.value);
    }
    static PyIntEquiv toCpp(PyObject* pyobj)
    {
        if (PyFloat_Check(pyobj))
            return (PyIntEquiv) PyFloat_AS_DOUBLE(pyobj);
        return (PyIntEquiv) PyInt_AS_LONG(pyobj);
    }
};

template <> struct Converter<char> : Converter_PyInt<char> {};
template <> struct Converter<unsigned char> : Converter_PyInt<unsigned char> {};
template <> struct Converter<int> : Converter_PyInt<int> {};
template <> struct Converter<unsigned int> : Converter_PyInt<unsigned int> {};
template <> struct Converter<short> : Converter_PyInt<short> {};
template <> struct Converter<unsigned short> : Converter_PyInt<unsigned short> {};
template <> struct Converter<long> : Converter_PyInt<long> {};

template <>
struct Converter<unsigned long>
{
    static PyObject* toPython(ValueHolder<unsigned long> holder)
    {
        return PyLong_FromUnsignedLong(holder.value);
    }
    static unsigned long toCpp(PyObject* pyobj)
    {
        return (unsigned long) PyLong_AsUnsignedLong(pyobj);
    }
};

template <>
struct Converter<PY_LONG_LONG>
{
    static PyObject* toPython(ValueHolder<PY_LONG_LONG> holder)
    {
        return PyLong_FromLongLong(holder.value);
    }
    static PY_LONG_LONG toCpp(PyObject* pyobj)
    {
        return (PY_LONG_LONG) PyLong_AsLongLong(pyobj);
    }
};

template <>
struct Converter<unsigned PY_LONG_LONG>
{
    static PyObject* toPython(ValueHolder<unsigned PY_LONG_LONG> holder)
    {
        return PyLong_FromUnsignedLongLong(holder.value);
    }
    static unsigned PY_LONG_LONG toCpp(PyObject* pyobj)
    {
        return (unsigned PY_LONG_LONG) PyLong_AsUnsignedLongLong(pyobj);
    }
};

template <typename PyFloatEquiv>
struct Converter_PyFloat
{
    static PyObject* toPython(ValueHolder<PyFloatEquiv> holder)
    {
        return PyFloat_FromDouble((double) holder.value);
    }
    static PyFloatEquiv toCpp(PyObject* pyobj)
    {
        if (PyInt_Check(pyobj))
            return (PyFloatEquiv) PyInt_AS_LONG(pyobj);
        return (PyFloatEquiv) PyFloat_AS_DOUBLE(pyobj);
    }
};

template <> struct Converter<float> : Converter_PyFloat<float> {};
template <> struct Converter<double> : Converter_PyFloat<double> {};

// C Sting Types --------------------------------------------------------------

template <typename CString>
struct Converter_CString
{
    static PyObject* toPython(ValueHolder<CString> holder)
    {
        return PyString_FromString(holder.value);
    }
    static CString toCpp(PyObject* pyobj)
    {
        return PyString_AsString(pyobj);
    }
};

template <> struct Converter<char*> : Converter_CString<char*> {};
template <> struct Converter<const char*> : Converter_CString<const char*> {};

} // namespace Shiboken

#endif // CONVERSIONS_H

