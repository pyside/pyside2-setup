#!/usr/bin/python

'''Test cases for a class with a private destructor.'''

import gc
import sys
import unittest

from sample import PrivateDtor


class PrivateDtorTest(unittest.TestCase):
    '''Test case for PrivateDtor class'''

    def testPrivateDtorInstanciation(self):
        '''Test if instanciation of class with a private destructor raises an exception.'''
        self.assertRaises(TypeError, PrivateDtor)

    def testPrivateDtorInheritance(self):
        '''Test if inheriting from PrivateDtor raises an exception.'''
        def inherit():
            class Foo(PrivateDtor):
                pass
        self.assertRaises(TypeError, inherit)

    def testPrivateDtorInstanceMethod(self):
        '''Test if PrivateDtor.instance() method return the proper singleton.'''
        pd1 = PrivateDtor.instance()
        calls = pd1.instanceCalls()
        self.assertEqual(type(pd1), PrivateDtor)
        pd2 = PrivateDtor.instance()
        self.assertEqual(pd2, pd1)
        self.assertEqual(pd2.instanceCalls(), calls + 1)

    def testPrivateDtorRefCounting(self):
        '''Test refcounting of the singleton returned by PrivateDtor.instance().'''
        pd1 = PrivateDtor.instance()
        calls = pd1.instanceCalls()
        refcnt = sys.getrefcount(pd1)
        pd2 = PrivateDtor.instance()
        self.assertEqual(pd2.instanceCalls(), calls + 1)
        self.assertEqual(sys.getrefcount(pd2), sys.getrefcount(pd1))
        self.assertEqual(sys.getrefcount(pd2), refcnt + 1)
        del pd1
        self.assertEqual(sys.getrefcount(pd2), refcnt)
        del pd2
        gc.collect()
        pd3 = PrivateDtor.instance()
        self.assertEqual(type(pd3), PrivateDtor)
        self.assertEqual(pd3.instanceCalls(), calls + 2)
        self.assertEqual(sys.getrefcount(pd3), refcnt)

if __name__ == '__main__':
    unittest.main()

