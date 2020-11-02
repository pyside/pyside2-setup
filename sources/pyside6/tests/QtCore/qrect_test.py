#!/usr/bin/python

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

'''Test cases for QRect'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QPoint, QRect, QRectF

class RectConstructor(unittest.TestCase):

    def testDefault(self):
        #QRect()
        obj = QRect()

        self.assertTrue(obj.isNull())

    def testConstructorQPoint(self):
        topLeft = QPoint(3, 0)
        bottomRight = QPoint(0, 3)

        rect1 = QRect(topLeft, bottomRight)
        rect2 = QRect(topLeft, bottomRight)

        self.assertEqual(rect1, rect2)

class RectOperator(unittest.TestCase):
    '''Test case for QRect operators'''

    def testEqual(self):
        '''QRect == QRect
        Note: operator == must be working as it's the main check
        for correctness'''
        rect1 = QRect()
        rect2 = QRect()
        self.assertEqual(rect1, rect2)

        rect1 = QRect(0, 4, 100, 300)
        rect2 = QRect(0, 4, 100, 300)
        self.assertEqual(rect1, rect2)

    def testNullRectIntersection(self):
        #QRect & QRect for null rects
        rect1 = QRect()
        rect2 = QRect()
        rect3 = rect1 & rect2
        self.assertEqual(rect3, rect1)
        self.assertEqual(rect3, rect2)

    def testNoIntersect(self):
        '''QRect & QRect for non-intersecting QRects
        Non-intersecting QRects return a 'null' QRect for operator &'''
        rect1 = QRect(10, 10, 5, 5)
        rect2 = QRect(20, 20, 5, 5)
        rect3 = rect1 & rect2
        self.assertEqual(rect3, QRect())

    def testIntersectPartial(self):
        #QRect & QRect for partial intersections
        rect1 = QRect(10, 10, 10, 10)
        rect2 = QRect(15, 15, 10, 10)
        rect3 = rect1 & rect2
        self.assertEqual(rect3, QRect(15, 15, 5, 5))

    def testIntersetEnclosed(self):
        #QRect & QRect for a qrect inside another
        rect1 = QRect(10, 10, 20, 20)
        rect2 = QRect(15, 15, 5, 5)
        rect3 = rect1 & rect2
        self.assertEqual(rect3, rect2)

    def testNullRectIntersectBounding(self):
        #QRect | QRect for null rects
        rect1 = QRect()
        rect2 = QRect()
        rect3 = rect1 & rect2
        self.assertEqual(rect3, rect1)
        self.assertEqual(rect3, rect2)

    def testNoIntersectBounding(self):
        '''QRect | QRect for non-intersecting QRects
        Non-intersecting QRects return a greater QRect for operator |'''
        rect1 = QRect(10, 10, 5, 5)
        rect2 = QRect(20, 20, 5, 5)
        rect3 = rect1 | rect2
        self.assertEqual(rect3, QRect(10, 10, 15, 15))

    def testBoundingPartialIntersection(self):
        #QRect | QRect for partial intersections
        rect1 = QRect(10, 10, 10, 10)
        rect2 = QRect(15, 15, 10, 10)
        rect3 = rect1 | rect2
        self.assertEqual(rect3, QRect(10, 10, 15, 15))

    def testBoundingEnclosed(self):
        #QRect | QRect for a qrect inside another
        rect1 = QRect(10, 10, 20, 20)
        rect2 = QRect(15, 15, 5, 5)
        rect3 = rect1 | rect2
        self.assertEqual(rect3, rect1)

    def testGetCoordsAndRect(self):
        rect1 = QRect(1, 2, 3, 4)
        self.assertEqual(rect1.getRect(), (1, 2, 3, 4))
        self.assertEqual(rect1.getCoords(), (1, 2, 3, 5))

        rect1 = QRectF(1, 2, 3, 4)
        self.assertEqual(rect1.getRect(), (1, 2, 3, 4))
        self.assertEqual(rect1.getCoords(), (1, 2, 4, 6))



if __name__ == '__main__':
    unittest.main()
