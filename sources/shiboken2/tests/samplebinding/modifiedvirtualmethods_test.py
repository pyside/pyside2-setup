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

'''Test cases for modified virtual methods.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import VirtualMethods, Str

class ExtendedVirtualMethods(VirtualMethods):
    def __init__(self):
        VirtualMethods.__init__(self)
        self.name_called = False
        self.sum0_called = False
        self.sum1_called = False
        self.sum2_called = False
        self.sum3_called = False
        self.sum4_called = False
        self.sumThree_called = False
        self.callMe_called = 0
        self.multiplier = 12345

    def sum0(self, a0, a1, a2):
        self.sum0_called = True
        return VirtualMethods.sumThree(self, a0, a1, a2) * self.multiplier

    def sumThree(self, a0, a1, a2):
        self.sumThree_called = True
        return VirtualMethods.sumThree(self, a0, a1, a2) * self.multiplier

    def sum1(self, a0, a1, a2):
        self.sum1_called = True
        return VirtualMethods.sum1(self, a0, a1, a2) * self.multiplier

    def sum2(self, a0, a1):
        self.sum2_called = True
        return VirtualMethods.sum2(self, a0, a1) * self.multiplier

    def sum3(self, a0, a1):
        self.sum3_called = True
        return VirtualMethods.sum3(self, a0, a1) * self.multiplier

    def sum4(self, a0, a1):
        self.sum4_called = True
        return VirtualMethods.sum4(self, a0, a1) * self.multiplier

    def name(self):
        self.name_called = True
        return Str('ExtendedVirtualMethods')

    def callMe(self):
        self.callMe_called += 1

    def getMargins(self):
        return tuple([m*2 for m in VirtualMethods.getMargins(self)])


class VirtualMethodsTest(unittest.TestCase):
    '''Test case for modified virtual methods.'''

    def setUp(self):
        self.vm = VirtualMethods()
        self.evm = ExtendedVirtualMethods()

    def tearDown(self):
        del self.vm
        del self.evm

    def testModifiedVirtualMethod0(self):
        '''Renamed virtual method.'''
        a0, a1, a2 = 2, 3, 5
        result0 = self.vm.callSum0(a0, a1, a2)
        result1 = self.vm.sumThree(a0, a1, a2)
        self.assertEqual(result0, a0 + a1 + a2)
        self.assertEqual(result0, result1)
        self.assertRaises(AttributeError, getattr, self.vm, 'sum0')

    def testReimplementedModifiedVirtualMethod0(self):
        '''Override of a renamed virtual method.'''
        a0, a1, a2 = 2, 3, 5
        result0 = self.vm.callSum0(a0, a1, a2)
        result1 = self.vm.sumThree(a0, a1, a2)
        result2 = self.evm.callSum0(a0, a1, a2)
        self.assertEqual(result0, result1)
        self.assertEqual(result0 * self.evm.multiplier, result2)
        self.assertTrue(self.evm.sumThree_called)
        self.assertFalse(self.evm.sum0_called)

    def testModifiedVirtualMethod1(self):
        '''Virtual method with three arguments and the last one
        changed to have the default value set to 1000.'''
        a0, a1, a2 = 2, 3, 5
        result0 = self.vm.sum1(a0, a1)
        self.assertEqual(result0, a0 + a1 + 1000)
        result1 = self.vm.sum1(a0, a1, a2)
        result2 = self.vm.callSum1(a0, a1, a2)
        self.assertEqual(result1, result2)

    def testReimplementedModifiedVirtualMethod1(self):
        '''Override of the virtual method with three arguments and
        the last one changed to have the default value set to 1000.'''
        a0, a1 = 2, 3
        result0 = self.vm.sum1(a0, a1)
        result1 = self.evm.callSum1(a0, a1, 1000)
        self.assertEqual(result0 * self.evm.multiplier, result1)
        self.assertTrue(self.evm.sum1_called)

    def testModifiedVirtualMethod2(self):
        '''Virtual method originally with three arguments, the last
        one was removed and the default value set to 2000.'''
        a0, a1 = 1, 2
        result0 = self.vm.sum2(a0, a1)
        self.assertEqual(result0, a0 + a1 + 2000)
        result1 = self.vm.sum2(a0, a1)
        result2 = self.vm.callSum2(a0, a1, 2000)
        self.assertEqual(result1, result2)
        self.assertRaises(TypeError, self.vm.sum2, 1, 2, 3)

    def testReimplementedModifiedVirtualMethod2(self):
        '''Override of the virtual method originally with three arguments,
        the last one was removed and the default value set to 2000.'''
        a0, a1 = 1, 2
        ignored = 54321
        result0 = self.vm.sum2(a0, a1)
        result1 = self.evm.callSum2(a0, a1, ignored)
        self.assertEqual(result0 * self.evm.multiplier, result1)
        self.assertTrue(self.evm.sum2_called)

    def testModifiedVirtualMethod3(self):
        '''Virtual method originally with three arguments have the second
        one removed and replaced by custom code that replaces it by the sum
        of the first and the last arguments.'''
        a0, a1 = 1, 2
        result0 = self.vm.sum3(a0, a1)
        self.assertEqual(result0, a0 + (a0 + a1) + a1)
        result1 = self.vm.callSum3(a0, 10, a1)
        self.assertNotEqual(result0, result1)
        result2 = self.vm.callSum3(a0, a0 + a1, a1)
        self.assertEqual(result0, result2)
        self.assertRaises(TypeError, self.vm.sum3, 1, 2, 3)

    def testReimplementedModifiedVirtualMethod3(self):
        '''Override of the virtual method originally with three arguments
        have the second one removed and replaced by custom code that
        replaces it by the sum of the first and the last arguments.'''
        a0, a1 = 1, 2
        ignored = 54321
        result0 = self.vm.sum3(a0, a1)
        result1 = self.evm.callSum3(a0, ignored, a1)
        self.assertEqual(result0 * self.evm.multiplier, result1)
        self.assertTrue(self.evm.sum3_called)

    def testModifiedVirtualMethod4(self):
        '''Virtual method originally with three arguments, the
        last one was removed and the default value set to 3000.'''
        a0, a1 = 1, 2
        default_value = 3000
        result0 = self.vm.sum4(a0, a1)
        self.assertEqual(result0, a0 + default_value + a1)
        removed_arg_value = 100
        result1 = self.vm.callSum4(a0, removed_arg_value, a1)
        self.assertEqual(result1, a0 + removed_arg_value + a1)
        self.assertRaises(TypeError, self.vm.sum4, 1, 2, 3)

    def testReimplementedModifiedVirtualMethod4(self):
        '''Override of the virtual method originally with three arguments,
        the last one was removed and the default value set to 3000.
        The method was modified with code injection on the binding override
        (the one that receives calls from C++ with the original signature
        and forwards it to Python overrides) that subtracts the value of the
        second argument (removed in Python) from the value of the first
        before sending them to Python.'''
        a0, a1 = 1, 2
        removed_arg_value = 2011
        default_value = 3000
        result = self.evm.callSum4(a0, removed_arg_value, a1)
        self.assertEqual(result, (a0 - removed_arg_value + a1 + default_value) * self.evm.multiplier)
        self.assertTrue(self.evm.sum4_called)

    def testOverridenMethodResultModification(self):
        '''Injected code modifies the result of a call to a virtual
        method overridden in Python.'''
        orig_name = self.vm.callName()
        self.assertEqual(orig_name, 'VirtualMethods')
        name = self.evm.callName()
        self.assertEqual(name, 'PimpedExtendedVirtualMethods')
        self.assertEqual(name, Str('PimpedExtendedVirtualMethods'))
        self.assertTrue(self.evm.name_called)

    def testInjectCodeCallsPythonVirtualMethodOverride(self):
        '''When injected code calls the Python override by itself
        no code for the method call should be generated.'''
        self.evm.callCallMe()
        self.assertEqual(self.evm.callMe_called, 1)

    def testAllArgumentsRemoved(self):
        values = (10, 20, 30, 40)
        self.vm.setMargins(*values)
        self.assertEqual(self.vm.getMargins(), values)

    def testAllArgumentsRemovedCallVirtual(self):
        values = (10, 20, 30, 40)
        self.vm.setMargins(*values)
        self.assertEqual(self.vm.callGetMargins(), values)

    def testExtendedAllArgumentsRemoved(self):
        values = (10, 20, 30, 40)
        self.evm.setMargins(*values)
        double = tuple([m*2 for m in values])
        self.assertEqual(self.evm.getMargins(), double)

    def testExtendedAllArgumentsRemovedCallVirtual(self):
        values = (10, 20, 30, 40)
        self.evm.setMargins(*values)
        double = tuple([m*2 for m in values])
        self.assertEqual(self.evm.callGetMargins(), double)

if __name__ == '__main__':
    unittest.main()

