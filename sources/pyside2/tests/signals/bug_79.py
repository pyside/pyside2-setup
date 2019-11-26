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

from __future__ import print_function
import unittest
import sys
import gc

from PySide2 import QtGui, QtWidgets

try:
    from sys import gettotalrefcount
    skiptest = False
except ImportError:
    skiptest = True

class ConnectTest(unittest.TestCase):

    def callback(self, o):
        print("callback")
        self._called = o

    def testNoLeaks_ConnectAndDisconnect(self):
        self._called = None
        app = QtWidgets.QApplication([])
        o = QtWidgets.QTreeView()
        o.setModel(QtGui.QStandardItemModel())
        o.selectionModel().destroyed.connect(self.callback)
        o.selectionModel().destroyed.disconnect(self.callback)
        gc.collect()
        # if this is no debug build, then we check at least that
        # we do not crash any longer.
        if not skiptest:
            total = gettotalrefcount()
        for idx in range(1000):
            o.selectionModel().destroyed.connect(self.callback)
            o.selectionModel().destroyed.disconnect(self.callback)
        gc.collect()
        if not skiptest:
            delta = gettotalrefcount() - total
            print("delta total refcount =", delta)
            self.assertTrue(abs(delta) < 10)


if __name__ == '__main__':
    unittest.main()
