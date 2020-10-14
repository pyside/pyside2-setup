/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef TYPESYSTYPEDEF_H
#define TYPESYSTYPEDEF_H

#include "libsamplemacros.h"

enum class LengthUnit { Millimeter, Inch };

template <class T, LengthUnit Unit>
class ValueWithUnit
{
    public:
    explicit ValueWithUnit(T value = {}) : m_value(value) {}

    T value() const { return m_value; }
    void setValue(const T &value) { m_value = value; }

private:
    T m_value;
};

class LIBSAMPLE_API ValueWithUnitUser
{
public:
    ValueWithUnitUser();

    static ValueWithUnit<double, LengthUnit::Millimeter> doubleInchToMillimeter(ValueWithUnit<double, LengthUnit::Inch>);
};

#endif // TYPESYSTYPEDEF_H
