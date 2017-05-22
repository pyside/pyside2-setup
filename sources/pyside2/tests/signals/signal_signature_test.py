# -*- coding: utf-8 -*-

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

'''Test case for signal signature received by QObject::connectNotify().'''

import unittest
from PySide2.QtCore import *
from helper import UsesQCoreApplication

class Obj(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.signal = ''

    def connectNotify(self, signal):
        self.signal = signal

def callback(arg=None):
    pass

class TestConnectNotifyWithNewStyleSignals(UsesQCoreApplication):
    '''Test case for signal signature received by QObject::connectNotify().'''

    def testOldStyle(self):
        sender = Obj()
        receiver = QObject()
        sender.connect(SIGNAL('destroyed()'), receiver, SLOT('deleteLater()'))
        # When connecting to a regular slot, and not a python callback function, QObject::connect
        # will use the non-cloned method signature, so connectinc to destroyed() will actually
        # connect to destroyed(QObject*).
        self.assertEqual(sender.signal.methodSignature(), 'destroyed(QObject*)')

    def testOldStyleWithPythonCallback(self):
        sender = Obj()
        sender.connect(SIGNAL('destroyed()'), callback)
        self.assertEqual(sender.signal.methodSignature(), 'destroyed()')

    def testNewStyle(self):
        sender = Obj()

        sender.destroyed.connect(callback)
        self.assertEqual(sender.signal.methodSignature(), 'destroyed()')

        sender.destroyed[QObject].connect(callback)
        self.assertEqual(sender.signal.methodSignature(), 'destroyed(QObject*)')

if __name__ == '__main__':
    unittest.main()

