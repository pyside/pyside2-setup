#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

'''The BlackBox class has cases of ownership transference between C++ and Python.'''

import sys
import unittest

from sample import ObjectType, BlackBox

class BlackBoxTest(unittest.TestCase):
    '''The BlackBox class has cases of ownership transference between C++ and Python.'''

    def testOwnershipTransference(self):
        '''Ownership transference from Python to C++ and back again.'''
        o1 = ObjectType()
        o1.setObjectName('object1')
        o1_refcnt = sys.getrefcount(o1)
        o2 = ObjectType()
        o2.setObjectName('object2')
        o2_refcnt = sys.getrefcount(o2)
        bb = BlackBox()
        o1_ticket = bb.keepObjectType(o1)
        o2_ticket = bb.keepObjectType(o2)
        self.assertEqual(set(bb.objects()), set([o1, o2]))
        self.assertEqual(str(o1.objectName()), 'object1')
        self.assertEqual(str(o2.objectName()), 'object2')
        self.assertEqual(sys.getrefcount(o1), o1_refcnt + 1) # PySide give +1 ref to object with c++ ownership
        self.assertEqual(sys.getrefcount(o2), o2_refcnt + 1)
        o2 = bb.retrieveObjectType(o2_ticket)
        self.assertEqual(sys.getrefcount(o2), o2_refcnt)
        del bb
        self.assertRaises(RuntimeError, o1.objectName)
        self.assertEqual(str(o2.objectName()), 'object2')
        self.assertEqual(sys.getrefcount(o2), o2_refcnt)

    def testBlackBoxReleasingUnknownObjectType(self):
        '''Asks BlackBox to release an unknown ObjectType.'''
        o1 = ObjectType()
        o2 = ObjectType()
        bb = BlackBox()
        o1_ticket = bb.keepObjectType(o1)
        o3 = bb.retrieveObjectType(-5)
        self.assertEqual(o3, None)

    def testOwnershipTransferenceCppCreated(self):
        '''Ownership transference using a C++ created object.'''
        o1 = ObjectType.create()
        o1.setObjectName('object1')
        o1_refcnt = sys.getrefcount(o1)
        bb = BlackBox()
        o1_ticket = bb.keepObjectType(o1)
        self.assertRaises(RuntimeError, o1.objectName)

if __name__ == '__main__':
    unittest.main()

