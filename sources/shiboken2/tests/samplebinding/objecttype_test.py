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

'''Tests ObjectType class of object-type with privates copy constructor and = operator.'''

import unittest
import sys

from sample import ObjectType, Str
import shiboken2 as shiboken


class ObjectTypeTest(unittest.TestCase):
    '''Test cases  ObjectType class of object-type with privates copy constructor and = operator.'''

    def testObjectTypeSetObjectNameWithStrVariable(self):
        '''ObjectType.setObjectName with Str variable as argument.'''
        s = Str('object name')
        o = ObjectType()
        o.setObjectName(s)
        self.assertEqual(str(o.objectName()), str(s))

    def testObjectTypeSetObjectNameWithStrInstantiation(self):
        '''ObjectType.setObjectName with Str instantiation as argument.'''
        s = 'object name'
        o = ObjectType()
        o.setObjectName(Str(s))
        self.assertEqual(str(o.objectName()), s)

    def testObjectTypeSetObjectNameWithPythonString(self):
        '''ObjectType.setObjectName with Python string as argument.'''
        o = ObjectType()
        o.setObjectName('object name')
        self.assertEqual(str(o.objectName()), 'object name')

    def testNullOverload(self):
        o = ObjectType()
        o.setObject(None)
        self.assertEqual(o.callId(), 0)
        o.setNullObject(None)
        self.assertEqual(o.callId(), 1)

    def testParentFromCpp(self):
        o = ObjectType()
        self.assertEqual(sys.getrefcount(o), 2)
        o.getCppParent().setObjectName('parent')
        self.assertEqual(sys.getrefcount(o), 3)
        o.getCppParent().setObjectName('parent')
        self.assertEqual(sys.getrefcount(o), 3)
        o.getCppParent().setObjectName('parent')
        self.assertEqual(sys.getrefcount(o), 3)
        o.getCppParent().setObjectName('parent')
        self.assertEqual(sys.getrefcount(o), 3)
        o.getCppParent().setObjectName('parent')
        self.assertEqual(sys.getrefcount(o), 3)
        o.destroyCppParent()
        self.assertEqual(sys.getrefcount(o), 2)

    def testNextInFocusChainCycle(self):
        parent = ObjectType()
        child = ObjectType(parent)
        next_focus = child.nextInFocusChain()

        shiboken.invalidate(parent)

    def testNextInFocusChainCycleList(self):
        '''As above but in for a list of objects'''
        parents = []
        children = []
        focus_chains = []
        for i in range(10):
            parent = ObjectType()
            child = ObjectType(parent)
            next_focus = child.nextInFocusChain()
            parents.append(parent)
            children.append(child)
            focus_chains.append(next_focus)

        shiboken.invalidate(parents)

    def testClassDecref(self):
        # Bug was that class PyTypeObject wasn't decrefed when instance died
        before = sys.getrefcount(ObjectType)

        for i in range(1000):
            obj = ObjectType()
            shiboken.delete(obj)

        after = sys.getrefcount(ObjectType)

        self.assertLess(abs(before - after), 5)

if __name__ == '__main__':
    unittest.main()
