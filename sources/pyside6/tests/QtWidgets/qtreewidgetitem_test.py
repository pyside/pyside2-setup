#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide6.
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

"""
Unit tests for QTreeWidgetItem
------------------------------

This test is actually meant for all types which provide `tp_richcompare`
but actually define something without providing `==` or `!=` operators.
QTreeWidgetItem for instance defines `<` only.

PYSIDE-74: We redirect to type `object`s handling which is anyway the default
           when `tp_richcompare` is undefined.
"""

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6 import QtCore, QtWidgets


class QTreeWidgetItemTest(unittest.TestCase):
    def testClass(self):
        app = QtWidgets.QApplication([])
        treewidget = QtWidgets.QTreeWidget()
        item = QtWidgets.QTreeWidgetItem(["Words and stuff"])
        item2 = QtWidgets.QTreeWidgetItem(["More words!"])
        treewidget.insertTopLevelItem(0, item)

        dummy_list = ["Numbers", "Symbols", "Spam"]
        self.assertFalse(item in dummy_list)
        self.assertTrue(item not in dummy_list)
        self.assertFalse(item == item2)
        self.assertTrue(item != item2)
        treewidget.show()
        QtCore.QTimer.singleShot(500, app.quit)
        app.exec_()


if __name__ == "__main__":
    unittest.main()

