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

import unittest
import os
from helper import UsesQApplication

from PySide2 import QtWidgets
from PySide2.QtUiTools import QUiLoader

class MyWidget(QtWidgets.QComboBox):
    def __init__(self, parent=None):
        QtWidgets.QComboBox.__init__(self, parent)

    def isPython(self):
        return True

class BugTest(UsesQApplication):
    def testCase(self):
        w = QtWidgets.QWidget()
        loader = QUiLoader()

        filePath = os.path.join(os.path.dirname(__file__), 'action.ui')
        result = loader.load(filePath, w)
        self.assertTrue(isinstance(result.actionFoo, QtWidgets.QAction))

    def testPythonCustomWidgets(self):
        w = QtWidgets.QWidget()
        loader = QUiLoader()
        loader.registerCustomWidget(MyWidget)

        filePath = os.path.join(os.path.dirname(__file__), 'pycustomwidget.ui')
        result = loader.load(filePath, w)
        self.assertTrue(isinstance(result.custom, MyWidget))
        self.assertTrue(result.custom.isPython())

    def testPythonCustomWidgetsTwice(self):
        w = QtWidgets.QWidget()
        loader = QUiLoader()
        loader.registerCustomWidget(MyWidget)

        filePath = os.path.join(os.path.dirname(__file__), 'pycustomwidget2.ui')
        result = loader.load(filePath, w)
        self.assertTrue(isinstance(result.custom, MyWidget))
        self.assertTrue(isinstance(result.custom2, MyWidget))
        self.assertTrue(result.custom.isPython())

if __name__ == '__main__':
    unittest.main()

