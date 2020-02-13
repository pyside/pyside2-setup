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

from PySide2 import QtCore, QtWidgets

from helper import TimedQApplication

class Signaller(QtCore.QObject):
    s1 = QtCore.Signal()
    s2 = QtCore.Signal()
    s3 = QtCore.Signal()

class Window(object):

    def __init__(self, s):
        self._window = QtWidgets.QMainWindow()
        self._window.setAttribute(QtCore.Qt.WA_DeleteOnClose, True)
        self._window.setWindowTitle("Demo!")

        self._s = s
        self._s.s1.connect(self._on_signal)
        self._s.s2.connect(self._on_signal)

    def show(self):
        self._window.show()

    def _on_signal(self):
        self._window.setWindowTitle("Signaled!")

class TestTimedApp(TimedQApplication):
    def testSignals(self):
        s = Signaller()
        w = Window(s)
        w.show()

        def midleFunction():
            def internalFunction():
                pass
            s.s3.connect(internalFunction)

        midleFunction()
        self.app.exec_()
        del w

        s.s1.emit()
        s.s2.emit()
        s.s3.emit()

if __name__ == '__main__':
    unittest.main()
