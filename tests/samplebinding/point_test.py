#!/usr/bin/python

'''Test cases for Point class'''

import sys
import unittest

from sample import Point

class PointTest(unittest.TestCase):
    '''Test case for Point class, including operator overloads.'''

    def testConstructor(self):
        '''Test Point class constructor.'''
        pt = Point(5.0, 2.3)
        self.assertEqual(pt.x(), 5.0)
        self.assertEqual(pt.y(), 2.3)

    def testPlusOperator(self):
        '''Test Point class + operator.'''
        pt1 = Point(5.0, 2.3)
        pt2 = Point(0.5, 3.2)
        self.assertEqual(pt1 + pt2, Point(5.0 + 0.5, 2.3 + 3.2))

    def testEqualOperator(self):
        '''Test Point class == operator.'''
        pt1 = Point(5.0, 2.3)
        pt2 = Point(5.0, 2.3)
        pt3 = Point(0.5, 3.2)
        self.assertTrue(pt1 == pt1)
        self.assertTrue(pt1 == pt2)
        self.assertFalse(pt1 == pt3)

    def testNotEqualOperator(self):
        '''Test Point class != operator.'''
        pt1 = Point(5.0, 2.3)
        pt2 = Point(5.0, 2.3)
        self.assertRaises(NotImplementedError, lambda : pt1.__ne__(pt2))

if __name__ == '__main__':
    unittest.main()

