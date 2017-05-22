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

'''Test case for overflowing C++ numeric types.'''

import sys
import unittest

from sample import *
from py3kcompat import IS_PY3K, long

class OverflowTest(unittest.TestCase):
    '''Test case for overflowing C++ numeric types.'''

    def testUnsignedInt(self):
        '''C++ function receives an unsigned int argument and raise OverflowError if the value is negative.'''
        val = 100
        self.assertEqual(doubleUnsignedInt(val), 2 * val)
        val *= -1
        self.assertRaises(OverflowError, doubleUnsignedInt, val)

    def testLongLong(self):
        '''C++ function receives an long long argument and raise OverflowError if the value is negative.'''
        val = 100
        self.assertEqual(doubleLongLong(val), 2 * val)
        val = long(100)
        self.assertEqual(doubleLongLong(val), 2 * val)
        val = (2 << 64) + 1
        self.assertRaises(OverflowError, doubleLongLong, val)

    def testUnsignedLongLong(self):
        '''C++ function receives an unsigned long long argument and raise OverflowError if the value is negative.'''
        val = 100
        self.assertEqual(doubleUnsignedLongLong(val), 2 * val)
        val = long(100)
        self.assertEqual(doubleUnsignedLongLong(val), 2 * val)
        val = -100
        self.assertRaises(OverflowError, doubleUnsignedLongLong, val)
        val = long(-200)
        self.assertRaises(OverflowError, doubleUnsignedLongLong, val)

    def testOverflow(self):
        '''Calls function with unsigned int parameter using an overflowing value.'''
        self.assertRaises(OverflowError, doubleUnsignedInt, 42415335332353253)
        doubleUnsignedInt(0xdeadbeef)

    def testShortOverflow(self):
        '''Calls function with short parameter using an overflowing value.'''
        doubleShort(-3)
        self.assertRaises(OverflowError, doubleShort, 0xFFFF*-1)
        self.assertRaises(OverflowError, doubleShort, 0xFFFF + 1)

    def testOverflowOnCtor(self):
        '''Calls object ctor with int parameter using overflowing values.'''
        self.assertRaises(OverflowError, Point, 42415335332353253, 42415335332353253)

if __name__ == '__main__':
    unittest.main()

