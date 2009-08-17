#!/usr/bin/python

'''Test cases for Derived class'''

import sys
import unittest

import sample
from sample import Abstract, Derived, PolymorphicFuncEnum

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
        self.assert_(inherited_methods.issubset(dir(Derived)))

    def testPolymorphicMethodCall(self):
        '''Test if the correct polymorphic method is being called.'''
        derived = Derived()

        result = derived.polymorphic(1, 2)
        self.assertEqual(type(result), PolymorphicFuncEnum)
        self.assertEqual(result, sample.PolymorphicFunc_ii)

        result = derived.polymorphic(3)
        self.assertEqual(type(result), PolymorphicFuncEnum)
        self.assertEqual(result, sample.PolymorphicFunc_ii)

        result = derived.polymorphic(4.4)
        self.assertEqual(type(result), PolymorphicFuncEnum)
        self.assertEqual(result, sample.PolymorphicFunc_d)

    def testOtherPolymorphicMethodCall(self):
        '''Another test to check polymorphic method calling, just to double check.'''
        derived = Derived()

        result = derived.otherPolymorphic(1, 2, True, 3.3)
        self.assertEqual(type(result), Derived.OtherPolymorphicFuncEnum)
        self.assertEqual(result, sample.Derived.OtherPolymorphicFunc_iibd)

        result = derived.otherPolymorphic(1, 2.2)
        self.assertEqual(type(result), Derived.OtherPolymorphicFuncEnum)
        self.assertEqual(result, Derived.OtherPolymorphicFunc_id)

    def testPolymorphicMethodCallWithDifferentNumericTypes(self):
        '''Test if the correct polymorphic method accepts a different numeric type as argument.'''
        derived = Derived()
        result = derived.polymorphic(1.1, 2.2)
        self.assertEqual(type(result), PolymorphicFuncEnum)
        self.assertEqual(result, sample.PolymorphicFunc_ii)

    def testPolymorphicMethodCallWithWrongNumberOfArguments(self):
        '''Test if a call to a polymorphic method with the wrong number of arguments raises an exception.'''
        derived = Derived()
        self.assertRaises(TypeError, lambda : derived.otherPolymorphic(1, 2, True))

    def testReimplementedPureVirtualMethodCall(self):
        '''Test if a Python override of a implemented pure virtual method is correctly called from C++.'''
        d = Deviant()
        d.callPureVirtual()
        self.assert_(d.pure_virtual_called)

    def testReimplementedVirtualMethodCall(self):
        '''Test if a Python override of a reimplemented virtual method is correctly called from C++.'''
        d = Deviant()
        d.callUnpureVirtual()
        self.assert_(d.unpure_virtual_called)

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
        self.assert_(d.singleArgument(False))
        self.assert_(not d.singleArgument(True))

    def testMethodCallWithDefaultValue(self):
        '''Test method call with default value.'''
        d = Derived()
        self.assertEqual(d.defaultValue(3), 3.1)
        self.assertEqual(d.defaultValue(), 0.1)

if __name__ == '__main__':
    unittest.main()

