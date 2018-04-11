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
import sample
from py3kcompat import IS_PY3K

class OldStyle:
    pass

class NewStyle(object):
    pass

class OldStyleNumber:
    def __init__(self, value):
        self.value = value
    def __trunc__(self):
        return self.value

class NewStyleNumber(object):
    def __init__(self, value):
        self.value = value
    def __int__(self):
        return int(self.value)
    def __trunc__(self):
        return self.value

class TestOldStyleClassAsNumber(unittest.TestCase):

    def testBasic(self):
        '''For the sake of calibration...'''
        self.assertEqual(sample.acceptInt(123), 123)

    def testOldStyleClassPassedAsInteger(self):
        '''Old-style classes aren't numbers and shouldn't be accepted.'''
        obj = OldStyle()
        self.assertRaises(TypeError, sample.acceptInt, obj)

    def testNewStyleClassPassedAsInteger(self):
        '''New-style classes aren't numbers and shouldn't be accepted.'''
        obj = NewStyle()
        self.assertRaises(TypeError, sample.acceptInt, obj)

    def testOldStyleClassWithNumberProtocol(self):
        obj = OldStyleNumber(123)
        self.assertEqual(sample.acceptInt(obj), obj.value)

    def testNewStyleClassWithNumberProtocol(self):
        obj = NewStyleNumber(123)
        self.assertEqual(sample.acceptInt(obj), obj.value)

if __name__ == "__main__" and not IS_PY3K:
    unittest.main()

