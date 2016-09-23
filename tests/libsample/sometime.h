/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

