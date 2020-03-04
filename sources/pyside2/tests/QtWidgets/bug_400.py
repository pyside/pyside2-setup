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

''' Test bug 400: http://bugs.openbossa.org/show_bug.cgi?id=400'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from helper.usesqapplication import UsesQApplication
from PySide2.QtWidgets import QTreeWidgetItemIterator, QTreeWidgetItem, QTreeWidget

class BugTest(UsesQApplication):
    def testCase(self):
        treeWidget = QTreeWidget()
        treeWidget.setColumnCount(1)
        items = []
        for i in range(10):
            items.append(QTreeWidgetItem(None, ["item: %i" % i]))

        treeWidget.insertTopLevelItems(0, items);
        _iter = QTreeWidgetItemIterator(treeWidget)
        index = 0
        while(_iter.value()):
            item = _iter.value()
            self.assertTrue(item is items[index])
            index += 1
            _iter += 1


if __name__ == '__main__':
    unittest.main()
