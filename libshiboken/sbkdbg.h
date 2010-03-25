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

#ifndef SBKDBG_H
#define SBKDBG_H

#include <Python.h>
#include <iostream>

#ifndef NOCOLOR
    #define COLOR_END "\033[0m"
    #define COLOR_WHITE "\033[1;37m"
    #define COLOR_YELLOW "\033[1;33m"
    #define COLOR_GREEN "\033[0;32m"
    #define COLOR_RED "\033[0;31m"
#else
    #define COLOR_END ""
    #define COLOR_WHITE ""
    #define COLOR_YELLOW ""
    #define COLOR_GREEN ""
    #define COLOR_RED ""
#endif

#ifndef NDEBUG

class BaseLogger
{
public:
    BaseLogger(std::ostream& output, const char* function, const char* context)
        : m_stream(output), m_function(function), m_context(context) {}
    ~BaseLogger()
    {
        m_stream << std::endl;
    }
    std::ostream& operator()() { return m_stream; };
    template <typename T>
    std::ostream& operator<<(const T& t)
    {
        m_stream << '[';
        if (m_context[0])
            m_stream << COLOR_GREEN << m_context << COLOR_END << "|";
        return m_stream << COLOR_WHITE << m_function << COLOR_END << "] " << t;
    }
private:
    std::ostream& m_stream;
    const char* m_function;
    const char* m_context;
};

inline std::ostream& operator<<(std::ostream& out, PyObject* obj)
{
    PyObject* repr = PyObject_Repr(obj);
    if (repr) {
        out << PyString_AS_STRING(repr);
        Py_DECREF(repr);
    } else {
        out << reinterpret_cast<void*>(obj);
    }
    return out;
}

class _SbkDbg : public BaseLogger
{
public:
    _SbkDbg(const char* function, const char* context = "") : BaseLogger(std::cout, function, context) {}
};

#ifdef __GNUG__
#define SbkDbg(X) _SbkDbg(__PRETTY_FUNCTION__, X"")
#else
#define SbkDbg(X) _SbkDbg(__FUNCTION__, X"")
#endif

#else

struct SbkDbg {
    template <typename T>
    SbkDbg& operator<<(const T&) { return *this; }
};

#endif
#endif // LOGGER_H
