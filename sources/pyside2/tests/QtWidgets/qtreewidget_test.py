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

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtWidgets import QTreeWidget, QTreeWidgetItem, QPushButton
from helper.usesqapplication import UsesQApplication

class QTreeWidgetTest(UsesQApplication):

    # PYSIDE-73:
    #   There was a problem when adding items to a QTreeWidget
    #   when the Widget was being build on the method call instead
    #   of as a separate variable.
    #   The problem was there was not ownership transfer, so the
    #   QTreeWidget did not own the QWidget element
    def testSetItemWidget(self):

        treeWidget = QTreeWidget()
        treeWidget.setColumnCount(2)

        item = QTreeWidgetItem(['text of column 0', ''])
        treeWidget.insertTopLevelItem(0, item)
        # Adding QPushButton inside the method
        treeWidget.setItemWidget(item, 1,
            QPushButton('Push button on column 1'))

        # Getting the widget back
        w = treeWidget.itemWidget(treeWidget.itemAt(0,1), 1)
        self.assertIsInstance(w, QPushButton)

        p = QPushButton('New independent button')
        # Adding QPushButton object from variable
        treeWidget.setItemWidget(item, 0, p)
        w = treeWidget.itemWidget(treeWidget.itemAt(0,0), 0)
        self.assertIsInstance(w, QPushButton)

if __name__ == '__main__':
    unittest.main()
