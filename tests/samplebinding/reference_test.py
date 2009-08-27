#!/usr/bin/python

'''Test cases for methods that receive references to objects.'''

import sys
import unittest

from sample import Reference

class ExtendedReference(Reference):
    def __init__(self):
        Reference.__init__(self)
        self.uses_reference_virtual_called = False
        self.uses_const_reference_virtual_called = False
        self.reference_inc = 1
        self.const_reference_inc = 2

    def usesReferenceVirtual(self, ref, inc):
        self.uses_reference_virtual_called = True
        return ref.objId() + inc + self.reference_inc

    def usesConstReferenceVirtual(self, ref, inc):
        self.uses_const_reference_virtual_called = True
        return ref.objId() + inc + self.const_reference_inc

class ReferenceTest(unittest.TestCase):
    '''Test case for methods that receive references to objects.'''

    def testMethodThatReceivesReference(self):
        '''Test a method that receives a reference to an object as argument.'''
        objId = 123
        r = Reference(objId)
        self.assertEqual(Reference.usesReference(r), objId)

    def testMethodThatReceivesConstReference(self):
        '''Test a method that receives a const reference to an object as argument.'''
        objId = 123
        r = Reference(objId)
        self.assertEqual(Reference.usesConstReference(r), objId)

    def testReimplementedVirtualMethodCallWithReferenceParameter(self):
        '''Test if a Python override of a virtual method with a reference parameter is correctly called from C++.'''
        inc = 9
        objId = 123
        r = Reference(objId)
        er = ExtendedReference()
        result = er.callUsesReferenceVirtual(r, inc)
        self.assertEqual(result, objId + inc + er.reference_inc)

    def testReimplementedVirtualMethodCallWithConstReferenceParameter(self):
        '''Test if a Python override of a virtual method with a const reference parameter is correctly called from C++.'''
        inc = 9
        objId = 123
        r = Reference(objId)
        er = ExtendedReference()
        result = er.callUsesConstReferenceVirtual(r, inc)
        self.assertEqual(result, objId + inc + er.const_reference_inc)

if __name__ == '__main__':
    unittest.main()

