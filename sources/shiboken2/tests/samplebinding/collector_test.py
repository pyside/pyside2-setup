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

'''Test cases for Collector class' shift operators.'''

import sys
import unittest

from sample import Collector, IntWrapper, ObjectType


class CollectorTest(unittest.TestCase):
    '''Test cases for Collector class' shift operators.'''

    def testLShiftOperatorSingleUse(self):
        '''Test case for using the Collector.__lshift__ operator just one time.'''
        collector = Collector()
        collector << 13
        self.assertEqual(collector.size(), 1)
        self.assertEqual(collector.items(), [13])

    def testLShiftOperatorMultipleUses(self):
        '''Test case for using the Collector.__lshift__ operator many times in the same line.'''
        collector = Collector()
        collector << 2 << 3 << 5 << 7 << 11
        self.assertEqual(collector.size(), 5)
        self.assertEqual(collector.items(), [2, 3, 5, 7, 11])

class CollectorExternalOperator(unittest.TestCase):
    '''Test cases for external operators of Collector'''

    def testLShiftExternal(self):
        '''Collector external operator'''
        collector = Collector()
        collector << IntWrapper(5)
        self.assertEqual(collector.size(), 1)
        self.assertEqual(collector.items(), [5])


class CollectorObjectType(unittest.TestCase):
    '''Test cases for Collector ObjectType'''

    def testBasic(self):
        '''Collector << ObjectType # greedy collector'''
        collector = Collector()
        obj = ObjectType()
        collector << obj
        self.assertEqual(collector.items()[0], obj.identifier())


if __name__ == '__main__':
    unittest.main()

