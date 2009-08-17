#!/usr/bin/python

'''Test cases for Python representation of C++ enums'''

import sys
import unittest

from sample import SampleNamespace

class EnumTest(unittest.TestCase):
    '''Test case for Abstract class'''

    def testPassingIntegerOnEnumArgument(self):
        '''Test if replacing an enum argument with an integer raises an exception.'''
        self.assertRaises(TypeError, lambda : SampleNamespace.getNumber(1))

    def testExtendingEnum(self):
        '''Test if can create new items for an enum declared as extensible on the typesystem file.'''
        name, value = 'NewItem', 13
        enumitem = SampleNamespace.Option(name, value)
        self.assert_(type(enumitem), SampleNamespace.Option)
        self.assert_(enumitem.name, name)
        self.assert_(int(enumitem), value)

    def testExtendingNonExtensibleEnum(self):
        '''Test if trying to create a new enum item for an unextensible enum raises an exception.'''
        self.assertRaises(TypeError, lambda : SampleNamespace.InValue(13))

    def testEnumConversionToAndFromPython(self):
        '''Test conversion of enum objects to Python and C++ in both directions.'''
        enumout = SampleNamespace.enumInEnumOut(SampleNamespace.TwoIn)
        self.assert_(enumout, SampleNamespace.TwoOut)

if __name__ == '__main__':
    unittest.main()

