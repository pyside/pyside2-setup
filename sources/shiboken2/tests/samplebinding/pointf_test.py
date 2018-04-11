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

'''Test cases for PointF class'''

import unittest

from sample import PointF

class PointFTest(unittest.TestCase):
    '''Test case for PointF class, including operator overloads.'''

    def testConstructor(self):
        '''Test PointF class constructor.'''
        pt = PointF(5.0, 2.3)
        self.assertEqual(pt.x(), 5.0)
        self.assertEqual(pt.y(), 2.3)

    def testPlusOperator(self):
        '''Test PointF class + operator.'''
        pt1 = PointF(5.0, 2.3)
        pt2 = PointF(0.5, 3.2)
        self.assertEqual(pt1 + pt2, PointF(5.0 + 0.5, 2.3 + 3.2))

    def testEqualOperator(self):
        '''Test PointF class == operator.'''
        pt1 = PointF(5.0, 2.3)
        pt2 = PointF(5.0, 2.3)
        pt3 = PointF(0.5, 3.2)
        self.assertTrue(pt1 == pt1)
        self.assertTrue(pt1 == pt2)
        self.assertFalse(pt1 == pt3)

    def testModifiedMethod(self):
        pt1 = PointF(0.0, 0.0)
        pt2 = PointF(10.0, 10.0)
        expected = PointF((pt1.x() + pt2.x()) / 2.0, (pt1.y() + pt2.y()) / 2.0)
        self.assertEqual(pt1.midpoint(pt2), expected)

if __name__ == '__main__':
    unittest.main()
