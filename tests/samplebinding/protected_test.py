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

'''Test cases for protected methods.'''

import os
import unittest

from sample import ProtectedNonPolymorphic, ProtectedPolymorphic, ProtectedVirtualDestructor
from sample import Point

class ExtendedProtectedPolymorphic(ProtectedPolymorphic):
    def __init__(self, name):
        ProtectedPolymorphic.__init__(self, name)
    def protectedName(self):
        return 'Extended' + ProtectedPolymorphic.protectedName(self)

class ExtendedProtectedVirtualDestructor(ProtectedVirtualDestructor):
    def __init__(self):
        ProtectedVirtualDestructor.__init__(self)

class ProtectedNonPolymorphicTest(unittest.TestCase):
    '''Test cases for protected method in a class without virtual methods.'''

    def testProtectedCall(self):
        '''Calls a non-virtual protected method.'''
        p = ProtectedNonPolymorphic('NonPoly')
        self.assertEqual(p.publicName(), p.protectedName())
        a0, a1 = 1, 2
        self.assertEqual(p.protectedSum(a0, a1), a0 + a1)

    def testProtectedCallWithInstanceCreatedOnCpp(self):
        '''Calls a non-virtual protected method on an instance created in C++.'''
        p = ProtectedNonPolymorphic.create()
        self.assertEqual(p.publicName(), p.protectedName())
        a0, a1 = 1, 2
        self.assertEqual(p.protectedSum(a0, a1), a0 + a1)

    def testModifiedProtectedCall(self):
        '''Calls a non-virtual protected method modified with code injection.'''
        p = ProtectedNonPolymorphic('NonPoly')
        self.assertEqual(p.dataTypeName(), 'integer')
        self.assertEqual(p.dataTypeName(1), 'integer')
        self.assertEqual(p.dataTypeName(Point(1, 2)), 'pointer')

class ProtectedPolymorphicTest(unittest.TestCase):
    '''Test cases for protected method in a class with virtual methods.'''

    def testProtectedCall(self):
        '''Calls a virtual protected method.'''
        p = ProtectedNonPolymorphic('Poly')
        self.assertEqual(p.publicName(), p.protectedName())
        a0, a1 = 1, 2
        self.assertEqual(p.protectedSum(a0, a1), a0 + a1)

    def testProtectedCallWithInstanceCreatedOnCpp(self):
        '''Calls a virtual protected method on an instance created in C++.'''
        p = ProtectedPolymorphic.create()
        self.assertEqual(p.publicName(), p.protectedName())
        self.assertEqual(p.callProtectedName(), p.protectedName())

    def testReimplementedProtectedCall(self):
        '''Calls a reimplemented virtual protected method.'''
        p = ExtendedProtectedPolymorphic('Poly')
        self.assertEqual(p.callProtectedName(), p.protectedName())


class ProtectedVirtualDtorTest(unittest.TestCase):
    '''Test cases for protected virtual destructor.'''

    def setUp(self):
        ProtectedVirtualDestructor.resetDtorCounter()

    def testVirtualProtectedDtor(self):
        '''Original protected virtual destructor is being called.'''
        dtor_called = ProtectedVirtualDestructor.dtorCalled()
        for i in range(1, 10):
            pvd = ProtectedVirtualDestructor()
            del pvd
            self.assertEqual(ProtectedVirtualDestructor.dtorCalled(), dtor_called + i)

    def testVirtualProtectedDtorOnCppCreatedObject(self):
        '''Original protected virtual destructor is being called for a C++ created object.'''
        dtor_called = ProtectedVirtualDestructor.dtorCalled()
        for i in range(1, 10):
            pvd = ProtectedVirtualDestructor.create()
            del pvd
            self.assertEqual(ProtectedVirtualDestructor.dtorCalled(), dtor_called + i)

    def testProtectedDtorOnDerivedClass(self):
        '''Original protected virtual destructor is being called for a derived class.'''
        dtor_called = ExtendedProtectedVirtualDestructor.dtorCalled()
        for i in range(1, 10):
            pvd = ExtendedProtectedVirtualDestructor()
            del pvd
            self.assertEqual(ExtendedProtectedVirtualDestructor.dtorCalled(), dtor_called + i)


if __name__ == '__main__':
    unittest.main()

