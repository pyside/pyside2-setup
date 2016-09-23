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

#ifndef MINBOOL_H
#define MINBOOL_H

#include "libminimalmacros.h"

class LIBMINIMAL_API MinBool
{
public:
    inline explicit MinBool(bool b) : m_value(b) {}
    bool value() const { return m_value; }
    inline MinBool operator!() const { return MinBool(!m_value); }
    inline MinBool& operator|=(const MinBool& other) {
        m_value = m_value | other.m_value;
        return *this;
    }
private:
    bool m_value;
};

inline bool operator==(MinBool b1, bool b2) { return (!b1).value() == !b2; }
inline bool operator==(bool b1, MinBool b2) { return (!b1) == (!b2).value(); }
inline bool operator==(MinBool b1, MinBool b2) { return (!b1).value() == (!b2).value(); }
inline bool operator!=(MinBool b1, bool b2) { return (!b1).value() != !b2; }
inline bool operator!=(bool b1, MinBool b2) { return (!b1) != (!b2).value(); }
inline bool operator!=(MinBool b1, MinBool b2) { return (!b1).value() != (!b2).value(); }

class LIBMINIMAL_API MinBoolUser
{
public:
    MinBoolUser() : m_minbool(MinBool(false)) {}
    virtual ~MinBoolUser() {}
    inline MinBool minBool() { return m_minbool; }
    inline void setMinBool(MinBool minBool) { m_minbool = minBool; }
    virtual MinBool invertedMinBool() { return !m_minbool; }
    inline MinBool callInvertedMinBool() { return invertedMinBool(); }
private:
    MinBool m_minbool;
};

#endif
