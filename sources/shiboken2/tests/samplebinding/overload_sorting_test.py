#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

'''Test cases for overload sorting'''

import unittest

from sample import *

class Dummy(object):
    pass

class SimpleOverloadSorting(unittest.TestCase):

    def setUp(self):
        self.obj = SortedOverload()

    def testIntDouble(self):
        '''Overloads with int and double'''
        self.assertEqual(self.obj.overload(3), "int")
        self.assertEqual(self.obj.overload(3.14), "double")

    def testImplicitConvert(self):
        '''Overloads with implicit convertible types'''
        self.assertEqual(self.obj.overload(ImplicitTarget()), "ImplicitTarget")
        self.assertEqual(self.obj.overload(ImplicitBase()), "ImplicitBase")

    def testContainer(self):
        '''Overloads with containers arguments'''
        self.assertEqual(self.obj.overload([ImplicitBase()]), "list(ImplicitBase)")

    def testPyObject(self):
        '''Overloads with PyObject args'''
        self.assertEqual(self.obj.overload(Dummy()), "PyObject")

    def testImplicitOnly(self):
        '''Passing an implicit convertible object to an overload'''
        self.assertTrue(self.obj.implicit_overload(ImplicitTarget()))

    def testPyObjectSort(self):
        self.assertEqual(self.obj.pyObjOverload(1, 2), "int,int")
        self.assertEqual(self.obj.pyObjOverload(object(), 2), "PyObject,int")


class DeepOverloadSorting(unittest.TestCase):

    def setUp(self):
        self.obj = SortedOverload()

    def testPyObject(self):
        '''Deep Overload - (int, PyObject *)'''
        self.assertEqual(self.obj.overloadDeep(1, Dummy()), "PyObject")

    def testImplicit(self):
        '''Deep Overload - (int, ImplicitBase *)'''
        self.assertEqual(self.obj.overloadDeep(1, ImplicitBase()), "ImplicitBase")

class EnumOverIntSorting(unittest.TestCase):
    def testEnumOverInt(self):
        ic = ImplicitConv(ImplicitConv.CtorTwo)
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorTwo)

if __name__ == '__main__':
    unittest.main()
