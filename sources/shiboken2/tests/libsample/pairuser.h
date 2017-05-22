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

#ifndef PAIRUSER_H
#define PAIRUSER_H

#include <utility>
#include "complex.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API PairUser
{
public:
    PairUser() {}
    virtual ~PairUser() {}

    virtual std::pair<int, int> createPair();
    std::pair<int, int> callCreatePair();
    static std::pair<Complex, Complex> createComplexPair(Complex cpx0, Complex cpx1);
    double sumPair(std::pair<int, double> pair);

    inline void setPair(std::pair<int, int> pair) { m_pair = pair; }
    inline std::pair<int, int> getPair() { return m_pair; }

private:
    std::pair<int, int> m_pair;
};
#endif // PAIRUSER_H

