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

#include <iostream>
#include "mapuser.h"

using namespace std;

std::map<std::string, std::pair<Complex, int> >
MapUser::callCreateMap()
{
    return createMap();
}


std::map<std::string, std::pair<Complex, int> >
MapUser::createMap()
{
    std::map<std::string, std::pair<Complex, int> > retval;

    std::pair<std::string, std::pair<Complex, int> >
            item0("zero", std::pair<Complex, int>(Complex(1.2, 3.4), 2));
    retval.insert(item0);

    std::pair<std::string, std::pair<Complex, int> >
            item1("one", std::pair<Complex, int>(Complex(5.6, 7.8), 3));
    retval.insert(item1);

    std::pair<std::string, std::pair<Complex, int> >
            item2("two", std::pair<Complex, int>(Complex(9.1, 2.3), 5));
    retval.insert(item2);

    return retval;
}

void
MapUser::showMap(std::map<std::string, int> mapping)
{
    std::map<std::string, int>::iterator it;
    cout << __FUNCTION__ << endl;
    for (it = mapping.begin() ; it != mapping.end(); it++)
        cout << (*it).first << " => " << (*it).second << endl;
}

std::map<int, std::list<std::list<double> > > MapUser::foo() const
{
    std::map<int, std::list<std::list<double> > > result;
    return result;
}
