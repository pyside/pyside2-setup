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

'''Test cases for std::pair container conversions'''

import sys
import unittest

from sample import PairUser

class ExtendedPairUser(PairUser):
    def __init__(self):
        PairUser.__init__(self)
        self.create_pair_called = False

    def createPair(self):
        self.create_pair_called = True
        return (7, 13)

class PairConversionTest(unittest.TestCase):
    '''Test case for std::pair container conversions'''

    def testReimplementedVirtualMethodCall(self):
        '''Test if a Python override of a virtual method is correctly called from C++.'''
        pu = ExtendedPairUser()
        pair = pu.callCreatePair()
        self.assertTrue(pu.create_pair_called)
        self.assertEqual(type(pair), tuple)
        self.assertEqual(type(pair[0]), int)
        self.assertEqual(type(pair[1]), int)
        self.assertEqual(pair, (7, 13))

    def testPrimitiveConversionInsideContainer(self):
        '''Test primitive type conversion inside conversible std::pair container.'''
        cpx0 = complex(1.2, 3.4)
        cpx1 = complex(5.6, 7.8)
        cp = PairUser.createComplexPair(cpx0, cpx1)
        self.assertEqual(type(cp), tuple)
        self.assertEqual(type(cp[0]), complex)
        self.assertEqual(type(cp[1]), complex)
        self.assertEqual(cp, (cpx0, cpx1))

    def testSumPair(self):
        '''Test method that sums the items of a pair using values of the types expected by C++ (int and double)'''
        pu = PairUser()
        pair = (3, 7.13)
        result = pu.sumPair(pair)
        self.assertEqual(result, sum(pair))

    def testSumPairDifferentTypes(self):
        '''Test method that sums the items of a pair using values of types different from the ones expected by C++ (int and double)'''
        pu = PairUser()
        pair = (3.3, 7)
        result = pu.sumPair(pair)
        self.assertNotEqual(result, sum(pair))
        self.assertEqual(result, int(pair[0]) + pair[1])

    def testConversionInBothDirections(self):
        '''Test converting a pair from Python to C++ and the other way around.'''
        pu = PairUser()
        pair = (3, 5)
        pu.setPair(pair)
        result = pu.getPair()
        self.assertEqual(result, pair)

    def testConversionInBothDirectionsWithSimilarContainer(self):
        '''Test converting a list, instead of the expected tuple, from Python to C++ and the other way around.'''
        pu = PairUser()
        pair = [3, 5]
        pu.setPair(pair)
        result = pu.getPair()
        self.assertNotEqual(result, pair)
        self.assertEqual(result, tuple(pair))

if __name__ == '__main__':
    unittest.main()

