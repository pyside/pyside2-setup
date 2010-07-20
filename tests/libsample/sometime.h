/*
 * This file is part of the Shiboken Python Binding Generator project.
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
 * more than 5% of the total time of the Library; and (ii) is limited to
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

#ifndef SOMETIME_H
#define SOMETIME_H

#include "libsamplemacros.h"
#include "str.h"
#include "implicitconv.h"
#include "objecttype.h"

class LIBSAMPLE_API Time
{
public:
    enum NumArgs {
        ZeroArgs,
        TwoArgs,
        ThreeArgs,
        FourArgs
    };

    Time()
        : m_hour(0), m_minute(0), m_second(0), m_msec(0), m_is_null(true)
    {}
    Time(int h, int m, int s = 0, int ms = 0)
        : m_hour(h), m_minute(m), m_second(s), m_msec(ms), m_is_null(false)
    {}

    ~Time() {}

    inline bool isNull() const { return m_is_null; }

    inline int hour() const { return m_hour; }
    inline int minute() const { return m_minute; }
    inline int second() const { return m_second; }
    inline int msec() const { return m_msec; }

    void setTime();
    void setTime(int h, int m, int s = 0, int ms = 0);

    // This one is completely different from the other methods in this class,
    // it was added to give the overload decisor a really hard time with
    // an value-type with implicit conversion and a default argument, and also
    // an object-type, just because I feel like it.
    NumArgs somethingCompletelyDifferent();
    NumArgs somethingCompletelyDifferent(int h, int m,
                                         ImplicitConv ic = ImplicitConv::CtorThree,
                                         ObjectType* type = 0);

    Str toString() const;
    bool operator==(const Time& other) const;
    bool operator!=(const Time& other) const;

    // This cast operator must become an implicit conversion of Str.
    operator Str() const;

private:
    int m_hour;
    int m_minute;
    int m_second;
    int m_msec;

    bool m_is_null;
};

#endif // SOMETIME_H

