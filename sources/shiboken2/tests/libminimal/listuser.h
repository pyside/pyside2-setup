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

#ifndef LISTUSER_H
#define LISTUSER_H

#include <list>
#include "obj.h"
#include "val.h"
#include "minbool.h"

#include "libminimalmacros.h"

struct LIBMINIMAL_API ListUser
{
    virtual ~ListUser() {}

    // List of C++ primitive type items
    virtual std::list<int> createIntList(int num);
    std::list<int> callCreateIntList(int num) { return createIntList(num); }
    virtual int sumIntList(std::list<int> intList);
    int callSumIntList(std::list<int> intList) { return sumIntList(intList); }

    // List of C++ MinBool objects used as primitives in Python
    virtual std::list<MinBool> createMinBoolList(MinBool mb1, MinBool mb2);
    std::list<MinBool> callCreateMinBoolList(MinBool mb1, MinBool mb2) { return createMinBoolList(mb1, mb2); }
    virtual MinBool oredMinBoolList(std::list<MinBool> minBoolList);
    MinBool callOredMinBoolList(std::list<MinBool> minBoolList) { return oredMinBoolList(minBoolList); }

    // List of C++ value types
    virtual std::list<Val> createValList(int num);
    std::list<Val> callCreateValList(int num) { return createValList(num); }
    virtual int sumValList(std::list<Val> valList);
    int callSumValList(std::list<Val> valList) { return sumValList(valList); }

    // List of C++ object types
    virtual std::list<Obj*> createObjList(Obj* o1, Obj* o2);
    std::list<Obj*> callCreateObjList(Obj* o1, Obj* o2) { return createObjList(o1, o2); }
    virtual int sumObjList(std::list<Obj*> objList);
    int callSumObjList(std::list<Obj*> objList) { return sumObjList(objList); }

    // List of lists of C++ primitive type items
    virtual std::list<std::list<int> > createListOfIntLists(int num);
    std::list<std::list<int> > callCreateListOfIntLists(int num) { return createListOfIntLists(num); }
    virtual int sumListOfIntLists(std::list<std::list<int> > intListList);
    int callSumListOfIntLists(std::list<std::list<int> > intListList) { return sumListOfIntLists(intListList); }
};

#endif // LISTUSER_H

