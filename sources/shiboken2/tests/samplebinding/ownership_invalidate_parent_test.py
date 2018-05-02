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

'''Tests for invalidating a parent of other objects.'''

import unittest

from sample import ObjectType, BlackBox


class InvalidateParentTest(unittest.TestCase):
    '''Tests for invalidating a parent of other objects.'''

    def testInvalidateParent(self):
        '''Invalidate parent should invalidate children'''
        parent = ObjectType.create()
        child1 = ObjectType(parent)
        child1.setObjectName("child1")
        child2 = ObjectType.create()
        child2.setObjectName("child2")
        child2.setParent(parent)
        grandchild1 = ObjectType(child1)
        grandchild1.setObjectName("grandchild1")
        grandchild2 = ObjectType.create()
        grandchild2.setObjectName("grandchild2")
        grandchild2.setParent(child2)
        bbox = BlackBox()

        bbox.keepObjectType(parent) # Should invalidate the parent

        self.assertRaises(RuntimeError, parent.objectName)
        # some children still valid they are wrapper classes
        self.assertEqual(child1.objectName(), "child1")
        self.assertRaises(RuntimeError, child2.objectName)
        self.assertEqual(grandchild1.objectName(), "grandchild1")
        self.assertRaises(RuntimeError, grandchild2.objectName)

if __name__ == '__main__':
    unittest.main()

