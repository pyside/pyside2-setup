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

'''Test cases for Derived class'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

import sample
from sample import Abstract, Derived, OverloadedFuncEnum

class Deviant(Derived):
    def __init__(self):
        Derived.__init__(self)
        self.pure_virtual_called = False
        self.unpure_virtual_called = False

    def pureVirtual(self):
        self.pure_virtual_called = True

    def unpureVirtual(self):
        self.unpure_virtual_called = True

    def className(self):
        return 'Deviant'

class DerivedTest(unittest.TestCase):
    '''Test case for Derived class'''

    def testParentClassMethodsAvailability(self):
        '''Test if Derived class really inherits its methods from parent.'''
        inherited_methods = set(['callPureVirtual', 'callUnpureVirtual',
                                 'id_', 'pureVirtual', 'unpureVirtual'])
        self.assertTrue(inherited_methods.issubset(dir(Derived)))

    def testOverloadedMethodCall(self):
        '''Test if the correct overloaded method is being called.'''
        derived = Derived()

        result = derived.overloaded(1, 2)
        self.assertEqual(type(result), OverloadedFuncEnum)
        self.assertEqual(result, sample.OverloadedFunc_ii)

        result = derived.overloaded(3)
        self.assertEqual(type(result), OverloadedFuncEnum)
        self.assertEqual(result, sample.OverloadedFunc_ii)

        result = derived.overloaded(4.4)
        self.assertEqual(type(result), OverloadedFuncEnum)
        self.assertEqual(result, sample.OverloadedFunc_d)

    def testOtherOverloadedMethodCall(self):
        '''Another test to check overloaded method calling, just to double check.'''
        derived = Derived()

        result = derived.otherOverloaded(1, 2, True, 3.3)
        self.assertEqual(type(result), Derived.OtherOverloadedFuncEnum)
        self.assertEqual(result, sample.Derived.OtherOverloadedFunc_iibd)

        result = derived.otherOverloaded(1, 2.2)
        self.assertEqual(type(result), Derived.OtherOverloadedFuncEnum)
        self.assertEqual(result, Derived.OtherOverloadedFunc_id)

    def testOverloadedMethodCallWithDifferentNumericTypes(self):
        '''Test if the correct overloaded method accepts a different numeric type as argument.'''
        derived = Derived()
        result = derived.overloaded(1.1, 2.2)
        self.assertEqual(type(result), OverloadedFuncEnum)
        self.assertEqual(result, sample.OverloadedFunc_ii)

    def testOverloadedMethodCallWithWrongNumberOfArguments(self):
        '''Test if a call to an overloaded method with the wrong number of arguments raises an exception.'''
        derived = Derived()
        self.assertRaises(TypeError, derived.otherOverloaded, 1, 2, True)

    def testReimplementedPureVirtualMethodCall(self):
        '''Test if a Python override of a implemented pure virtual method is correctly called from C++.'''
        d = Deviant()
        d.callPureVirtual()
        self.assertTrue(d.pure_virtual_called)

    def testReimplementedVirtualMethodCall(self):
        '''Test if a Python override of a reimplemented virtual method is correctly called from C++.'''
        d = Deviant()
        d.callUnpureVirtual()
        self.assertTrue(d.unpure_virtual_called)

    def testVirtualMethodCallString(self):
        '''Test virtual method call returning string.'''
        d = Derived()
        self.assertEqual(d.className(), 'Derived')
        self.assertEqual(d.getClassName(), 'Derived')

    def testReimplementedVirtualMethodCallReturningString(self):
        '''Test if a Python override of a reimplemented virtual method is correctly called from C++.'''
        d = Deviant()
        self.assertEqual(d.className(), 'Deviant')
        self.assertEqual(d.getClassName(), 'Deviant')

    def testSingleArgument(self):
        '''Test singleArgument call.'''
        d = Derived()
        self.assertTrue(d.singleArgument(False))
        self.assertTrue(not d.singleArgument(True))

    def testMethodCallWithDefaultValue(self):
        '''Test method call with default value.'''
        d = Derived()
        self.assertEqual(d.defaultValue(3), 3.1)
        self.assertEqual(d.defaultValue(), 0.1)

    def testCallToMethodWithAbstractArgument(self):
        '''Call to method that expects an Abstract argument.'''
        objId = 123
        d = Derived(objId)
        self.assertEqual(Abstract.getObjectId(d), objId)

    def testObjectCreationWithParentType(self):
        '''Derived class creates an instance of itself in C++ and returns it as a pointer to its ancestor Abstract.'''
        obj = Derived.createObject()
        self.assertEqual(type(obj), Derived)

if __name__ == '__main__':
    unittest.main()

