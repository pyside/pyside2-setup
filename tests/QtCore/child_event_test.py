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

'''Test case for QObject.childEvent and QTimer.childEvent overloading'''

import unittest
from time import sleep
from PySide2.QtCore import QObject, QTimer, QCoreApplication

from helper import UsesQCoreApplication

class ExtQObject(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.child_event_received = False

    def childEvent(self, event):
        QObject.childEvent(self, event)
        self.child_event_received = True

class ExtQTimer(QTimer):
    def __init__(self):
        QTimer.__init__(self)
        self.child_event_received = False

    def childEvent(self, event):
        QTimer.childEvent(self, event)
        self.child_event_received = True


class TestChildEvent(UsesQCoreApplication):
    '''Test case for QObject::childEvent and QTimer::childEvent'''

    def setUp(self):
        UsesQCoreApplication.setUp(self)

    def tearDown(self):
        UsesQCoreApplication.tearDown(self)

    def testQObject(self):
        parent = ExtQObject()
        child = QObject()
        child.setParent(parent)
        self.assertTrue(parent.child_event_received)

    def testQTimer(self):
        parent = ExtQTimer()
        child = QObject()
        child.setParent(parent)
        self.assertTrue(parent.child_event_received)

if __name__ == '__main__':
    unittest.main()

