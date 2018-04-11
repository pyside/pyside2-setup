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

'''Tests for destroying the parent'''

import sys
import unittest

from sample import ObjectType


class DeleteParentTest(unittest.TestCase):
    '''Test case for deleting a parent object'''

    def testParentDestructor(self):
        '''Delete parent object should invalidate child'''
        parent = ObjectType()
        child = ObjectType()
        child.setParent(parent)

        refcount_before = sys.getrefcount(child)

        del parent
        self.assertRaises(RuntimeError, child.objectName)
        self.assertEqual(sys.getrefcount(child), refcount_before-1)

    def testParentDestructorMultipleChildren(self):
        '''Delete parent object should invalidate all children'''
        parent = ObjectType()
        children = [ObjectType() for _ in range(10)]

        for child in children:
            child.setParent(parent)

        del parent
        for i, child in enumerate(children):
            self.assertRaises(RuntimeError, child.objectName)
            self.assertEqual(sys.getrefcount(child), 4)

    def testRecursiveParentDelete(self):
        '''Delete parent should invalidate grandchildren'''
        parent = ObjectType()
        child = ObjectType(parent)
        grandchild = ObjectType(child)

        del parent
        self.assertRaises(RuntimeError, child.objectName)
        self.assertEqual(sys.getrefcount(child), 2)
        self.assertRaises(RuntimeError, grandchild.objectName)
        self.assertEqual(sys.getrefcount(grandchild), 2)


if __name__ == '__main__':
    unittest.main()
