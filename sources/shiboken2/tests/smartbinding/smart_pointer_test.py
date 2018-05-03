#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

import unittest
from copy import copy
from smart import Obj, Registry, Integer

def objCount():
    return Registry.getInstance().countObjects()

def integerCount():
    return Registry.getInstance().countIntegers()

class SmartPointerTests(unittest.TestCase):
    def testObjSmartPointer(self):
        # Uncomment to see more debug info about creation of objects and ref counts.
        # Registry.getInstance().setShouldPrint(True)

        # Create Obj.
        o = Obj()
        self.assertEqual(objCount(), 1)

        # Create a shared pointer to an Obj together with an Obj.
        ptrToObj = o.giveSharedPtrToObj()
        self.assertEqual(objCount(), 2)

        # Delete the old Obj.
        o = None
        self.assertEqual(objCount(), 1)

        # Get a wrapper to the Obj inside of the shared pointer, object count should not change.
        obj = ptrToObj.data()
        self.assertEqual(objCount(), 1)
        obj.m_integer = 50
        self.assertEqual(obj.m_integer, 50)

        # Set and get a member value via shared pointer (like operator->).
        ptrToObj.m_integer = 100
        self.assertEqual(ptrToObj.m_integer, 100)

        # Get inner PyObject via shared pointer (like operator->) and set value in it.
        ptrToObj.m_internalInteger.m_int = 200
        self.assertEqual(ptrToObj.m_internalInteger.m_int, 200)

        # Pass smart pointer as argument to a method, return value is the value of m_integer of
        # passed Obj inside the smart pointer.
        result = ptrToObj.takeSharedPtrToObj(ptrToObj)
        self.assertEqual(result, 100)

        # Pass an Integer as an argument that returns itself.
        result = ptrToObj.takeInteger(ptrToObj.m_internalInteger)
        self.assertEqual(integerCount(), 2)
        result = None
        self.assertEqual(integerCount(), 1)

        # Make a copy of the shared pointer, object count should not change.
        ptrToObj2 = copy(ptrToObj)
        self.assertEqual(objCount(), 1)

        # Delete the first shared pointer, object count should not change because the second
        # one still has a reference.
        del ptrToObj
        self.assertEqual(objCount(), 1)

        # Delete the second smart pointer, object should be deleted.
        del ptrToObj2
        self.assertEqual(objCount(), 0)
        self.assertEqual(integerCount(), 0)

    def testIntegerSmartPointer(self):
        # Uncomment to see more debug info about creation of objects and ref counts.
        # Registry.getInstance().setShouldPrint(True)

        # Create Obj.
        o = Obj()
        self.assertEqual(objCount(), 1)

        # Create a shared pointer to an Integer together with an Integer.
        ptrToInteger = o.giveSharedPtrToInteger()
        self.assertEqual(objCount(), 1)
        self.assertEqual(integerCount(), 2)

        # Get a wrapper to the Integer inside of the shared pointer, integer count should not
        # change.
        integer = ptrToInteger.data()
        self.assertEqual(integerCount(), 2)
        integer.m_int = 50
        self.assertEqual(integer.m_int, 50)

        # Set and get a member value via shared pointer (like operator->).
        ptrToInteger.m_int = 100
        self.assertEqual(ptrToInteger.m_int, 100)

        # Pass smart pointer as argument to a method, return value is the value of m_int of
        # passed Integer inside the smart pointer.
        result = o.takeSharedPtrToInteger(ptrToInteger)
        self.assertEqual(result, 100)

        # Make a copy of the shared pointer, integer count should not change.
        ptrToInteger2 = copy(ptrToInteger)
        self.assertEqual(integerCount(), 2)

        # Delete the first shared pointer, integer count should not change because the second
        # one still has a reference.
        del ptrToInteger
        self.assertEqual(integerCount(), 2)

        # Delete the second smart pointer, integer should be deleted.
        del ptrToInteger2
        self.assertEqual(objCount(), 1)
        self.assertEqual(integerCount(), 1)

        # Delete the original object which was used to create the integer.
        del o
        self.assertEqual(objCount(), 0)
        self.assertEqual(integerCount(), 0)

if __name__ == '__main__':
    unittest.main()
