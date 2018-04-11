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

import unittest
import sys

from helper import UsesQApplication
from PySide2.QtCore import *
from PySide2.QtWidgets import *

class MyLayout(QLayout):
    def __init__(self, parent=None):
        QLayout.__init__(self, parent)
        self._list = []

    def addItem(self, item):
        self.add(item)

    def addWidget(self, widget):
        self.add(QWidgetItem(widget))

    def itemAt(self, index):
        if index < len(self._list):
            return self._list[index]

        return None

    def count(self):
        return len(self._list)

    def add(self, item):
        self._list.append(item)

class MissingItemAtLayout(QLayout):
    def __init__(self, parent=None):
        QLayout.__init__(self, parent)
        self._list = []

    def addItem(self, item):
        self.add(item)

    def addWidget(self, widget):
        self.add(QWidgetItem(widget))

    def count(self):
        return len(self._list)

    def add(self, item):
        self._list.append(item)

#Test if a layout implemented in python, the QWidget.setLayout works
#fine because this implement som layout functions used in glue code of
#QWidget, then in c++ when call a virtual function this need call the QLayout
#function implemented in python

class QLayoutTest(UsesQApplication):
    def testOwnershipTransfer(self):
        b = QPushButton("teste")
        l = MyLayout()

        l.addWidget(b)

        self.assertEqual(sys.getrefcount(b), 2)

        w = QWidget()

        #transfer ref
        w.setLayout(l)

        self.assertEqual(sys.getrefcount(b), 3)


    def testReferenceTransfer(self):
        b = QPushButton("teste")
        l = QHBoxLayout()

        # keep ref
        l.addWidget(b)
        self.assertEqual(sys.getrefcount(b), 3)

        w = QWidget()

        # transfer ref
        w.setLayout(l)

        self.assertEqual(sys.getrefcount(b), 3)

        # release ref
        del w

        self.assertEqual(sys.getrefcount(b), 2)

    def testMissingFunctions(self):
        w = QWidget()
        b = QPushButton("test")
        l = MissingItemAtLayout()

        l.addWidget(b)

        self.assertRaises(RuntimeError, w.setLayout, l)

    def testQFormLayout(self):
        w = QWidget()
        formLayout = QFormLayout()
        spacer = QSpacerItem(100, 30)
        formLayout.setItem(0, QFormLayout.SpanningRole, spacer)
        w.setLayout(formLayout)
        w.show()
        QTimer.singleShot(10, w.close)
        self.app.exec_()
        del w
        self.assertRaises(RuntimeError, spacer.isEmpty)

if __name__ == '__main__':
    unittest.main()
