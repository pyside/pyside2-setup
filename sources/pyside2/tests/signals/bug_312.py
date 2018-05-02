#!/usr/bin/env python
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

import unittest
import sys
from PySide2.QtCore import QObject, SIGNAL

MAX_LOOPS = 5
MAX_OBJECTS = 200

class Dummy(object):
    def __init__(self, parent):
        self._parent = parent

    def callback(self):
        self._called = True

class MultipleSlots(unittest.TestCase):
    def myCB(self):
        self._count += 1

    """
    def testUnboundSignal(self):
        o = QObject()
        self._count  = 0
        for i in range(MAX_OBJECTS):
            QObject.connect(o, SIGNAL("fire()"), lambda: self.myCB())

        o.emit(SIGNAL("fire()"))
        self.assertEqual(self._count, MAX_OBJECTS)

    """
    def testDisconnectCleanup(self):
        for c in range(MAX_LOOPS):
            self._count = 0
            self._senders = []
            for i in range(MAX_OBJECTS):
                o = QObject()
                QObject.connect(o, SIGNAL("fire()"), lambda: self.myCB())
                self._senders.append(o)
                o.emit(SIGNAL("fire()"))

            self.assertEqual(self._count, MAX_OBJECTS)

            #delete all senders will disconnect the signals
            self._senders = []

if __name__ == '__main__':
    unittest.main()


