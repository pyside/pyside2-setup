# This file is part of PySide: Python for Qt
#
# Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
#
# Contact: PySide team <contact@pyside.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA

import unittest
from PySide import QtCore, QtGui
from helper import UsesQApplication


class TestBugPYSIDE189(UsesQApplication):

    def testDisconnect(self):
        # Disconnecting from a signal owned by a destroyed object
        # should raise an exception, not segfault.
        def onValueChanged(self, value):
            pass

        sld = QtGui.QSlider()
        sld.valueChanged.connect(onValueChanged)

        sld.deleteLater()

        QtCore.QTimer.singleShot(0, self.app.quit)
        self.app.exec_()

        self.assertRaises(RuntimeError, sld.valueChanged.disconnect, onValueChanged)


if __name__ == '__main__':
    unittest.main()
