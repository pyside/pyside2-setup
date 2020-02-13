#!/usr/bin/python
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

'''Test cases for QObject.sender()'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import *
from helper import UsesQCoreApplication

class ExtQTimer(QTimer):
    def __init__(self):
        QTimer.__init__(self)

class Receiver(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.the_sender = None

    def callback(self):
        self.the_sender = self.sender()
        if QCoreApplication.instance():
            QCoreApplication.instance().exit()

class ObjectSenderTest(unittest.TestCase):
    '''Test case for QObject.sender() method.'''

    def testSenderPythonSignal(self):
        sender = QObject()
        recv = Receiver()
        QObject.connect(sender, SIGNAL('foo()'), recv.callback)
        sender.emit(SIGNAL('foo()'))
        self.assertEqual(sender, recv.the_sender)

class ObjectSenderCheckOnReceiverTest(unittest.TestCase):
    '''Test case for QObject.sender() method, this one tests the equality on the Receiver object.'''

    def testSenderPythonSignal(self):
        sender = QObject()
        recv = Receiver()
        QObject.connect(sender, SIGNAL('foo()'), recv.callback)
        sender.emit(SIGNAL('foo()'))
        self.assertEqual(sender, recv.the_sender)

class ObjectSenderWithQAppTest(UsesQCoreApplication):
    '''Test case for QObject.sender() method with QApplication.'''

    def testSenderCppSignal(self):
        sender = QTimer()
        sender.setObjectName('foo')
        recv = Receiver()
        QObject.connect(sender, SIGNAL('timeout()'), recv.callback)
        sender.start(10)
        self.app.exec_()
        self.assertEqual(sender, recv.the_sender)

    def testSenderCppSignalSingleShotTimer(self):
        recv = Receiver()
        QTimer.singleShot(10, recv.callback)
        self.app.exec_()
        self.assertTrue(isinstance(recv.the_sender, QObject))

    def testSenderCppSignalWithPythonExtendedClass(self):
        sender = ExtQTimer()
        recv = Receiver()
        QObject.connect(sender, SIGNAL('timeout()'), recv.callback)
        sender.start(10)
        self.app.exec_()
        self.assertEqual(sender, recv.the_sender)

class ObjectSenderWithQAppCheckOnReceiverTest(UsesQCoreApplication):
    '''Test case for QObject.sender() method with QApplication.'''

    def testSenderCppSignal(self):
        sender = QTimer()
        sender.setObjectName('foo')
        recv = Receiver()
        QObject.connect(sender, SIGNAL('timeout()'), recv.callback)
        sender.start(10)
        self.app.exec_()
        self.assertEqual(sender, recv.the_sender)

    def testSenderCppSignalWithPythonExtendedClass(self):
        sender = ExtQTimer()
        recv = Receiver()
        QObject.connect(sender, SIGNAL('timeout()'), recv.callback)
        sender.start(10)
        self.app.exec_()
        self.assertEqual(sender, recv.the_sender)

if __name__ == '__main__':
    unittest.main()

