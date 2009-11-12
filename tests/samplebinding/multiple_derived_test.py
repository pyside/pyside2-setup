#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# This file is part of the Shiboken Python Bindings Generator project.
#
# Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
#
# Contact: PySide team <contact@pyside.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation. Please
# review the following information to ensure the GNU Lesser General
# Public License version 2.1 requirements will be met:
# http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
# #
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA

'''Test cases for multiple inheritance'''

import sys
import unittest

from sample import Base1, Base2, Base3, Base4, Base5, MDerived1, MDerived2, MDerived3

class MultipleDerivedTest(unittest.TestCase):
    '''Test cases for multiple inheritance'''

    def testIsInstance(self):
        '''MDerived1 is instance of its parents Base1 and Base2.'''
        a = MDerived1()
        self.assert_(isinstance(a, MDerived1))
        self.assert_(isinstance(a, Base1))
        self.assert_(isinstance(a, Base2))

    def testIsSubclass(self):
        '''MDerived1 is subclass of its parents Base1 and Base2.'''
        self.assert_(issubclass(MDerived1, Base1))
        self.assert_(issubclass(MDerived1, Base2))

    def testCallToFunctionWithBase1ArgumentThatCastsBackToMDerived1(self):
        '''MDerived1 is passed as an Base1 argument to method that returns it casted back to MDerived1.'''
        a = MDerived1()
        b = MDerived1.transformFromBase1(a)
        self.assertEqual(a, b)

    def testCallToFunctionWithBase2ArgumentThatCastsBackToMDerived1(self):
        '''MDerived1 is passed as an Base2 argument to method that returns it casted back to MDerived1.'''
        a = MDerived1()
        b = MDerived1.transformFromBase2(a)
        self.assertEqual(a, b)

    def testCastFromMDerived1ToBase1(self):
        '''MDerived1 is casted by C++ to its first parent Base2 and the binding must return the MDerived1 wrapper.'''
        a = MDerived1()
        refcnt = sys.getrefcount(a)
        b = a.castToBase1()
        self.assert_(isinstance(b, MDerived1))
        self.assertEqual(a, b)
        self.assertEqual(sys.getrefcount(a), refcnt + 1)

    def testCastFromMDerived1ToBases(self):
        '''MDerived1 is casted by C++ to its parents and the binding must return the MDerived1 wrapper.'''
        a = MDerived1()
        refcnt = sys.getrefcount(a)
        b1 = a.castToBase1()
        b2 = a.castToBase2()
        self.assert_(isinstance(b1, MDerived1))
        self.assert_(isinstance(b2, MDerived1))
        self.assertEqual(a, b1)
        self.assertEqual(a, b2)
        self.assertEqual(sys.getrefcount(a), refcnt + 2)

    def testCastFromMDerived2ToBases(self):
        '''MDerived2 is casted by C++ to its parents and the binding must return the MDerived2 wrapper.'''
        a = MDerived2()
        refcnt = sys.getrefcount(a)
        b3 = a.castToBase3()
        b4 = a.castToBase4()
        b5 = a.castToBase5()
        b6 = a.castToBase6()
        self.assert_(isinstance(b3, MDerived2))
        self.assert_(isinstance(b4, MDerived2))
        self.assert_(isinstance(b5, MDerived2))
        self.assert_(isinstance(b6, MDerived2))
        self.assertEqual(a, b3)
        self.assertEqual(a, b4)
        self.assertEqual(a, b5)
        self.assertEqual(a, b6)
        self.assertEqual(sys.getrefcount(a), refcnt + 4)

if __name__ == '__main__':
    unittest.main()

