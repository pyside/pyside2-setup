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

std::list<int>
ListUser::createIntList(int num)
{
    std::list<int> retval;
    for (int i = 0; i < num; ++i)
        retval.push_back(i);
    return retval;
}

int
ListUser::sumIntList(std::list<int> intList)
{
    int total = 0;
    for (std::list<int>::iterator iter = intList.begin(); iter != intList.end(); iter++)
        total += *iter;
    return total;
}

std::list<MinBool>
ListUser::createMinBoolList(MinBool mb1, MinBool mb2)
{
    std::list<MinBool> retval;
    retval.push_back(mb1);
    retval.push_back(mb2);
    return retval;
}

MinBool
ListUser::oredMinBoolList(std::list<MinBool> minBoolList)
{
    MinBool result(false);
    for (std::list<MinBool>::iterator iter = minBoolList.begin(); iter != minBoolList.end(); iter++)
        result |= *iter;
    return result;
}

std::list<Val>
ListUser::createValList(int num)
{
    std::list<Val> retval;
    for (int i = 0; i < num; ++i)
        retval.push_back(Val(i));
    return retval;
}

int
ListUser::sumValList(std::list<Val> valList)
{
    int total = 0;
    for (std::list<Val>::iterator iter = valList.begin(); iter != valList.end(); iter++)
        total += iter->valId();
    return total;
}

std::list<Obj*>
ListUser::createObjList(Obj* o1, Obj* o2)
{
    std::list<Obj*> retval;
    retval.push_back(o1);
    retval.push_back(o2);
    return retval;
}

int
ListUser::sumObjList(std::list<Obj*> objList)
{
    int total = 0;
    for (std::list<Obj*>::iterator iter = objList.begin(); iter != objList.end(); iter++)
        total += (*iter)->objId();
    return total;
}

std::list<std::list<int> >
ListUser::createListOfIntLists(int num)
{
    std::list<std::list<int> > retval;
    for (int i = 0; i < num; ++i)
        retval.push_back(createIntList(num));
    return retval;
}

int
ListUser::sumListOfIntLists(std::list<std::list<int> > intListList)
{
    int total = 0;
    for (std::list<std::list<int> >::iterator it0 = intListList.begin(); it0 != intListList.end(); it0++) {
        for (std::list<int>::iterator it1 = (*it0).begin(); it1 != (*it0).end(); it1++)
            total += *it1;
    }
    return total;
}

