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

#ifndef LISTUSER_H
#define LISTUSER_H

#include <list>
#include "complex.h"
#include "point.h"
#include "pointf.h"

#include "libsamplemacros.h"

class LIBSAMPLE_API ListUser
{
public:
    using PointList = std::list<Point *>;

    enum ListOfSomething {
        ListOfPoint,
        ListOfPointF
    };

    ListUser() {}
    ListUser(const ListUser& other) : m_lst(other.m_lst) {}
    virtual ~ListUser() {}

    virtual std::list<int> createList();
    std::list<int> callCreateList();

    static std::list<Complex> createComplexList(Complex cpx0, Complex cpx1);

    double sumList(std::list<int> vallist);
    double sumList(std::list<double> vallist);

    static ListOfSomething listOfPoints(const std::list<Point>& pointlist);
    static ListOfSomething listOfPoints(const std::list<PointF>& pointlist);

    static void multiplyPointList(PointList& points, double multiplier);

    inline void setList(std::list<int> lst) { m_lst = lst; }
    inline std::list<int> getList() { return m_lst; }

private:
    std::list<int> m_lst;
};

#endif // LISTUSER_H

