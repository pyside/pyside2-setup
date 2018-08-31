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

#ifndef SBKDBG_H
#define SBKDBG_H

#include "sbkpython.h"
#include "basewrapper.h"
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
    BaseLogger(const BaseLogger&) = delete;
    BaseLogger(BaseLogger&&) = delete;
    BaseLogger& operator=(const BaseLogger&) = delete;
    BaseLogger& operator=(BaseLogger&&) = delete;

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
    PyObject* repr = Shiboken::Object::isValid(obj, false) ? PyObject_Repr(obj) : 0;
    if (repr) {
#ifdef IS_PY3K
        PyObject* str = PyUnicode_AsUTF8String(repr);
        Py_DECREF(repr);
        repr = str;
#endif
        out << PyBytes_AS_STRING(repr);
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
