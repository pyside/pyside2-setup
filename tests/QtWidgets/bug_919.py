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

from helper import TimedQApplication
from PySide2.QtGui import QPainter
from PySide2.QtWidgets import QPushButton, QStyleOptionButton, QApplication, QStyle

class MyWidget(QPushButton):
    def __init__(self, parent = None):
        QPushButton.__init__(self, parent)
        self._painted = False

    def paintEvent(self, e):
        p = QPainter(self)
        style = QApplication.style()
        option = QStyleOptionButton()
        style.drawControl(QStyle.CE_PushButton, option, p)
        self._painted = True


class TestBug919(TimedQApplication):
    def testFontInfo(self):
        w = MyWidget()
        w.show()
        self.app.exec_()
        self.assert_(w._painted)

if __name__ == '__main__':
    unittest.main()
