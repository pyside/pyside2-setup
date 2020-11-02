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

""" Unittest for bug #547 """
""" http://bugs.openbossa.org/show_bug.cgi?id=547 """

import sys
import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6 import QtWidgets

class MyMainWindow(unittest.TestCase):
    app = QtWidgets.QApplication(sys.argv)
    def testCase1(self):
        self._tree = QtWidgets.QTreeWidget()
        self._tree.setColumnCount(2)
        self._i1 = None
        self._i11 = None

        self._updateTree()
        self.assertEqual(sys.getrefcount(self._i1), 3)
        self.assertEqual(sys.getrefcount(self._i11), 3)

        self._i11.parent().setExpanded(True)
        self._i11.setExpanded(True)

        self._updateTree()
        self.assertEqual(sys.getrefcount(self._i1), 3)
        self.assertEqual(sys.getrefcount(self._i11), 3)

    def testCase2(self):
        self._tree = QtWidgets.QTreeWidget()
        self._tree.setColumnCount(2)
        self._i1 = None
        self._i11 = None

        self._updateTree()
        self.assertEqual(sys.getrefcount(self._i1), 3)
        self.assertEqual(sys.getrefcount(self._i11), 3)

        self._i11.parent().setExpanded(True)
        self._i11.setExpanded(True)

        self.assertEqual(sys.getrefcount(self._i1), 3)
        self.assertEqual(sys.getrefcount(self._i11), 3)

    def _updateTree(self):
        self._tree.clear()
        if self._i1 and self._i11:
            self.assertEqual(sys.getrefcount(self._i1), 2)
            self.assertEqual(sys.getrefcount(self._i11), 2)

        self._i1 = QtWidgets.QTreeWidgetItem(self._tree, ['1', ])
        self.assertEqual(sys.getrefcount(self._i1), 3)
        self._i11 = QtWidgets.QTreeWidgetItem(self._i1, ['11', ])
        self.assertEqual(sys.getrefcount(self._i11), 3)

if __name__ == '__main__':
    unittest.main()

