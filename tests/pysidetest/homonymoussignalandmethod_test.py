#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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
from testbinding import TestObject

'''Tests the behaviour of homonymous signals and slots.'''

class HomonymousSignalAndMethodTest(unittest.TestCase):

    def setUp(self):
        self.value = 123
        self.called = False
        self.obj = TestObject(self.value)

    def tearDown(self):
        del self.value
        del self.called
        del self.obj

    def testIdValueSignalEmission(self):
        def callback(idValue):
            self.assertEqual(idValue, self.value)
        self.obj.idValue.connect(callback)
        self.obj.emitIdValueSignal()

    def testStaticMethodDoubleSignalEmission(self):
        def callback():
            self.called = True
        self.obj.staticMethodDouble.connect(callback)
        self.obj.emitStaticMethodDoubleSignal()
        self.assertTrue(self.called)

    def testSignalNotCallable(self):
        self.assertRaises(TypeError, self.obj.justASignal)

    def testCallingInstanceMethodWithArguments(self):
        self.assertRaises(TypeError, TestObject.idValue, 1)

    def testCallingInstanceMethodWithoutArguments(self):
        self.assertRaises(TypeError, TestObject.idValue)

    def testHomonymousSignalAndMethod(self):
        self.assertEqual(self.obj.idValue(), self.value)

    def testHomonymousSignalAndStaticMethod(self):
        self.assertEqual(TestObject.staticMethodDouble(3), 6)

    def testHomonymousSignalAndStaticMethodFromInstance(self):
        self.assertEqual(self.obj.staticMethodDouble(4), 8)

if __name__ == '__main__':
    unittest.main()

