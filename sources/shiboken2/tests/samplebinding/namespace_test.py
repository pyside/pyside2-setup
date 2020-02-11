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

'''Test cases for std::map container conversions'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import *


class TestEnumUnderNamespace(unittest.TestCase):
    def testInvisibleNamespace(self):
        o1 = EnumOnNamespace.Option1
        self.assertEqual(o1, 1)

class TestClassesUnderNamespace(unittest.TestCase):
    def testIt(self):
        c1 = SampleNamespace.SomeClass()
        e1 = SampleNamespace.SomeClass.ProtectedEnum()
        c2 = SampleNamespace.SomeClass.SomeInnerClass()
        e2 = SampleNamespace.SomeClass.SomeInnerClass.ProtectedEnum()
        c3 = SampleNamespace.SomeClass.SomeInnerClass.OkThisIsRecursiveEnough()
        e3 = SampleNamespace.SomeClass.SomeInnerClass.OkThisIsRecursiveEnough.NiceEnum()

    def testFunctionAddedOnNamespace(self):
        res = SampleNamespace.ImInsideANamespace(2, 2)
        self.assertEqual(res, 4)

    def testTpNames(self):
        self.assertEqual(str(SampleNamespace.SomeClass),
            "<class 'sample.SampleNamespace.SomeClass'>")
        self.assertEqual(str(SampleNamespace.SomeClass.ProtectedEnum),
            "<class 'sample.SampleNamespace.SomeClass.ProtectedEnum'>")
        self.assertEqual(str(SampleNamespace.SomeClass.SomeInnerClass.ProtectedEnum),
            "<class 'sample.SampleNamespace.SomeClass.SomeInnerClass.ProtectedEnum'>")
        self.assertEqual(str(SampleNamespace.SomeClass.SomeInnerClass.OkThisIsRecursiveEnough),
            "<class 'sample.SampleNamespace.SomeClass.SomeInnerClass.OkThisIsRecursiveEnough'>")
        self.assertEqual(str(SampleNamespace.SomeClass.SomeInnerClass.OkThisIsRecursiveEnough.NiceEnum),
            "<class 'sample.SampleNamespace.SomeClass.SomeInnerClass.OkThisIsRecursiveEnough.NiceEnum'>")

if __name__ == '__main__':
    unittest.main()
