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

from __future__ import print_function

import unittest
from PySide2.QtGui import QTextCursor
from PySide2.QtPrintSupport import QPrinter
from PySide2.QtWidgets import QLayout, QWidget, QGraphicsLayout, QGraphicsLayoutItem

from helper import UsesQApplication

class Layout(QLayout):
    def __init__(self):
        QLayout.__init__(self)

class GraphicsLayout(QGraphicsLayout):
    def __init__(self):
        QGraphicsLayout.__init__(self)

class GraphicsLayoutItem(QGraphicsLayoutItem):
    def __init__(self):
        QGraphicsLayoutItem.__init__(self)

class ReturnsQuadruplesOfNumbers(UsesQApplication):
    def compareTuples(self, ta, tb):
        for va,vb in zip(ta, tb):
            if round(va) != round(vb):
                return False
        return True

    def testQGraphicsLayoutGetContentsMargins(self):
        obj = GraphicsLayout()
        values = (10.0, 20.0, 30.0, 40.0)
        obj.setContentsMargins(*values)
        self.assertTrue(self.compareTuples(obj.getContentsMargins(), values))

    def testQGraphicsLayoutItemGetContentsMargins(self):
        obj = GraphicsLayoutItem()
        self.assertTrue(self.compareTuples(obj.getContentsMargins(), (0.0, 0.0, 0.0, 0.0)))

    def testQWidgetGetContentsMargins(self):
        obj = QWidget()
        values = (10, 20, 30, 40)
        obj.setContentsMargins(*values)
        self.assertTrue(self.compareTuples(obj.getContentsMargins(), values))

    def testQLayoutGetContentsMargins(self):
        obj = Layout()
        values = (10, 20, 30, 40)
        obj.setContentsMargins(*values)
        self.assertTrue(self.compareTuples(obj.getContentsMargins(), values))

    def testQTextCursorSelectedTableCells(self):
        obj = QTextCursor()
        self.assertEqual(obj.selectedTableCells(), (-1, -1, -1, -1))

    def testQPrinterGetPageMargins(self):
        # Bug #742
        obj = QPrinter()
        # On macOS the minimum margin of a page is ~12, setting something lower than that will
        # actually fail to set all the margins.
        values = (15.0, 16.0, 17.0, 18.0, QPrinter.Point)
        obj.setPageMargins(*values)
        print(obj.getPageMargins(QPrinter.Point), values[:-1])
        self.assertTrue(self.compareTuples(obj.getPageMargins(QPrinter.Point), values[:-1]))

if __name__ == "__main__":
   unittest.main()

