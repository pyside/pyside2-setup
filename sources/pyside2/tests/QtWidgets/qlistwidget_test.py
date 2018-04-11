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
from PySide2 import QtWidgets, QtCore
from helper import UsesQApplication

class QListWidgetTest(UsesQApplication):

    def populateList(self, lst):
        o = QtCore.QObject()
        o.setObjectName("obj")

        item = QtWidgets.QListWidgetItem("item0")
        item.setData(QtCore.Qt.UserRole, o)
        #item._data = o
        self.assertTrue(sys.getrefcount(o), 3)
        self.assertTrue(sys.getrefcount(item), 2)
        lst.addItem(item)
        self.assertTrue(sys.getrefcount(item), 3)

    def checkCurrentItem(self, lst):
        item = lst.currentItem()
        self.assertTrue(sys.getrefcount(item), 3)

    def checkItemData(self, lst):
        item = lst.currentItem()
        o = item.data(QtCore.Qt.UserRole)
        self.assertTrue(sys.getrefcount(o), 4)
        self.assertEqual(o, item._data)
        self.assertTrue(sys.getrefcount(o), 2)

    def testConstructorWithParent(self):
        lst = QtWidgets.QListWidget()
        self.populateList(lst)
        self.checkCurrentItem(lst)
        i = lst.item(0)
        self.assertTrue(sys.getrefcount(i), 3)

        del lst
        self.assertTrue(sys.getrefcount(i), 2)
        del i

    def testIt(self):
        lst = QtWidgets.QListWidget()
        lst.show()
        slot = lambda : lst.removeItemWidget(lst.currentItem())
        lst.addItem(QtWidgets.QListWidgetItem("foo"))
        QtCore.QTimer.singleShot(0, slot)
        QtCore.QTimer.singleShot(0, lst.close)
        self.app.exec_()
        self.assertEqual(lst.count(), 1)

    def testClear(self):
        lst = QtWidgets.QListWidget()
        lst.addItem("foo")
        item = lst.item(0)
        self.assertIsNone(lst.clear())
        self.assertRaises(RuntimeError, lambda: item.text())

if __name__ == '__main__':
    unittest.main()
