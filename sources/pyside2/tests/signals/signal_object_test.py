#!/usr/bin/env python

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

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QTimer, Signal, QObject, Slot, Qt
from helper.usesqcoreapplication import UsesQCoreApplication

class MyObject(QTimer):
    sig1 = Signal()
    sig2 = Signal(int, name='rangeChanged')
    sig3 = Signal(int)
    sig4 = Signal((int,), (str,))
    sig5 = Signal((str,), (int,))
    sig6 = Signal(QObject)

    @Slot(int)
    def myRange(self, r):
        self._range = r

    def slot1(self):
        self._called = True

    def slotString(self, s):
        self._s = s

    def slotObject(self, o):
        self._o = o


class SignalObjectTest(UsesQCoreApplication):
    def cb(self):
        self._cb_called = True
        self.app.exit()

    def testsingleConnect(self):
        o = MyObject()
        o.sig1.connect(o.slot1)
        o.sig1.emit()
        self.assertTrue(o._called)

    def testSignalWithArgs(self):
        o = MyObject()
        o.sig3.connect(o.myRange)
        o.sig3.emit(10)
        self.assertEqual(o._range, 10)

    def testSignatureParse(self):
        o = MyObject()
        o.sig2.connect(o.myRange)
        o.sig2.emit(10)

    def testDictOperator(self):
        o = MyObject()
        o.sig4[str].connect(o.slotString)
        o.sig4[str].emit("PySide")
        self.assertEqual(o._s, "PySide")

    def testGeneretedSignal(self):
        o = MyObject()
        o.timeout.connect(self.cb)
        o.start(100)
        self.app.exec_()
        self.assertTrue(self._cb_called)

    def testConnectionType(self):
        o = MyObject()
        o.timeout.connect(self.cb, type=Qt.DirectConnection)
        o.start(100)
        self.app.exec_()
        self.assertTrue(self._cb_called)

    def testSignalWithSignal(self):
        o = MyObject()
        o.sig2.connect(o.myRange)
        o.sig5.connect(o.sig2)
        o.sig5[int].emit(10)
        self.assertEqual(o._range, 10)

    def testSignalWithObject(self):
        o = MyObject()
        o.sig6.connect(o.slotObject)
        arg = QObject()
        o.sig6.emit(arg)
        self.assertEqual(arg, o._o)

if __name__ == '__main__':
    unittest.main()
