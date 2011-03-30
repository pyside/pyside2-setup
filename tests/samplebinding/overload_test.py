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

'''Test cases for Overload class'''

import unittest

from sample import Overload, Point, Size

class OverloadTest(unittest.TestCase):
    '''Test case for Overload class'''

    def testOverloadMethod0(self):
        '''Check overloaded method call for signature "overloaded()".'''
        overload = Overload()
        self.assertEqual(overload.overloaded(), Overload.Function0)

    def testOverloadMethod1(self):
        '''Check overloaded method call for signature "overloaded(Size*)".'''
        overload = Overload()
        size = Size()
        self.assertEqual(overload.overloaded(size), Overload.Function1)

    def testOverloadMethod2(self):
        '''Check overloaded method call for signature "overloaded(Point*, ParamEnum)".'''
        overload = Overload()
        point = Point()
        self.assertEqual(overload.overloaded(point, Overload.Param1), Overload.Function2)

    def testOverloadMethod3(self):
        '''Check overloaded method call for signature "overloaded(const Point&)".'''
        overload = Overload()
        point = Point()
        self.assertEqual(overload.overloaded(point), Overload.Function3)

    def testDifferentReturnTypes(self):
        '''Check method calls for overloads with different return types.'''
        overload = Overload()
        self.assertEqual(overload.differentReturnTypes(), None)
        self.assertEqual(overload.differentReturnTypes(Overload.Param1), None)
        self.assertEqual(overload.differentReturnTypes(Overload.Param0, 13), 13)

    def testIntOverloads(self):
        overload = Overload()
        self.assertEqual(overload.intOverloads(2, 3), 2)
        self.assertEqual(overload.intOverloads(2, 4.5), 3)
        self.assertEqual(overload.intOverloads(Point(0, 0), 3), 1)

if __name__ == '__main__':
    unittest.main()

