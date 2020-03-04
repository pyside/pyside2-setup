#!/usr/bin/python

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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(True)

from testbinding import TestObject
from PySide2.QtCore import QObject, SIGNAL

'''Tests the behaviour of signals with default values when emitted from Python.'''

class SignalEmissionFromPython(unittest.TestCase):

    def setUp(self):
        self.obj1 = TestObject(0)
        self.obj2 = TestObject(0)
        self.one_called = 0
        self.two_called = 0

    def tearDown(self):
        del self.obj1
        del self.obj2
        del self.one_called
        del self.two_called

    def testConnectNewStyleEmitVoidSignal(self):
        def callbackOne():
            self.one_called += 1
            self.obj2.signalWithDefaultValue.emit()
        def callbackTwo():
            self.two_called += 1
        self.obj1.signalWithDefaultValue.connect(callbackOne)
        self.obj2.signalWithDefaultValue.connect(callbackTwo)
        self.obj1.emitSignalWithDefaultValue_void()
        self.obj2.emitSignalWithDefaultValue_void()
        self.assertEqual(self.one_called, 1)
        self.assertEqual(self.two_called, 2)

    def testConnectOldStyleEmitVoidSignal(self):
        def callbackOne():
            self.one_called += 1
            self.obj2.signalWithDefaultValue.emit()
        def callbackTwo():
            self.two_called += 1
        QObject.connect(self.obj1, SIGNAL('signalWithDefaultValue()'), callbackOne)
        QObject.connect(self.obj2, SIGNAL('signalWithDefaultValue()'), callbackTwo)
        self.obj1.emitSignalWithDefaultValue_void()
        self.obj2.emitSignalWithDefaultValue_void()
        self.assertEqual(self.one_called, 1)
        self.assertEqual(self.two_called, 2)

    def testConnectNewStyleEmitBoolSignal(self):
        def callbackOne():
            self.one_called += 1
            self.obj2.signalWithDefaultValue[bool].emit(True)
        def callbackTwo():
            self.two_called += 1
        self.obj1.signalWithDefaultValue.connect(callbackOne)
        self.obj2.signalWithDefaultValue.connect(callbackTwo)
        self.obj1.emitSignalWithDefaultValue_void()
        self.obj2.emitSignalWithDefaultValue_void()
        self.assertEqual(self.one_called, 1)
        self.assertEqual(self.two_called, 2)

    def testConnectOldStyleEmitBoolSignal(self):
        def callbackOne():
            self.one_called += 1
            self.obj2.signalWithDefaultValue[bool].emit(True)
        def callbackTwo():
            self.two_called += 1
        QObject.connect(self.obj1, SIGNAL('signalWithDefaultValue()'), callbackOne)
        QObject.connect(self.obj2, SIGNAL('signalWithDefaultValue()'), callbackTwo)
        self.obj1.emitSignalWithDefaultValue_void()
        self.obj2.emitSignalWithDefaultValue_void()
        self.assertEqual(self.one_called, 1)
        self.assertEqual(self.two_called, 2)



if __name__ == '__main__':
    unittest.main()

