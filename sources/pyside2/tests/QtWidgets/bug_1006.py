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
import weakref
import sys

from PySide2.QtCore import Qt
from PySide2.QtWidgets import QDialog, QLabel, QGridLayout, QHBoxLayout, QWidget

from helper import TimedQApplication

class LabelWindow(QDialog):
    def __init__(self, parent):
        super(LabelWindow, self).__init__(parent)

        self.test_layout = QGridLayout()
        label = QLabel("Label")
        self.test_layout.addWidget(label, 0, 0)
        self.setLayout(self.test_layout)
        self._destroyCalled = False


    def replace(self, unit):
        old_item = self.test_layout.itemAtPosition(0, 0)
        old_label = old_item.widget()
        ref = weakref.ref(old_item, self._destroyed)

        self.test_layout.removeWidget(old_label)
        unit.assertRaises(RuntimeError, old_item.widget)
        del old_item

        label = QLabel("Label New")
        old_label.deleteLater()
        label.setAlignment(Qt.AlignCenter)
        self.test_layout.addWidget(label, 0, 0)

    def _destroyed(self, obj):
        self._destroyCalled = True

class TestBug1006 (TimedQApplication):

    def testLayoutItemLifeTime(self):
        window = LabelWindow(None)
        window.replace(self)
        self.assertTrue(window._destroyCalled)
        self.app.exec_()

    def testParentLayout(self):
        def createLayout():
            label = QLabel()
            layout = QHBoxLayout()
            layout.addWidget(label)

            widget = QWidget()
            widget.setLayout(layout)
            return (layout, widget)
        (layout, widget) = createLayout()
        item = layout.itemAt(0)
        self.assertTrue(isinstance(item.widget(), QWidget))

    def testRemoveOrphanWidget(self):
        widget = QLabel()
        layout = QHBoxLayout()
        layout.addWidget(widget)
        self.assertEqual(sys.getrefcount(widget), 3)

        layout.removeWidget(widget)
        widget.setObjectName("MyWidget")
        self.assertEqual(sys.getrefcount(widget), 2)

    def testRemoveChildWidget(self):
        parent = QLabel()
        widget = QLabel(parent)
        self.assertEqual(sys.getrefcount(widget), 3)

        layout = QHBoxLayout()
        layout.addWidget(widget)
        self.assertEqual(sys.getrefcount(widget), 3)

        layout.removeWidget(widget)
        widget.setObjectName("MyWidget")
        self.assertEqual(sys.getrefcount(widget), 3)

if __name__ == "__main__":
    unittest.main()
