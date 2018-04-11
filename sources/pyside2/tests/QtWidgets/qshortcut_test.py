# -*- coding: utf-8 -*-

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

''' Test the QShortcut constructor'''

import unittest
import sys

from PySide2 import QtCore, QtGui, QtWidgets

class Foo(QtWidgets.QWidget):
    def __init__(self):
        QtWidgets.QWidget.__init__(self)
        self.ok = False
        self.copy = False

    def slot_of_foo(self):
        self.ok = True

    def slot_of_copy(self):
        self.copy = True

class MyShortcut(QtWidgets.QShortcut):
    def __init__(self, keys, wdg, slot):
        QtWidgets.QShortcut.__init__(self, keys, wdg, slot)

    def emit_signal(self):
        self.emit(QtCore.SIGNAL("activated()"))

class QAppPresence(unittest.TestCase):

    def testQShortcut(self):
        self.qapp = QtWidgets.QApplication([])
        f = Foo()

        self.sc = MyShortcut(QtGui.QKeySequence(QtCore.Qt.Key_Return), f, f.slot_of_foo)
        self.scstd = MyShortcut(QtGui.QKeySequence.Copy, f, f.slot_of_copy)
        QtCore.QTimer.singleShot(0, self.init);
        self.qapp.exec_()
        self.assertEqual(f.ok, True)
        self.assertEqual(f.copy, True)

    def init(self):
        self.sc.emit_signal();
        self.scstd.emit_signal();
        self.qapp.quit()

if __name__ == '__main__':
    unittest.main()
