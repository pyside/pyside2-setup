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

import unittest

from helper import UsesQApplication
from PySide2.QtWidgets import QPushButton, QMenu, QWidget
from PySide2.QtCore import QTimer

class MyWidget(QWidget):
    def __init__(self):
        QWidget.__init__(self)

        m = QMenu(self)
        b = QPushButton("Hello", self)
        b.setMenu(m)


class QPushButtonTest(UsesQApplication):
    def createMenu(self, button):
        m = QMenu()
        button.setMenu(m)

    def testSetMenu(self):
        w = MyWidget()
        w.show()

        timer = QTimer.singleShot(100, self.app.quit)
        self.app.exec_()

    def buttonCb(self, checked):
        self._clicked = True

    def testBoolinSignal(self):
        b = QPushButton()
        b.setCheckable(True)
        self._clicked = False
        b.toggled[bool].connect(self.buttonCb)
        b.toggle()
        self.assertTrue(self._clicked)

if __name__ == '__main__':
    unittest.main()

