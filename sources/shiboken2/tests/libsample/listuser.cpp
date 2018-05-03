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

#include <numeric>
#include <cstdlib>
#include "listuser.h"

using namespace std;

std::list<int>
ListUser::callCreateList()
{
    return createList();
}

std::list<int>
ListUser::createList()
{
    std::list<int> retval;
    for (int i = 0; i < 4; i++)
        retval.push_front(rand());
    return retval;
}

std::list<Complex>
ListUser::createComplexList(Complex cpx0, Complex cpx1)
{
    std::list<Complex> retval;
    retval.push_back(cpx0);
    retval.push_back(cpx1);
    return retval;
}

double
ListUser::sumList(std::list<int> vallist)
{
    return std::accumulate(vallist.begin(), vallist.end(), 0.0);
}

double
ListUser::sumList(std::list<double> vallist)
{
    return std::accumulate(vallist.begin(), vallist.end(), 0.0);
}

ListUser::ListOfSomething
ListUser::listOfPoints(const std::list<Point>& pointlist)
{
    return ListOfPoint;
}

ListUser::ListOfSomething
ListUser::listOfPoints(const std::list<PointF>& pointlist)
{
    return ListOfPointF;
}

void
ListUser::multiplyPointList(PointList& points, double multiplier)
{
    for(PointList::iterator piter = points.begin(); piter != points.end(); piter++) {
        (*piter)->setX((*piter)->x() * multiplier);
        (*piter)->setY((*piter)->y() * multiplier);
    }
}

