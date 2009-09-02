#!/usr/bin/python
# -*- coding: utf-8 -*-

'''Test cases for ...'''

import sys
import unittest

from sample import NonDefaultCtor

class DerivedNonDefaultCtor (NonDefaultCtor):
    def returnMyselfVirtual(self):
        return NonDefaultCtor(self.value()+1)
    pass

class NonDefaultCtorTest(unittest.TestCase):

    def testNonDefaultCtor(self):
        c = NonDefaultCtor(2)
        # these functions returns NonDefaultCtor by value, so a PyObjecy  is created every time
        self.assertNotEqual(c.returnMyself(), c)
        self.assertEqual(c.returnMyself().value(), 2)
        self.assertNotEqual(c.returnMyself(3), c)
        self.assertEqual(c.returnMyself(3).value(), 2)
        self.assertNotEqual(c.returnMyself(4, c), c)
        self.assertEqual(c.returnMyself(4, c).value(), 2)

    def testVirtuals(self):
        c = DerivedNonDefaultCtor(3)
        # these functions returns NonDefaultCtor by value, so a PyObjecy  is created every time
        self.assertNotEqual(c.returnMyselfVirtual(), c)
        self.assertEqual(c.returnMyselfVirtual().value(), 4)
        self.assertEqual(c.callReturnMyselfVirtual().value(), 4)


if __name__ == '__main__':
    unittest.main()

