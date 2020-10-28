#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

from functools import reduce
import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from minimal import ListUser, Val, Obj


class ExtListUser(ListUser):
    def __init__(self):
        ListUser.__init__(self)

    def createIntList(self, num):
        return list(range(0, num * 2, 2))

    def sumIntList(self, intList):
        return sum(intList) * 2

    def createMinBoolList(self, mb1, mb2):
        return [not mb1, not mb2]

    def oredMinBoolList(self, minBoolList):
        return not reduce(lambda a, b: a|b, minBoolList)

    def createValList(self, num):
        return [Val(i) for i in range(0, num * 2, 2)]

    def sumValList(self, valList):
        return sum([val.valId() for val in valList]) * 2

    def createObjList(self, o1, o2):
        o1.setObjId(o1.objId() * 2)
        o2.setObjId(o2.objId() * 2)
        return [o1, o2]

    def sumObjList(self, objList):
        return sum([obj.objId() for obj in objList]) * 2

    def createListOfIntLists(self, num):
        return [self.createIntList(num)] * 4

    def sumListOfIntLists(self, intListList):
        return sum([sum(line) for line in intListList]) * 2


class IntListConversionTest(unittest.TestCase):

    def testCreateIntList(self):
        num = 4
        lu = ListUser()
        lst = lu.createIntList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), int)
        self.assertEqual(lst, list(range(num)))
        lst = lu.callCreateIntList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), int)
        self.assertEqual(lst, list(range(num)))

    def testCreateIntListFromExtendedClass(self):
        lu = ExtListUser()
        num = 4
        lst = lu.createIntList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), int)
        self.assertEqual(lst, list(range(0, num * 2, 2)))
        lst = lu.callCreateIntList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), int)
        self.assertEqual(lst, list(range(0, num * 2, 2)))

    def testSumIntList(self):
        lu = ListUser()
        lst = range(4)
        self.assertEqual(lu.sumIntList(lst), sum(lst))
        self.assertEqual(lu.callSumIntList(lst), sum(lst))

    def testSumIntListFromExtendedClass(self):
        lu = ExtListUser()
        lst = range(4)
        self.assertEqual(lu.sumIntList(lst), sum(lst) * 2)
        self.assertEqual(lu.callSumIntList(lst), sum(lst) * 2)


class MinBoolListConversionTest(unittest.TestCase):

    def testCreateMinBoolList(self):
        lu = ListUser()
        lst = lu.createMinBoolList(True, False)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), bool)
        self.assertEqual(lst, [True, False])

        lst = lu.callCreateMinBoolList(False, True)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), bool)
        self.assertEqual(lst, [False, True])

    def testCreateMinBoolListFromExtendedClass(self):
        lu = ExtListUser()
        lst = lu.createMinBoolList(True, False)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), bool)
        self.assertEqual(lst, [False, True])

        lst = lu.callCreateMinBoolList(False, True)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), bool)
        self.assertEqual(lst, [True, False])

    def testOredMinBoolList(self):
        lu = ListUser()
        lst = [False, False, True]
        self.assertTrue(lu.oredMinBoolList(lst))
        self.assertTrue(lu.callOredMinBoolList(lst))
        lst = [False, False, False]
        self.assertFalse(lu.oredMinBoolList(lst))
        self.assertFalse(lu.callOredMinBoolList(lst))

    def testOredMinBoolListFromExtendedClass(self):
        lu = ExtListUser()
        lst = [False, False, True]
        self.assertFalse(lu.oredMinBoolList(lst))
        self.assertFalse(lu.callOredMinBoolList(lst))
        lst = [False, False, False]
        self.assertTrue(lu.oredMinBoolList(lst))
        self.assertTrue(lu.callOredMinBoolList(lst))


