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

import sys
import unittest

from sample import Overload, Point, Size

class OVerloadTest(unittest.TestCase):
    '''Test case for Overload class'''

    def testOverloadMethod0(self):
        '''Check overloaded method calling for signature "overloaded()".'''
        overload = Overload()
        result = overload.overloaded()
        self.assertEqual(result, Overload.Function0)

    def testOverloadMethod1(self):
        '''Check overloaded method calling for signature "overloaded(Size*)".'''
        overload = Overload()
        size = Size()
        result = overload.overloaded(size)
        self.assertEqual(result, Overload.Function1)

    def testOverloadMethod2(self):
        '''Check overloaded method calling for signature "overloaded(Point*, ParamEnum)".'''
        overload = Overload()
        point = Point()
        result = overload.overloaded(point, Overload.Param1)
        self.assertEqual(result, Overload.Function2)

    def testOverloadMethod3(self):
        '''Check overloaded method calling for signature "overloaded(const Point&)".'''
        overload = Overload()
        point = Point()
        result = overload.overloaded(point)
        self.assertEqual(result, Overload.Function3)

if __name__ == '__main__':
    unittest.main()

