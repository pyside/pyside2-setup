#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

from PySide2.QtGui import QStandardItemModel
from PySide2.QtWidgets import (QWidget, QTreeView, QVBoxLayout,
    QStyledItemDelegate, QHeaderView)
from PySide2.QtCore import Qt
from helper.usesqapplication import UsesQApplication

class Widget(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.treeView = QTreeView(self)
        layout = QVBoxLayout()
        layout.addWidget(self.treeView)
        self.setLayout(layout)
        self.treeView.setModel(QStandardItemModel())

        self.treeView.model().setHorizontalHeaderLabels(('3', '1', '5'))

class QWidgetTest(UsesQApplication):

    def testDelegates(self):
        widget = Widget()
        t = widget.treeView

        # When calling setItemDelegateForColumn using a separate variable
        # for the second argument (QAbstractItemDelegate), there was no problem
        # on keeping the reference to this object, since the variable was kept
        # alive (case A)
        # Contrary, when instantiating this argument on the function call
        # Using QStyledItemDelegate inside the call the reference of the
        # object was lost, causing a segfault. (case B)

        # Case A
        d = QStyledItemDelegate()
        # Using QStyledItemDelegate from a variable so we keep the reference alive
        # and we encounter no segfault.
        t.setItemDelegateForColumn(0, d)
        # This raised the Segmentation Fault too, because manually destroying
        # the object caused a missing refrence.
        del d

        # Getting the delegates
        a = t.itemDelegateForColumn(0)
        self.assertIsInstance(a, QStyledItemDelegate)

        # Case B
        t.setItemDelegateForColumn(1, QStyledItemDelegate())

        # Getting the delegates
        b = t.itemDelegateForColumn(1)
        self.assertIsInstance(b, QStyledItemDelegate)

        # Test for Rows
        t.setItemDelegateForRow(0, QStyledItemDelegate())
        self.assertIsInstance(t.itemDelegateForRow(0), QStyledItemDelegate)

        # Test for general delegate
        t.setItemDelegate(QStyledItemDelegate())
        self.assertIsInstance(t.itemDelegate(), QStyledItemDelegate)

    def testHeader(self):
        tree = QTreeView()
        tree.setHeader(QHeaderView(Qt.Horizontal))
        self.assertIsNotNone(tree.header())

if __name__ == '__main__':
    unittest.main()
