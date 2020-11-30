#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

'''Test cases for snake case generation'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import (SnakeCaseTest, SnakeCaseDerivedTest,
                    snake_case_global_function)


class OverrideTest(SnakeCaseDerivedTest):
    def virtual_func(self):
        return 4711


class SnakeCaseTestCase(unittest.TestCase):
    '''Test for SnakeCaseTest'''
    def testMemberFunctions(self):
        s = SnakeCaseTest()
        self.assertEqual(s.test_function1(), 42)

        self.assertEqual(s.testFunctionDisabled(), 42)

        self.assertEqual(s.testFunctionBoth(), 42)
        self.assertEqual(s.test_function_both(), 42)

    def virtualFunctions(self):
        s = OverrideTest()
        self.assertEqual(s.call_virtual_func(), 4711)

    def testMemberFields(self):
        s = SnakeCaseTest()
        old_value = s.test_field
        s.test_field = old_value + 1
        self.assertEqual(s.test_field, old_value + 1)

        old_value = s.testFieldDisabled
        s.testFieldDisabled = old_value + 1
        self.assertEqual(s.testFieldDisabled, old_value + 1)

        old_value = s.test_field_both
        s.test_field_both = old_value + 1
        self.assertEqual(s.test_field_both, old_value + 1)
        self.assertEqual(s.testFieldBoth, old_value + 1)

    def testGlobalFunction(self):
        self.assertEqual(snake_case_global_function(), 42)


if __name__ == '__main__':
    unittest.main()
