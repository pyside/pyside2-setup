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

'''Test case for inplicit converting C++ numeric types.'''

import unittest
import sys
import sample
from py3kcompat import IS_PY3K, l, long

# Hardcode the limits of the underlying C-types depending on architecture and memory
# model (taking MSVC using LLP64 into account).
cIntMin = -2147483648
cIntMax = 2147483647
cLongMin = cIntMin
cLongMax = cIntMax
maxRepresentableInt = sys.maxsize if IS_PY3K else sys.maxint
is64bitArchitecture = maxRepresentableInt > 2**32
if is64bitArchitecture and sys.platform != 'win32':
    cLongMin = -9223372036854775808
    cLongMax = 9223372036854775807

class NumericTester(unittest.TestCase):
    '''Helper class for numeric comparison testing'''

    def check_value(self, source, expected, callback, desired_type=None):
        result = callback(source)
        self.assertEqual(result, expected)

        if desired_type:
            self.assertEqual(type(result), desired_type)


class FloatImplicitConvert(NumericTester):
    '''Test case for implicit converting C++ numeric types.'''

    def testFloatAsInt(self):
        '''Float as Int'''
        self.check_value(3.14, 3, sample.acceptInt, int)
        self.assertRaises(OverflowError, sample.acceptInt, cIntMax + 400)

    def testFloatAsLong(self):
        '''Float as Long'''
        #C++ longs are python ints for us
        self.check_value(3.14, 3, sample.acceptLong, int)
        self.assertRaises(OverflowError, sample.acceptLong, cLongMax + 400)

    def testFloatAsUInt(self):
        '''Float as unsigned Int'''
        self.check_value(3.14, 3, sample.acceptUInt, long)
        self.assertRaises(OverflowError, sample.acceptUInt, -3.14)

    def testFloatAsULong(self):
        '''Float as unsigned Long'''
        #FIXME Breaking with SystemError "bad argument to internal function"
        self.check_value(3.14, 3, sample.acceptULong, long)
        self.assertRaises(OverflowError, sample.acceptULong, -3.14)

    def testFloatAsDouble(self):
        '''Float as double'''
        self.check_value(3.14, 3.14, sample.acceptDouble, float)


class IntImplicitConvert(NumericTester):
    '''Test case for implicit converting C++ numeric types.'''

    def testIntAsInt(self):
        '''Int as Int'''
        self.check_value(3, 3, sample.acceptInt, int)

    def testIntAsLong(self):
        '''Int as Long'''
        self.check_value(3, 3, sample.acceptLong, int)

        # cLongMax goes here as CPython implements int as a C long
        self.check_value(cLongMax, cLongMax, sample.acceptLong, int)
        self.check_value(cLongMin, cLongMin, sample.acceptLong, int)

    def testIntAsUInt(self):
        '''Int as unsigned Int'''
        self.check_value(3, 3, sample.acceptUInt, long)
        self.assertRaises(OverflowError, sample.acceptUInt, -3)

    def testIntAsULong(self):
        '''Int as unsigned Long'''
        self.check_value(3, 3, sample.acceptULong, long)
        self.assertRaises(OverflowError, sample.acceptULong, -3)

    def testFloatAsDouble(self):
        '''Float as double'''
        self.check_value(3.14, 3.14, sample.acceptDouble, float)


class LongImplicitConvert(NumericTester):
    '''Test case for implicit converting C++ numeric types.'''

    def testLongAsInt(self):
        '''Long as Int'''
        self.check_value(l(24224), 24224, sample.acceptInt, int)
        self.assertRaises(OverflowError, sample.acceptInt, cIntMax + 20)

    def testLongAsLong(self):
        '''Long as Long'''
        self.check_value(l(2405), 2405, sample.acceptLong, int)
        self.assertRaises(OverflowError, sample.acceptLong, cLongMax + 20)

    def testLongAsUInt(self):
        '''Long as unsigned Int'''
        self.check_value(l(260), 260, sample.acceptUInt, long)
        self.assertRaises(OverflowError, sample.acceptUInt, -42)

    def testLongAsULong(self):
        '''Long as unsigned Long'''
        self.check_value(l(128), 128, sample.acceptULong, long)
        self.assertRaises(OverflowError, sample.acceptULong, l(-334))

    def testLongAsDouble(self):
        '''Float as double'''
        self.check_value(l(42), 42, sample.acceptDouble, float)


if __name__ == '__main__':
    unittest.main()
