#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# This file is part of the Shiboken Python Bindings Generator project.
#
# Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
#
# Contact: PySide team <contact@pyside.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation. Please
# review the following information to ensure the GNU Lesser General
# Public License version 2.1 requirements will be met:
# http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
# #
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA

'''The KinderGarten class has cases of ownership transference between C++ and Python.'''

import sys
import unittest

from sample import ObjectType, KinderGarten

class KinderGartenTest(unittest.TestCase):
    '''The KinderGarten class has cases of ownership transference between C++ and Python.'''

    def testOwnershipTransference(self):
        '''Ownership transference from Python to C++ and back again.'''
        o1 = ObjectType()
        o1.setObjectName('object1')
        o1_refcnt = sys.getrefcount(o1)
        o2 = ObjectType()
        o2.setObjectName('object2')
        o2_refcnt = sys.getrefcount(o2)
        kg = KinderGarten()
        kg.addChild(o1)
        kg.addChild(o2)
        self.assertEqual(kg.children(), [o1, o2])
        self.assertEqual(str(o1.objectName()), 'object1')
        self.assertEqual(str(o2.objectName()), 'object2')
        self.assertEqual(sys.getrefcount(o1), o1_refcnt)
        self.assertEqual(sys.getrefcount(o2), o2_refcnt)
        o2 = kg.releaseChild(o2)
        self.assertEqual(sys.getrefcount(o2), o2_refcnt)
        del kg
        self.assertNotEqual(str(o1.objectName()), 'object1')
        self.assertEqual(str(o2.objectName()), 'object2')
        self.assertEqual(sys.getrefcount(o2), o2_refcnt)

    def testKinderGartenReleasingUnknownChild(self):
        '''Asks KinderGarten to release an unknown child object.'''
        o1 = ObjectType()
        o2 = ObjectType()
        kg = KinderGarten()
        kg.addChild(o1)
        o3 = kg.releaseChild(o2)
        self.assertEqual(o3, None)

if __name__ == '__main__':
    unittest.main()

