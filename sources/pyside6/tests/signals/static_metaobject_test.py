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

"""Tests covering signal emission and receiving to python slots"""

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QObject, SIGNAL, Slot
from helper.usesqcoreapplication import UsesQCoreApplication

class MyObject(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self._slotCalledCount = 0

    # this '@Slot()' is needed to get the right sort order in testSharedSignalEmission.
    # For some reason, it also makes the tests actually work!
    @Slot()
    def mySlot(self):
        self._slotCalledCount = self._slotCalledCount + 1


class StaticMetaObjectTest(UsesQCoreApplication):

    def testSignalPropagation(self):
        o = MyObject()
        o2 = MyObject()

        # SIGNAL foo not created yet
        self.assertEqual(o.metaObject().indexOfSignal("foo()"), -1)

        o.connect(SIGNAL("foo()"), o2.mySlot)
        # SIGNAL foo create after connect
        self.assertTrue(o.metaObject().indexOfSignal("foo()") > 0)

        # SIGNAL does not propagate to others objects of the same type
        self.assertEqual(o2.metaObject().indexOfSignal("foo()"), -1)

        del o
        del o2
        o = MyObject()
        # The SIGNAL was destroyed with old objects
        self.assertEqual(o.metaObject().indexOfSignal("foo()"), -1)


    def testSharedSignalEmission(self):
        o = QObject()
        m = MyObject()

        o.connect(SIGNAL("foo2()"), m.mySlot)
        m.connect(SIGNAL("foo2()"), m.mySlot)
        o.emit(SIGNAL("foo2()"))
        self.assertEqual(m._slotCalledCount, 1)
        del o
        m.emit(SIGNAL("foo2()"))
        self.assertEqual(m._slotCalledCount, 2)

if __name__ == '__main__':
    unittest.main()
