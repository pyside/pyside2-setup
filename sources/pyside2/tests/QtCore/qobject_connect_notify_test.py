# -*- coding: utf-8 -*-

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

''' Test case for QObject::connectNotify()'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import *
from helper import UsesQCoreApplication

def cute_slot():
    pass

class Obj(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.con_notified = False
        self.dis_notified = False
        self.signal = ""

    def connectNotify(self, signal):
        self.con_notified = True
        self.signal = signal

    def disconnectNotify(self, signal):
        self.dis_notified = True

    def reset(self):
        self.con_notified = False
        self.dis_notified = False

class TestQObjectConnectNotify(UsesQCoreApplication):
    '''Test case for QObject::connectNotify'''
    def setUp(self):
        UsesQCoreApplication.setUp(self)
        self.called = False

    def tearDown(self):
        UsesQCoreApplication.tearDown(self)

    def testBasic(self):
        sender = Obj()
        receiver = QObject()
        sender.connect(SIGNAL("destroyed()"), receiver, SLOT("deleteLater()"))
        self.assertTrue(sender.con_notified)
        # When connecting to a regular slot, and not a python callback function, QObject::connect
        # will use the non-cloned method signature, so connecting to destroyed() will actually
        # connect to destroyed(QObject*).
        self.assertEqual(sender.signal.methodSignature(), "destroyed(QObject*)")
        sender.disconnect(SIGNAL("destroyed()"), receiver, SLOT("deleteLater()"))
        self.assertTrue(sender.dis_notified)

    def testPySignal(self):
        sender = Obj()
        receiver = QObject()
        sender.connect(SIGNAL("foo()"), receiver, SLOT("deleteLater()"))
        self.assertTrue(sender.con_notified)
        sender.disconnect(SIGNAL("foo()"), receiver, SLOT("deleteLater()"))
        self.assertTrue(sender.dis_notified)

    def testPySlots(self):
        sender = Obj()
        receiver = QObject()
        sender.connect(SIGNAL("destroyed()"), cute_slot)
        self.assertTrue(sender.con_notified)
        sender.disconnect(SIGNAL("destroyed()"), cute_slot)
        self.assertTrue(sender.dis_notified)

    def testpyAll(self):
        sender = Obj()
        receiver = QObject()
        sender.connect(SIGNAL("foo()"), cute_slot)
        self.assertTrue(sender.con_notified)
        sender.disconnect(SIGNAL("foo()"), cute_slot)
        self.assertTrue(sender.dis_notified)

if __name__ == '__main__':
    unittest.main()
