#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

import unittest
from minimal import *
from py3kcompat import IS_PY3K

try:
    import numpy as np
except ImportError as e:
    print(e)
    np = None


if IS_PY3K:
    import functools
    reduce = functools.reduce


class TypedefTest(unittest.TestCase):

    def setUp(self):
        self.the_size = 8

    def test_arrayFuncInt(self):
        none = ()
        full = range(self.the_size)
        self.assertTrue(arrayFuncInt(none), "None is empty, arrayFuncInt should return true")
        self.assertFalse(arrayFuncInt(full), "Full is NOT empty, arrayFuncInt should return false")

        self.assertTrue(arrayFuncInt(np.array(none)), "None is empty, arrayFuncInt should return true")
        self.assertFalse(arrayFuncInt(np.array(full)), "Full is NOT empty, arrayFuncInt should return false")

    def test_arrayFuncIntTypedef(self):
        none = ()
        full = (1, 2, 3)
        self.assertTrue(arrayFuncIntTypedef(none), "None is empty, arrayFuncIntTypedef should return true")
        self.assertFalse(arrayFuncIntTypedef(full), "Full is NOT empty, arrayFuncIntTypedef should return false")

        self.assertTrue(arrayFuncIntTypedef(np.array(none)), "None is empty, arrayFuncIntTypedef should return true")
        self.assertFalse(arrayFuncIntTypedef(np.array(full)), "Full is NOT empty, arrayFuncIntTypedef should return false")

    def test_arrayFuncIntReturn(self):
        none = arrayFuncIntReturn(0)
        full = arrayFuncIntReturn(self.the_size)
        self.assertTrue((len(none) == 0), "none should be empty")
        self.assertTrue((len(full) == self.the_size), "full should have " + str(self.the_size) + " elements")

    def test_arrayFuncIntReturnTypedef(self):
        none = arrayFuncIntReturnTypedef(0)
        full = arrayFuncIntReturnTypedef(self.the_size)
        self.assertTrue((len(none) == 0), "none should be empty")
        self.assertTrue((len(full) == self.the_size), "full should have " + str(self.the_size) + " elements")

    def test_arrayFunc(self):
        none = ()
        full = range(self.the_size)
        self.assertTrue(arrayFunc(none), "None is empty, arrayFunc should return true")
        self.assertFalse(arrayFunc(full), "Full is NOT empty, arrayFunc should return false")

        self.assertTrue(arrayFunc(np.array(none)), "None is empty, arrayFunc should return true")
        self.assertFalse(arrayFunc(np.array(full)), "Full is NOT empty, arrayFunc should return false")

    def test_arrayFuncTypedef(self):
        none = ()
        full = (1, 2, 3)
        self.assertTrue(arrayFuncTypedef(none), "None is empty, arrayFuncTypedef should return true")
        self.assertFalse(arrayFuncTypedef(full), "Full is NOT empty, arrayFuncTypedef should return false")

        self.assertTrue(arrayFuncTypedef(np.array(none)), "None is empty, arrayFuncTypedef should return true")
        self.assertFalse(arrayFuncTypedef(np.array(full)), "Full is NOT empty, arrayFuncTypedef should return false")

    def test_arrayFuncReturn(self):
        none = arrayFuncReturn(0)
        full = arrayFuncReturn(self.the_size)
        self.assertTrue((len(none) == 0), "none should be empty")
        self.assertTrue((len(full) == self.the_size), "full should have " + str(self.the_size) + " elements")

    def test_arrayFuncReturnTypedef(self):
        none = arrayFuncReturnTypedef(0)
        full = arrayFuncReturnTypedef(self.the_size)
        self.assertTrue((len(none) == 0), "none should be empty")
        self.assertTrue((len(full) == self.the_size), "full should have " + str(self.the_size) + " elements")


if __name__ == '__main__':
    if np != None:
        unittest.main()
