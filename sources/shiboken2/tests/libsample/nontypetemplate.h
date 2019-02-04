/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef NONTYPETEMPLATE_H
#define NONTYPETEMPLATE_H

#include "libsamplemacros.h"

#include <algorithm>
#include <numeric>

template <int Size> class IntArray
{
public:
    explicit IntArray(const int *data) { std::copy(data, data + Size, m_array); }
    explicit IntArray(int v) { std::fill(m_array, m_array + Size, v); }

    int sum() const { return std::accumulate(m_array, m_array + Size, int(0)); }

private:
    int m_array[Size];
};

typedef IntArray<2> IntArray2;
typedef IntArray<3> IntArray3;

#endif // NONTYPETEMPLATE_H
