#!/usr/bin/python

'''Test cases for operator overloads on Size class'''

import sys
import unittest

from sample import Size

class PointTest(unittest.TestCase):
    '''Test case for Size class, including operator overloads.'''

    def testConstructor(self):
        '''Test Size class constructor.'''
        width, height = (5.0, 2.3)
        size = Size(width, height)
        self.assertEqual(size.width(), width)
        self.assertEqual(size.height(), height)
        self.assertEqual(size.calculateArea(), width * height)

    def testPlusOperator(self):
        '''Test Size class + operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(0.5, 3.2)
        self.assertEqual(s1 + s2, Size(5.0 + 0.5, 2.3 + 3.2))

    def testEqualOperator(self):
        '''Test Size class == operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertTrue(s1 == s1)
        self.assertTrue(s1 == s2)
        self.assertFalse(s1 == s3)

    def testNotEqualOperator(self):
        '''Test Size class != operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertFalse(s1 != s1)
        self.assertFalse(s1 != s2)
        self.assertTrue(s1 != s3)

    def testMinorEqualOperator(self):
        '''Test Size class <= operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertTrue(s1 <= s1)
        self.assertTrue(s1 <= s2)
        self.assertTrue(s3 <= s1)
        self.assertFalse(s1 <= s3)

    def testMinorOperator(self):
        '''Test Size class < operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(0.5, 3.2)
        self.assertFalse(s1 < s1)
        self.assertFalse(s1 < s2)
        self.assertTrue(s2 < s1)

    def testMajorEqualOperator(self):
        '''Test Size class >= operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(5.0, 2.3)
        s3 = Size(0.5, 3.2)
        self.assertTrue(s1 >= s1)
        self.assertTrue(s1 >= s2)
        self.assertTrue(s1 >= s3)
        self.assertFalse(s3 >= s1)

    def testMajorOperator(self):
        '''Test Size class > operator.'''
        s1 = Size(5.0, 2.3)
        s2 = Size(0.5, 3.2)
        self.assertFalse(s1 > s1)
        self.assertTrue(s1 > s2)
        self.assertFalse(s2 > s1)

if __name__ == '__main__':
    unittest.main()

