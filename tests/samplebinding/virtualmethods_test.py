#!/usr/bin/python

'''Test cases for virtual methods.'''

import sys
import unittest

from sample import VirtualMethods, Point

class ExtendedVirtualMethods(VirtualMethods):
    def __init__(self):
        VirtualMethods.__init__(self)
        self.virtual_method0_called = False

    def virtualMethod0(self, pt, val, cpx, b):
        self.virtual_method0_called = True
        return VirtualMethods.virtualMethod0(self, pt, val, cpx, b) * -1.0

class VirtualMethodsTest(unittest.TestCase):
    '''Test case for virtual methods'''

    def testReimplementedVirtualMethod0(self):
        '''Test Python override of a virtual method with various different parameters is correctly called from C++.'''
        vm = VirtualMethods()
        evm = ExtendedVirtualMethods()
        pt = Point(1.1, 2.2)
        val = 4
        cpx = complex(3.3, 4.4)
        b = True
        result0 = vm.callVirtualMethod0(pt, val, cpx, b)
        result1 = evm.callVirtualMethod0(pt, val, cpx, b)
        self.assertEqual(result0 * -1.0, result1)

if __name__ == '__main__':
    unittest.main()

