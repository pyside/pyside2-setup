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

'''Tests for object reparenting.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()
import sys

from sample import ObjectType

class ExtObjectType(ObjectType):
    def __init__(self):
        ObjectType.__init__(self)


class ReparentingTest(unittest.TestCase):
    '''Tests for object reparenting.'''

    def testReparentedObjectTypeIdentity(self):
        '''Reparent children from one parent to another.'''
        object_list = []
        old_parent = ObjectType()
        new_parent = ObjectType()
        for i in range(3):
            obj = ObjectType()
            object_list.append(obj)
            obj.setParent(old_parent)
        for obj in object_list:
            obj.setParent(new_parent)
        for child in new_parent.children():
            self.assertTrue(child in object_list)

    def testReparentWithTheSameParent(self):
        '''Set the same parent twice to check if the ref continue the same'''
        obj = ObjectType()
        parent = ObjectType()
        self.assertEqual(sys.getrefcount(obj), 2)
        obj.setParent(parent)
        self.assertEqual(sys.getrefcount(obj), 3)
        obj.setParent(parent)
        self.assertEqual(sys.getrefcount(obj), 3)

    def testReparentedExtObjectType(self):
        '''Reparent children from one extended parent to another.'''
        object_list = []
        old_parent = ExtObjectType()
        new_parent = ExtObjectType()
        for i in range(3):
            obj = ExtObjectType()
            object_list.append(obj)
            obj.setParent(old_parent)
        for obj in object_list:
            obj.setParent(new_parent)
        for orig, child in zip(object_list, new_parent.children()):
            self.assertEqual(type(orig), type(child))

    def testReparentedObjectTypeIdentityWithParentsCreatedInCpp(self):
        '''Reparent children from one parent to another, both created in C++.'''
        object_list = []
        old_parent = ObjectType.create()
        new_parent = ObjectType.create()
        for i in range(3):
            obj = ObjectType()
            object_list.append(obj)
            obj.setParent(old_parent)
        for obj in object_list:
            obj.setParent(new_parent)
        for child in new_parent.children():
            self.assertTrue(child in object_list)

    def testReparentedObjectTypeIdentityWithChildrenCreatedInCpp(self):
        '''Reparent children created in C++ from one parent to another.'''
        object_list = []
        old_parent = ObjectType()
        new_parent = ObjectType()
        for i in range(3):
            obj = ObjectType.create()
            object_list.append(obj)
            obj.setParent(old_parent)
        for obj in object_list:
            obj.setParent(new_parent)
        for child in new_parent.children():
            self.assertTrue(child in object_list)

    def testReparentedObjectTypeIdentityWithParentsAndChildrenCreatedInCpp(self):
        '''Reparent children from one parent to another. Parents and children are created in C++.'''
        object_list = []
        old_parent = ObjectType.create()
        new_parent = ObjectType.create()
        for i in range(3):
            obj = ObjectType.create()
            object_list.append(obj)
            obj.setParent(old_parent)
        for obj in object_list:
            obj.setParent(new_parent)
        for child in new_parent.children():
            self.assertTrue(child in object_list)


if __name__ == '__main__':
    unittest.main()

