/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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

#ifndef ODDBOOL_H
#define ODDBOOL_H

#include "libsamplemacros.h"

class OddBool
{

public:
    inline explicit OddBool(bool b) : m_value(b) {}
    bool value() const { return m_value; }

    inline OddBool operator!() const { return OddBool(!m_value); }

private:
    bool m_value;
};

inline bool operator==(OddBool b1, bool b2) { return (!b1).value() == !b2; }
inline bool operator==(bool b1, OddBool b2) { return (!b1) == (!b2).value(); }
inline bool operator==(OddBool b1, OddBool b2) { return (!b1).value() == (!b2).value(); }
inline bool operator!=(OddBool b1, bool b2) { return (!b1).value() != !b2; }
inline bool operator!=(bool b1, OddBool b2) { return (!b1) != (!b2).value(); }
inline bool operator!=(OddBool b1, OddBool b2) { return (!b1).value() != (!b2).value(); }

class OddBoolUser
{
public:
    OddBoolUser() : m_oddbool(OddBool(false)) {}
    OddBoolUser(const OddBool& oddBool) : m_oddbool(oddBool) {}
    virtual ~OddBoolUser() {}

    inline OddBool oddBool() { return m_oddbool; }
    inline void setOddBool(OddBool oddBool) { m_oddbool = oddBool; }

    virtual OddBool invertedOddBool()
    {
        return !m_oddbool;
    }

    inline OddBool callInvertedOddBool()
    {
        return invertedOddBool();
    }

    static inline OddBool getOddBool(const OddBoolUser& oddBoolUser)
    {
        return oddBoolUser.m_oddbool;
    }

private:
    OddBool m_oddbool;
};

#endif
