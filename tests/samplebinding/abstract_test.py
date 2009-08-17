#!/usr/bin/python

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

    def unpureVirtual(self):
        self.unpure_virtual_called = True


class AbstractTest(unittest.TestCase):
    '''Test case for Abstract class'''

    def testAbstractPureVirtualMethodAvailability(self):
        '''Test if Abstract class pure virtual method was properly wrapped.'''
        self.assert_('pureVirtual' in dir(Abstract))

    def testAbstractInstanciation(self):
        '''Test if instanciation of an abstract class raises the correct exception.'''
        self.assertRaises(NotImplementedError, Abstract)

    def testUnimplementedPureVirtualMethodCall(self):
        '''Test if calling a pure virtual method raises the correct exception.'''
        i = Incomplete()
        self.assertRaises(NotImplementedError, i.pureVirtual)

    def testReimplementedVirtualMethodCall(self):
        '''Test if instanciation of an abstract class raises the correct exception.'''
        i = Concrete()
        self.assertRaises(NotImplementedError, i.callPureVirtual)

    def testReimplementedVirtualMethodCall(self):
        '''Test if a Python override of a virtual method is correctly called from C++.'''
        c = Concrete()
        c.callUnpureVirtual()
        self.assert_(c.unpure_virtual_called)

    def testImplementedPureVirtualMethodCall(self):
        '''Test if a Python override of a pure virtual method is correctly called from C++.'''
        c = Concrete()
        c.callPureVirtual()
        self.assert_(c.pure_virtual_called)

if __name__ == '__main__':
    unittest.main()

