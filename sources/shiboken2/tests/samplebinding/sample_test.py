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

'''Test cases for libsample bindings module'''

import unittest

import sample

class ModuleTest(unittest.TestCase):
    '''Test case for module and global functions'''

    def testModuleMembers(self):
        '''Test availability of classes, global functions and other members on binding'''
        expected_members = set(['Abstract', 'Derived', 'Point',
                                'ListUser', 'PairUser', 'MapUser',
                                'gimmeComplexList', 'gimmeDouble', 'gimmeInt',
                                'makeCString', 'multiplyPair', 'returnCString',
                                'SampleNamespace', 'transmuteComplexIntoPoint',
                                'transmutePointIntoComplex', 'sumComplexPair',
                                'FirstThing', 'SecondThing', 'ThirdThing',
                                'GlobalEnum', 'NoThing'])
        self.assertTrue(expected_members.issubset(dir(sample)))

    def testAbstractPrintFormatEnum(self):
        '''Test availability of PrintFormat enum from Abstract class'''
        enum_members = set(['PrintFormat', 'Short', 'Verbose',
                            'OnlyId', 'ClassNameAndId'])
        self.assertTrue(enum_members.issubset(dir(sample.Abstract)))

    def testSampleNamespaceOptionEnum(self):
        '''Test availability of Option enum from SampleNamespace namespace'''
        enum_members = set(['Option', 'None_', 'RandomNumber', 'UnixTime'])
        self.assertTrue(enum_members.issubset(dir(sample.SampleNamespace)))

    def testAddedFunctionAtModuleLevel(self):
        '''Calls function added to module from type system description.'''
        str1 = 'Foo'
        self.assertEqual(sample.multiplyString(str1, 3), str1 * 3)
        self.assertEqual(sample.multiplyString(str1, 0), str1 * 0)

    def testAddedFunctionWithVarargs(self):
        '''Calls function that receives varargs added to module from type system description.'''
        self.assertEqual(sample.countVarargs(1), 0)
        self.assertEqual(sample.countVarargs(1, 2), 1)
        self.assertEqual(sample.countVarargs(1, 2, 3, 'a', 'b', 4, (5, 6)), 6)

if __name__ == '__main__':
    unittest.main()

