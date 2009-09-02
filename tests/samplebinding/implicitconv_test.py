#!/usr/bin/python

'''Test cases for implicit conversions'''

import sys
import unittest

from sample import ImplicitConv

class ImplicitConvTest(unittest.TestCase):
    '''Test case for implicit conversions'''

    def testImplicitConversions(self):
        '''Test if overloaded function call decisor takes implicit conversions into account.'''
        ic = ImplicitConv.implicitConvCommon(ImplicitConv())
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorNone)

        ic = ImplicitConv.implicitConvCommon(3)
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorOne)
        self.assertEqual(ic.objId(), 3)

        ic = ImplicitConv.implicitConvCommon(ImplicitConv.CtorThree)
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorThree)

if __name__ == '__main__':
    unittest.main()

