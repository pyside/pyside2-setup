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

import sys

import unittest
from PySide2.QtCore import QItemSelection
from PySide2.QtGui import QStandardItemModel, QStandardItem
from PySide2.QtWidgets import QApplication
class Bug324(unittest.TestCase):
    def testOperators(self):
        model = QStandardItemModel()
        for i in range(100):
            model.appendRow(QStandardItem("Item: %d"%i))

        first = model.index(0, 0)
        second = model.index(10, 0)
        third = model.index(20, 0)
        fourth = model.index(30, 0)

        sel = QItemSelection(first, second)
        sel2 = QItemSelection()
        sel2.select(third, fourth)

        sel3 = sel + sel2 #check operator +
        self.assertEqual(len(sel3), 2)
        sel4 = sel
        sel4 += sel2 #check operator +=
        self.assertEqual(len(sel4), 2)
        self.assertEqual(sel4, sel3)

if __name__ == "__main__":
    unittest.main()
