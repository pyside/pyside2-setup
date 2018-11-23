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

'''Test cases for libsample's Point multiply operator defined in libother module.'''

import unittest

from sample import Point
from other import Number

class PointOperationsWithNumber(unittest.TestCase):
    '''Test cases for libsample's Point multiply operator defined in libother module.'''

    def testPointTimesInt(self):
        '''sample.Point * int'''
        pt1 = Point(2, 7)
        num = 3
        pt2 = Point(pt1.x() * num, pt1.y() * num)
        self.assertEqual(pt1 * num, pt2)

    def testIntTimesPoint(self):
        '''int * sample.Point'''
        pt1 = Point(2, 7)
        num = 3
        pt2 = Point(pt1.x() * num, pt1.y() * num)
        self.assertEqual(num * pt1, pt2)

    def testPointTimesNumber(self):
        '''sample.Point * other.Number'''
        pt = Point(2, 7)
        num = Number(11)
        self.assertEqual(pt * num.value(), pt * 11)

if __name__ == '__main__':
    unittest.main()

