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

import os
from helper import UsesQApplication
from PySide2.QtCore import QTimer
from PySide2.QtGui import QPicture, QPainter
from PySide2.QtWidgets import QWidget

class MyWidget(QWidget):
    def paintEvent(self, e):
        p = QPainter(self)
        p.drawPicture(0, 0, self._picture)
        self._app.quit()

class QPictureTest(UsesQApplication):
    def testFromData(self):
        picture = QPicture()
        painter = QPainter()
        painter.begin(picture)
        painter.drawEllipse(10,20, 80,70)
        painter.end()

        data = picture.data()
        picture2 = QPicture()
        picture2.setData(data)

        self.assertEqual(picture2.data(), picture.data())

        w = MyWidget()
        w._picture = picture2
        w._app = self.app

        QTimer.singleShot(300, w.show)
        self.app.exec_()

if __name__ == '__main__':
    unittest.main()

