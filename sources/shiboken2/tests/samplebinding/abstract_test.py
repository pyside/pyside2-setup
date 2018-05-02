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

'''Test cases for Abstract class'''

import sys
import unittest

from sample import Abstract

class Incomplete(Abstract):
    def __init__(self):
        Abstract.__init__(self)

class Concrete(Abstract):
    def __init__(self):
        Abstract.__init__(self)
        self.pure_virtual_called = False
        self.unpure_virtual_called = False

    def pureVirtual(self):
        self.pure_virtual_called = True

    def pureVirtualReturningVoidPtr(self):
        return 42

    def unpureVirtual(self):
        self.unpure_virtual_called = True

    def virtualGettingAEnum(self, enum):
        self.virtual_getting_enum = True


class AbstractTest(unittest.TestCase):
    '''Test case for Abstract class'''

    def testAbstractPureVirtualMethodAvailability(self):
        '''Test if Abstract class pure virtual method was properly wrapped.'''
        self.assertTrue('pureVirtual' in dir(Abstract))

    def testAbstractInstanciation(self):
        '''Test if instanciation of an abstract class raises the correct exception.'''
        self.assertRaises(NotImplementedError, Abstract)

    def testUnimplementedPureVirtualMethodCall(self):
        '''Test if calling a pure virtual method raises the correct exception.'''
        i = Incomplete()
        self.assertRaises(NotImplementedError, i.pureVirtual)

    def testPureVirtualReturningVoidPtrReturnValue(self):
        '''Test if a pure virtual method returning void ptr can be properly reimplemented'''
        # Note that the semantics of reimplementing the pure virtual method in
        # Python and calling it from C++ is undefined until it's decided how to
        # cast the Python data types to void pointers
        c = Concrete()
        self.assertEqual(c.pureVirtualReturningVoidPtr(),42)

    def testReimplementedVirtualMethodCall(self):
        '''Test if instanciation of an abstract class raises the correct exception.'''
        i = Concrete()
        self.assertRaises(NotImplementedError, i.callPureVirtual)

    def testReimplementedVirtualMethodCall(self):
        '''Test if a Python override of a virtual method is correctly called from C++.'''
        c = Concrete()
        c.callUnpureVirtual()
        self.assertTrue(c.unpure_virtual_called)

    def testImplementedPureVirtualMethodCall(self):
        '''Test if a Python override of a pure virtual method is correctly called from C++.'''
        c = Concrete()
        c.callPureVirtual()
        self.assertTrue(c.pure_virtual_called)

    def testEnumParameterOnVirtualMethodCall(self):
        '''testEnumParameterOnVirtualMethodCall'''
        c = Concrete()
        c.callVirtualGettingEnum(Abstract.Short)
        self.assertTrue(c.virtual_getting_enum)

if __name__ == '__main__':
    unittest.main()

