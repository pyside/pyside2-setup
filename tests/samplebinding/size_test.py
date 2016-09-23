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

'''Test cases for operator overloads on Size class'''

import unittest

from sample import Size

class PointTest(unittest.TestCase):
    '''Test case for Size class, including operator overloads.'''

    def testConstructor(self):
        '''Test Size class constructor.'''
        width, height = (5.0, 2.3)
        size = Size(width, height)
        self.assertEqual(size.width(), width)
        self.assertEqual(size.height(), height)
        self.assertEqual(size.calculateArea(), width * height)

    def testCopyConstructor(self):
        '''Test Size class copy constructor.'''
        width, height = (5.0, 2.3)
        s1 = Size(width, height)
        s2 = Size(s1)
        self.assertFalse(s1 is s2)
        self.assertEqual(s1, s2)

    def testPlusOperator(self):
        '''Test Size class + operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(0.5, 3.2)
        self.assertEqual(s1 + s2, Size(5.0 + 0.5, 2.3 + 3.2))

    def testEqualOperator(self):
        '''Test Size class == operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertTrue(s1 == s1)
        self.assertTrue(s1 == s2)
        self.assertFalse(s1 == s3)

    def testNotEqualOperator(self):
        '''Test Size class != operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertFalse(s1 != s1)
        self.assertFalse(s1 != s2)
        self.assertTrue(s1 != s3)

    def testMinorEqualOperator(self):
        '''Test Size class <= operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertTrue(s1 <= s1)
        self.assertTrue(s1 <= s2)
        self.assertTrue(s3 <= s1)
        self.assertFalse(s1 <= s3)

    def testMinorOperator(self):
        '''Test Size class < operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(0.5, 3.2)
        self.assertFalse(s1 < s1)
        self.assertFalse(s1 < s2)
        self.assertTrue(s2 < s1)

    def testMajorEqualOperator(self):
        '''Test Size class >= operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertTrue(s1 >= s1)
        self.assertTrue(s1 >= s2)
        self.assertTrue(s1 >= s3)
        self.assertFalse(s3 >= s1)

    def testMajorOperator(self):
        '''Test Size class > operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(0.5, 3.2)
        self.assertFalse(s1 > s1)
        self.assertTrue(s1 > s2)
        self.assertFalse(s2 > s1)

if __name__ == '__main__':
    unittest.main()

