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

from sample import Echo, Overload, Point, PointF, Polygon, Rect, RectF, Size, Str

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

    def testIntDoubleOverloads(self):
        overload = Overload()
        self.assertEqual(overload.intDoubleOverloads(1, 2), Overload.Function0)
        self.assertEqual(overload.intDoubleOverloads(1, 2.0), Overload.Function0)
        self.assertEqual(overload.intDoubleOverloads(1.0, 2), Overload.Function1)
        self.assertEqual(overload.intDoubleOverloads(1.0, 2.0), Overload.Function1)

    def testWrapperIntIntOverloads(self):
        overload = Overload()
        self.assertEqual(overload.wrapperIntIntOverloads(Point(), 1, 2), Overload.Function0)
        self.assertEqual(overload.wrapperIntIntOverloads(Polygon(), 1, 2), Overload.Function1)

    def testDrawTextPointAndStr(self):
        overload = Overload()
        self.assertEqual(overload.drawText(Point(), Str()), Overload.Function0)
        self.assertEqual(overload.drawText(Point(), ''), Overload.Function0)
        self.assertEqual(overload.drawText(PointF(), Str()), Overload.Function1)
        self.assertEqual(overload.drawText(PointF(), ''), Overload.Function1)

    def testDrawTextRectIntStr(self):
        overload = Overload()
        self.assertEqual(overload.drawText(Rect(), 1, Str()), Overload.Function2)
        self.assertEqual(overload.drawText(Rect(), 1, ''), Overload.Function2)
        self.assertEqual(overload.drawText(RectF(), 1, Str()), Overload.Function3)
        self.assertEqual(overload.drawText(RectF(), 1, ''), Overload.Function3)

    def testDrawTextRectFStrEcho(self):
        overload = Overload()
        self.assertEqual(overload.drawText(RectF(), Str()), Overload.Function4)
        self.assertEqual(overload.drawText(RectF(), ''), Overload.Function4)
        self.assertEqual(overload.drawText(RectF(), Str(), Echo()), Overload.Function4)
        self.assertEqual(overload.drawText(RectF(), '', Echo()), Overload.Function4)
        self.assertEqual(overload.drawText(Rect(), Str()), Overload.Function4)
        self.assertEqual(overload.drawText(Rect(), ''), Overload.Function4)
        self.assertEqual(overload.drawText(Rect(), Str(), Echo()), Overload.Function4)
        self.assertEqual(overload.drawText(Rect(), '', Echo()), Overload.Function4)

    def testDrawTextIntIntStr(self):
        overload = Overload()
        self.assertEqual(overload.drawText(1, 2, Str()), Overload.Function5)
        self.assertEqual(overload.drawText(1, 2, ''), Overload.Function5)

    def testDrawTextIntIntIntIntStr(self):
        overload = Overload()
        self.assertEqual(overload.drawText(1, 2, 3, 4, 5, Str()), Overload.Function6)
        self.assertEqual(overload.drawText(1, 2, 3, 4, 5, ''), Overload.Function6)

    def testDrawText2IntIntIntIntStr(self):
        overload = Overload()
        self.assertEqual(overload.drawText2(1, 2, 3, 4, 5, Str()), Overload.Function6)
        self.assertEqual(overload.drawText2(1, 2, 3, 4, 5, ''), Overload.Function6)
        self.assertEqual(overload.drawText2(1, 2, 3, 4, 5), Overload.Function6)
        self.assertEqual(overload.drawText2(1, 2, 3, 4), Overload.Function6)
        self.assertEqual(overload.drawText2(1, 2, 3), Overload.Function6)

    def testDrawText3(self):
        overload = Overload()
        self.assertEqual(overload.drawText3(Str(), Str(), Str()), Overload.Function0)
        self.assertEqual(overload.drawText3('', '', ''), Overload.Function0)
        self.assertEqual(overload.drawText3(1, 2, 3, 4, 5), Overload.Function1)
        self.assertEqual(overload.drawText3(1, 2, 3, 4, 5), Overload.Function1)

    def testDrawText3Exception(self):
        overload = Overload()
        # NOTE: Using 'try' because assertRaisesRegexp is not available.
        # to check the error text.
        try:
            overload.drawText3(Str(), Str(), Str(), 4, 5)
        except Exception as err:
            self.assertEqual(type(err), TypeError)
            self.assertTrue('called with wrong argument types:' in str(err))

    def testDrawText4(self):
        overload = Overload()
        self.assertEqual(overload.drawText4(1, 2, 3), Overload.Function0)
        self.assertEqual(overload.drawText4(1, 2, 3, 4, 5), Overload.Function1)


if __name__ == '__main__':
    unittest.main()

