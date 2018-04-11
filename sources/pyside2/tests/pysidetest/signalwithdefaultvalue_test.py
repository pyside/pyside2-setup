#!/usr/bin/python

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
from testbinding import TestObject
from PySide2.QtCore import QObject, SIGNAL

'''Tests the behaviour of signals with default values.'''

class SignalWithDefaultValueTest(unittest.TestCase):

    def setUp(self):
        self.obj = TestObject(0)
        self.void_called = False
        self.bool_called = False

    def tearDown(self):
        del self.obj
        del self.void_called
        del self.bool_called

    def testConnectNewStyleEmitVoidSignal(self):
        def callbackVoid():
            self.void_called = True
        def callbackBool(value):
            self.bool_called = True
        self.obj.signalWithDefaultValue.connect(callbackVoid)
        self.obj.signalWithDefaultValue[bool].connect(callbackBool)
        self.obj.emitSignalWithDefaultValue_void()
        self.assertTrue(self.void_called)
        self.assertTrue(self.bool_called)

    def testConnectNewStyleEmitBoolSignal(self):
        def callbackVoid():
            self.void_called = True
        def callbackBool(value):
            self.bool_called = True
        self.obj.signalWithDefaultValue.connect(callbackVoid)
        self.obj.signalWithDefaultValue[bool].connect(callbackBool)
        self.obj.emitSignalWithDefaultValue_bool()
        self.assertTrue(self.void_called)
        self.assertTrue(self.bool_called)

    def testConnectOldStyleEmitVoidSignal(self):
        def callbackVoid():
            self.void_called = True
        def callbackBool(value):
            self.bool_called = True
        QObject.connect(self.obj, SIGNAL('signalWithDefaultValue()'), callbackVoid)
        QObject.connect(self.obj, SIGNAL('signalWithDefaultValue(bool)'), callbackBool)
        self.obj.emitSignalWithDefaultValue_void()
        self.assertTrue(self.void_called)
        self.assertTrue(self.bool_called)

    def testConnectOldStyleEmitBoolSignal(self):
        def callbackVoid():
            self.void_called = True
        def callbackBool(value):
            self.bool_called = True
        QObject.connect(self.obj, SIGNAL('signalWithDefaultValue()'), callbackVoid)
        QObject.connect(self.obj, SIGNAL('signalWithDefaultValue(bool)'), callbackBool)
        self.obj.emitSignalWithDefaultValue_bool()
        self.assertTrue(self.void_called)
        self.assertTrue(self.bool_called)

if __name__ == '__main__':
    unittest.main()

