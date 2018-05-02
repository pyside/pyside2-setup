#!/usr/bin/python

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

import unittest

from PySide2.QtWidgets import QGraphicsScene, QGraphicsRectItem, QGraphicsView, QApplication
from PySide2.QtGui import QBrush, QColor
from PySide2.QtCore import QTimer
from helper import UsesQApplication

qgraphics_item_painted = False

class RoundRectItem(QGraphicsRectItem):

    def __init__(self, x, y, w, h):
        QGraphicsRectItem.__init__(self, x, y, w, h)

    def paint(self, painter, qstyleoptiongraphicsitem, qwidget):
        global qgraphics_item_painted
        qgraphics_item_painted = True


class QGraphicsItemTest(UsesQApplication):

    def createRoundRect(self, scene):
        item = RoundRectItem(10, 10, 100, 100)
        item.setBrush(QBrush(QColor(255, 0, 0)))
        scene.addItem(item)
        return item

    def quit_app(self):
        self.app.quit()

    def test_setParentItem(self):
        global qgraphics_item_painted

        scene = QGraphicsScene()
        scene.addText("test")
        view = QGraphicsView(scene)

        rect = self.createRoundRect(scene)
        view.show()
        QTimer.singleShot(1000, self.quit_app)
        self.app.exec_()
        self.assertTrue(qgraphics_item_painted)


if __name__ == '__main__':
    unittest.main()

