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

from helper import UsesQApplication

from PySide2.QtCore import Qt, QTimer
from PySide2.QtGui import QPen, QPainter
from PySide2.QtWidgets import QWidget

class Painting(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.penFromEnum = None
        self.penFromInteger = None

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setPen(Qt.NoPen)
        self.penFromEnum = painter.pen()
        painter.setPen(int(Qt.NoPen))
        self.penFromInteger = painter.pen()


class QPenTest(UsesQApplication):

    def testCtorWithCreatedEnums(self):
        '''A simple case of QPen creation using created enums.'''
        width = 0
        style = Qt.PenStyle(0)
        cap = Qt.PenCapStyle(0)
        join = Qt.PenJoinStyle(0)
        pen = QPen(Qt.blue, width, style, cap, join)

    def testSetPenWithPenStyleEnum(self):
        '''Calls QPainter.setPen with both enum and integer. Bug #511.'''
        w = Painting()
        w.show()
        QTimer.singleShot(1000, self.app.quit)
        self.app.exec_()
        self.assertEqual(w.penFromEnum.style(), Qt.NoPen)
        self.assertEqual(w.penFromInteger.style(), Qt.SolidLine)


if __name__ == '__main__':
    unittest.main()

