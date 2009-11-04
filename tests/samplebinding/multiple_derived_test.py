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

from sample import MBase1, MBase2, MDerived

class MultipleDerivedTest(unittest.TestCase):
    '''Test cases for multiple inheritance'''

    def testIsInstance(self):
        '''MDerived is instance of its parents MBase1 and MBase2.'''
        a = MDerived()
        self.assert_(isinstance(a, MDerived))
        self.assert_(isinstance(a, MBase1))
        self.assert_(isinstance(a, MBase2))

    def testIsSubclass(self):
        '''MDerived is subclass of its parents MBase1 and MBase2.'''
        self.assert_(issubclass(MDerived, MBase1))
        self.assert_(issubclass(MDerived, MBase2))

    def testCallToFunctionWithMBase1ArgumentThatCastsBackToMDerived(self):
        '''MDerived is passed as an MBase1 argument to method that returns it casted back to MDerived.'''
        a = MDerived()
        b = MDerived.transformFromBase1(a)
        self.assertEqual(a, b)

    def testCallToFunctionWithMBase2ArgumentThatCastsBackToMDerived(self):
        '''MDerived is passed as an MBase2 argument to method that returns it casted back to MDerived.'''
        a = MDerived()
        b = MDerived.transformFromBase2(a)
        self.assertEqual(a, b)

    def testCastFromMDerivedToMBase1(self):
        '''MDerived is casted by C++ to its first parent MBase2 and the binding must return the MDerived wrapper.'''
        a = MDerived()
        refcnt = sys.getrefcount(a)
        b = a.castToMBase1()
        self.assert_(isinstance(b, MDerived))
        self.assertEqual(a, b)
        self.assertEqual(sys.getrefcount(a), refcnt + 1)

    """
    # This method must be commented since it will break the test flow until the problem is fixed.
    def testCastFromMDerivedToMBase2(self):
        '''MDerived is casted by C++ to its second parent MBase2 and the binding must return the MDerived wrapper.'''
        a = MDerived()
        refcnt = sys.getrefcount(a)
        b = a.castToMBase2()
        self.assert_(isinstance(b, MDerived))
        self.assertEqual(a, b)
        self.assertEqual(sys.getrefcount(a), refcnt + 1)
    """

if __name__ == '__main__':
    unittest.main()

