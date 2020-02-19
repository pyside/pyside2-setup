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

'''Test cases for OtherDerived class'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import Abstract, Derived
from other import OtherDerived, Number

class Multiple(Derived, Number):
    def __init__(self):
        Derived.__init__(self, 42)
        Number.__init__(self, 42)

    def testCall(self):
        return True

class OtherDeviant(OtherDerived):
    def __init__(self):
        OtherDerived.__init__(self)
        self.pure_virtual_called = False
        self.unpure_virtual_called = False

    def pureVirtual(self):
        self.pure_virtual_called = True

    def unpureVirtual(self):
        self.unpure_virtual_called = True

    def className(self):
        return 'OtherDeviant'

class MultipleTest(unittest.TestCase):
    '''Test case for Multiple derived class'''

    def testConstructor(self):
        o = Multiple()
        self.assertTrue(isinstance(o, Multiple))
        self.assertTrue(isinstance(o, Number))
        self.assertTrue(isinstance(o, Derived))
        del o

    def testMethodCall(self):
        o = Multiple()
        self.assertTrue(o.id_(), 42)
        self.assertTrue(o.value(), 42)
        self.assertTrue(o.testCall())

class OtherDerivedTest(unittest.TestCase):
    '''Test case for OtherDerived class'''

    def testParentClassMethodsAvailability(self):
        '''Test if OtherDerived class really inherits its methods from parent.'''
        inherited_methods = set(['callPureVirtual', 'callUnpureVirtual',
                                 'id_', 'pureVirtual', 'unpureVirtual'])
        self.assertTrue(inherited_methods.issubset(dir(OtherDerived)))

    def testReimplementedPureVirtualMethodCall(self):
        '''Test if a Python override of a implemented pure virtual method is correctly called from C++.'''
        d = OtherDeviant()
        d.callPureVirtual()
        self.assertTrue(d.pure_virtual_called)

    def testReimplementedVirtualMethodCall(self):
        '''Test if a Python override of a reimplemented virtual method is correctly called from C++.'''
        d = OtherDeviant()
        d.callUnpureVirtual()
        self.assertTrue(d.unpure_virtual_called)

    def testVirtualMethodCallString(self):
        '''Test virtual method call returning string.'''
        d = OtherDerived()
        self.assertEqual(d.className(), 'OtherDerived')
        self.assertEqual(d.getClassName(), 'OtherDerived')

    def testReimplementedVirtualMethodCallReturningString(self):
        '''Test if a Python override of a reimplemented virtual method is correctly called from C++.'''
        d = OtherDeviant()
        self.assertEqual(d.className(), 'OtherDeviant')
        self.assertEqual(d.getClassName(), 'OtherDeviant')

    def testCallToMethodWithAbstractArgument(self):
        '''Call to method that expects an Abstract argument.'''
        objId = 123
        d = OtherDerived(objId)
        self.assertEqual(Abstract.getObjectId(d), objId)

if __name__ == '__main__':
    unittest.main()