class ValListConversionTest(unittest.TestCase):

    def testCreateValList(self):
        num = 4
        lu = ListUser()
        lst = lu.createValList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), Val)
        self.assertEqual([val.valId() for val in lst], list(range(num)))
        lst = lu.callCreateValList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), Val)
        self.assertEqual([val.valId() for val in lst], list(range(num)))

    def testCreateValListFromExtendedClass(self):
        lu = ExtListUser()
        num = 4
        lst = lu.createValList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), Val)
        self.assertEqual([val.valId() for val in lst], list(range(0, num * 2, 2)))
        lst = lu.callCreateValList(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), Val)
        self.assertEqual([val.valId() for val in lst], list(range(0, num * 2, 2)))

    def testSumValList(self):
        lu = ListUser()
        lst = [Val(i) for i in range(4)]
        self.assertEqual(lu.sumValList(lst), sum([val.valId() for val in lst]))
        self.assertEqual(lu.callSumValList(lst), sum([val.valId() for val in lst]))

    def testSumValListFromExtendedClass(self):
        lu = ExtListUser()
        lst = [Val(i) for i in range(4)]
        self.assertEqual(lu.sumValList(lst), sum([val.valId() for val in lst]) * 2)
        self.assertEqual(lu.callSumValList(lst), sum([val.valId() for val in lst]) * 2)


class ObjListConversionTest(unittest.TestCase):

    def testCreateObjList(self):
        o1 = Obj(1)
        o2 = Obj(2)
        lu = ListUser()
        lst = lu.createObjList(o1, o2)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), Obj)
        self.assertEqual(lst, [o1, o2])
        self.assertEqual([obj.objId() for obj in lst], [1, 2])

        lst = lu.callCreateObjList(o1, o2)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), Obj)
        self.assertEqual(lst, [o1, o2])
        self.assertEqual([obj.objId() for obj in lst], [1, 2])

    def testCreateObjListFromExtendedClass(self):
        o1 = Obj(1)
        o2 = Obj(2)
        lu = ExtListUser()
        lst = lu.createObjList(o1, o2)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), Obj)
        self.assertEqual(lst, [o1, o2])
        self.assertEqual([obj.objId() for obj in lst], [2, 4])

        lst = lu.callCreateObjList(o1, o2)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), 2)
        for i in lst:
            self.assertEqual(type(i), Obj)
        self.assertEqual(lst, [o1, o2])
        self.assertEqual([obj.objId() for obj in lst], [4, 8])

    def testSumObjList(self):
        lu = ListUser()
        lst = [Obj(i) for i in list(range(4))]
        self.assertEqual(lu.sumObjList(lst), sum([obj.objId() for obj in lst]))
        self.assertEqual(lu.callSumObjList(lst), sum([obj.objId() for obj in lst]))

    def testSumObjListFromExtendedClass(self):
        lu = ExtListUser()
        lst = [Obj(i) for i in list(range(4))]
        self.assertEqual(lu.sumObjList(lst), sum([obj.objId() for obj in lst]) * 2)
        self.assertEqual(lu.callSumObjList(lst), sum([obj.objId() for obj in lst]) * 2)


class ListOfIntListConversionTest(unittest.TestCase):

    def testCreateListOfIntLists(self):
        num = 4
        lu = ListUser()
        lst = lu.createListOfIntLists(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), list)
            self.assertEqual(i, list(range(num)))
            for j in i:
                self.assertEqual(type(j), int)
        self.assertEqual(lst, [list(range(num))] * 4)

    def testCreateListOfIntListsFromExtendedClass(self):
        num = 4
        lu = ExtListUser()
        lst = lu.createListOfIntLists(num)
        self.assertEqual(type(lst), list)
        self.assertEqual(len(lst), num)
        for i in lst:
            self.assertEqual(type(i), list)
            self.assertEqual(i, list(range(0, num * 2, 2)))
            for j in i:
                self.assertEqual(type(j), int)
        self.assertEqual(lst, [list(range(0, num * 2, 2))] * 4)

    def testSumListIntLists(self):
        lu = ListUser()
        lst = [range(4)] * 4
        self.assertEqual(lu.sumListOfIntLists(lst), sum([sum(line) for line in [range(4)] * 4]))
        self.assertEqual(lu.callSumListOfIntLists(lst), sum([sum(line) for line in [range(4)] * 4]))

    def testSumListOfIntListsFromExtendedClass(self):
        lu = ExtListUser()
        lst = [range(4)] * 4
        self.assertEqual(lu.sumListOfIntLists(lst), sum([sum(line) for line in [range(4)] * 4]) * 2)
        self.assertEqual(lu.callSumListOfIntLists(lst), sum([sum(line) for line in [range(4)] * 4]) * 2)


if __name__ == '__main__':
    unittest.main()

