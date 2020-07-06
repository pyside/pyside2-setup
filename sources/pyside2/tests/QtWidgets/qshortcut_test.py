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

''' Test the QShortcut constructor'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import Qt, QTimer
from PySide2.QtGui import QKeySequence, QShortcut
from PySide2.QtWidgets import QApplication, QWidget

class Foo(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.ok = False
        self.copy = False

    def slot_of_foo(self):
        self.ok = True

    def slot_of_copy(self):
        self.copy = True

class MyShortcut(QShortcut):
    def __init__(self, keys, wdg, slot):
        QShortcut.__init__(self, keys, wdg, slot)

    def emit_signal(self):
        self.activated.emit()

class QAppPresence(unittest.TestCase):

    def testQShortcut(self):
        self.qapp = QApplication([])
        f = Foo()

        self.sc = MyShortcut(QKeySequence(Qt.Key_Return), f, f.slot_of_foo)
        self.scstd = MyShortcut(QKeySequence.Copy, f, f.slot_of_copy)
        QTimer.singleShot(0, self.init);
        self.qapp.exec_()
        self.assertEqual(f.ok, True)
        self.assertEqual(f.copy, True)

    def init(self):
        self.sc.emit_signal();
        self.scstd.emit_signal();
        self.qapp.quit()

if __name__ == '__main__':
    unittest.main()
