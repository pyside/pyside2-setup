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

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QCoreApplication, QTimer, QEvent, Qt
from PySide6.QtWidgets import QApplication, QSpinBox
from PySide6.QtGui import QKeyEvent

class MySpinBox(QSpinBox):

    def validate(self,text,pos):
        return QSpinBox.validate(self,text,pos)

class TestBug879 (unittest.TestCase):

    def testIt(self):
        app = QApplication([])
        self.box = MySpinBox()
        self.box.show()

        QTimer.singleShot(0, self.sendKbdEvent)
        QTimer.singleShot(100, app.quit)
        app.exec_()

        self.assertEqual(self.box.text(), '0')

    def sendKbdEvent(self):
        ev = QKeyEvent(QEvent.KeyPress, Qt.Key_A, Qt.NoModifier, 'a')
        QCoreApplication.sendEvent(self.box, ev)

if __name__ == '__main__':
    unittest.main()
