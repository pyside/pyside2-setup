#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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

'''Tests cases for ObjectTypeLayout class.'''

import sys
import unittest

from sample import *


class ObjectTypeLayoutTest(unittest.TestCase):
    '''Test cases for ObjectTypeLayout class.'''

    def testOwnershipOverride(self):
        l = ObjectTypeLayout()

        o1 = ObjectType(l)
        o1.setObjectName('o1')

        self.assertEqual(sys.getrefcount(o1), 3)
        l.takeChild('o1')
        self.assertEqual(sys.getrefcount(o1), 2)


    def testSetNullLayout(self):
        '''ObjectType.setLayout(0).'''
        o2 = ObjectType()
        o2.setLayout(None)

    def testSetNullLayoutToObjectTypeCreatedInCpp(self):
        '''ObjectType.setLayout(0) to object created in C++.'''
        o1 = ObjectType.create()
        o1.setLayout(None)

    def testObjectTypeLayout(self):
        '''ObjectType.setLayout.'''
        p1 = ObjectType()
        c1 = ObjectType()
        c2 = ObjectType()
        c3 = ObjectType()
        layout = ObjectTypeLayout()
        layout.addObject(c1)
        layout.addObject(c2)
        layout.addObject(c3)
        self.assertEqual(c1.parent(), None)
        self.assertEqual(c2.parent(), None)
        self.assertEqual(c3.parent(), None)

        p1.setLayout(layout)
        del p1 # This must kill c1, c2 and c3

        self.assertRaises(RuntimeError, c1.objectName)
        self.assertRaises(RuntimeError, c2.objectName)
        self.assertRaises(RuntimeError, c3.objectName)
        self.assertRaises(RuntimeError, layout.objectName)

    def testObjectTypeLayoutWithObjectsCreatedInCpp(self):
        '''ObjectType.setLayout with objects created in C++.'''
        p1 = ObjectType.create()
        c1 = ObjectType.create()
        c2 = ObjectType.create()
        c3 = ObjectType.create()
        layout = ObjectTypeLayout()
        layout.addObject(c1)
        layout.addObject(c2)
        layout.addObject(c3)
        self.assertEqual(c1.parent(), None)
        self.assertEqual(c2.parent(), None)
        self.assertEqual(c3.parent(), None)

        p1.setLayout(layout)
        del p1 # This must kill c1, c2 and c3

        self.assertRaises(RuntimeError, c1.objectName)
        self.assertRaises(RuntimeError, c2.objectName)
        self.assertRaises(RuntimeError, c3.objectName)
        self.assertRaises(RuntimeError, layout.objectName)

    def testObjectTypeLayoutTransference(self):
        '''Transfer a layout from one ObjectType to another, so that all the items in the layout get reparented.'''
        p1 = ObjectType()
        p2 = ObjectType()
        c1 = ObjectType()
        c2 = ObjectType()

        layout = ObjectTypeLayout()
        layout.addObject(c1)
        layout.addObject(c2)

        p1.setLayout(layout)

        self.assertEqual(len(p2.children()), 0)
        self.assertEqual(c1.parent(), p1)
        self.assertEqual(c2.parent(), p1)
        self.assertEqual(set(p1.children()), set([c1, c2, layout]))

        p2.setLayout(layout)

        self.assertEqual(len(p1.children()), 0)
        self.assertEqual(c1.parent(), p2)
        self.assertEqual(c2.parent(), p2)
        self.assertEqual(set(p2.children()), set([c1, c2, layout]))

    def testObjectTypeLayoutInsideAnotherLayout(self):
        '''Adds one ObjectTypeLayout to another and sets the parent to an ObjectType.'''
        p1 = ObjectType()

        l1 = ObjectTypeLayout()
        c1 = ObjectType()
        l1.addObject(c1)
        c2 = ObjectType()
        l1.addObject(c2)

        l2 = ObjectTypeLayout()
        c3 = ObjectType()
        l2.addObject(c3)
        c4 = ObjectType()
        l2.addObject(c4)

        l1.addObject(l2)

        p1.setLayout(l1)

        self.assertEqual(c1.parent(), p1)
        self.assertEqual(c2.parent(), p1)
        self.assertEqual(c3.parent(), p1)
        self.assertEqual(c4.parent(), p1)
        self.assertEqual(l1.parent(), p1)
        self.assertEqual(l2.parent(), l1)

        del p1

        self.assertRaises(RuntimeError, c1.objectName)
        self.assertRaises(RuntimeError, c2.objectName)
        self.assertRaises(RuntimeError, c3.objectName)
        self.assertRaises(RuntimeError, c4.objectName)
        self.assertRaises(RuntimeError, l1.objectName)
        self.assertRaises(RuntimeError, l2.objectName)

    def testObjectTypeLayoutInsideAnotherLayoutAndEveryoneCreatedInCpp(self):
        '''Adds one ObjectTypeLayout to another and sets the parent to an ObjectType. All the objects are created in C++.'''
        p1 = ObjectType.create()

        l1 = ObjectTypeLayout.create()
        c1 = ObjectType.create()
        l1.addObject(c1)
        c2 = ObjectType.create()
        l1.addObject(c2)

        l2 = ObjectTypeLayout.create()
        c3 = ObjectType.create()
        l2.addObject(c3)
        c4 = ObjectType.create()
        l2.addObject(c4)

        l1.addObject(l2)

        p1.setLayout(l1)

        self.assertEqual(c1.parent(), p1)
        self.assertEqual(c2.parent(), p1)
        self.assertEqual(c3.parent(), p1)
        self.assertEqual(c4.parent(), p1)
        self.assertEqual(l1.parent(), p1)
        self.assertEqual(l2.parent(), l1)

        del p1

        self.assertRaises(RuntimeError, c1.objectName)
        self.assertRaises(RuntimeError, c2.objectName)
        self.assertRaises(RuntimeError, c3.objectName)
        self.assertRaises(RuntimeError, c4.objectName)
        self.assertRaises(RuntimeError, l1.objectName)
        self.assertRaises(RuntimeError, l2.objectName)

    def testTransferNestedLayoutsBetweenObjects(self):
        '''Adds one ObjectTypeLayout to another, sets the parent to an ObjectType and then transfer it to another object.'''
        p1 = ObjectType()
        p2 = ObjectType()

        l1 = ObjectTypeLayout()
        c1 = ObjectType()
        l1.addObject(c1)
        c2 = ObjectType()
        l1.addObject(c2)

        l2 = ObjectTypeLayout()
        c3 = ObjectType()
        l2.addObject(c3)
        c4 = ObjectType()
        l2.addObject(c4)

        l1.addObject(l2)

        p1.setLayout(l1)

        self.assertEqual(c1.parent(), p1)
        self.assertEqual(c2.parent(), p1)
        self.assertEqual(c3.parent(), p1)
        self.assertEqual(c4.parent(), p1)
        self.assertEqual(l1.parent(), p1)
        self.assertEqual(l2.parent(), l1)

        p2.setLayout(l1)
        del p1

        self.assertEqual(c1.parent(), p2)
        self.assertEqual(c2.parent(), p2)
        self.assertEqual(c3.parent(), p2)
        self.assertEqual(c4.parent(), p2)
        self.assertEqual(l1.parent(), p2)
        self.assertEqual(l2.parent(), l1)

        del p2

        self.assertRaises(RuntimeError, c1.objectName)
        self.assertRaises(RuntimeError, c2.objectName)
        self.assertRaises(RuntimeError, c3.objectName)
        self.assertRaises(RuntimeError, c4.objectName)
        self.assertRaises(RuntimeError, l1.objectName)
        self.assertRaises(RuntimeError, l2.objectName)

    def testTransferNestedLayoutsBetweenObjectsAndEveryoneCreatedInCpp(self):
        '''Adds one ObjectTypeLayout to another, sets the parent to an ObjectType and then transfer it to another object.
        All the objects are created in C++.'''
        p1 = ObjectType.create()
        p2 = ObjectType.create()

        l1 = ObjectTypeLayout.create()
        c1 = ObjectType.create()
        l1.addObject(c1)
        c2 = ObjectType.create()
        l1.addObject(c2)

        l2 = ObjectTypeLayout.create()
        c3 = ObjectType.create()
        l2.addObject(c3)
        c4 = ObjectType.create()
        l2.addObject(c4)

        l1.addObject(l2)

        p1.setLayout(l1)

        self.assertEqual(c1.parent(), p1)
        self.assertEqual(c2.parent(), p1)
        self.assertEqual(c3.parent(), p1)
        self.assertEqual(c4.parent(), p1)
        self.assertEqual(l1.parent(), p1)
        self.assertEqual(l2.parent(), l1)

        p2.setLayout(l1)
        del p1

        self.assertEqual(c1.parent(), p2)
        self.assertEqual(c2.parent(), p2)
        self.assertEqual(c3.parent(), p2)
        self.assertEqual(c4.parent(), p2)
        self.assertEqual(l1.parent(), p2)
        self.assertEqual(l2.parent(), l1)

        del p2

        self.assertRaises(RuntimeError, c1.objectName)
        self.assertRaises(RuntimeError, c2.objectName)
        self.assertRaises(RuntimeError, c3.objectName)
        self.assertRaises(RuntimeError, c4.objectName)
        self.assertRaises(RuntimeError, l1.objectName)
        self.assertRaises(RuntimeError, l2.objectName)

if __name__ == '__main__':
    unittest.main()

