#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

"""
This is the example from https://bugreports.qt.io/browse/PYSIDE-772
with no interaction as a unittest.
"""

import unittest
from PySide2.QtCore import Signal
from PySide2.QtWidgets import QApplication, QWidget
from PySide2 import QtWidgets

class Harness(QWidget):
    clicked = Signal()

    def __init__(self):
        QWidget.__init__(self)
        self.clicked.connect(self.method)
        self.clicked.connect(self._method)
        self.clicked.connect(self.__method)

    def method(self):  # Public method
        self.method_result = self.sender()

    def _method(self):  # Private method
        self.method__result = self.sender()

    def __method(self):  # Name mangled method
        self.method___result = self.sender()


class _Under(QWidget):
    clicked = Signal()

    def __init__(self):
        QWidget.__init__(self)
        self.clicked.connect(self.method)
        self.clicked.connect(self._method)
        self.clicked.connect(self.__method)

    def method(self):  # Public method
        self.method_result = self.sender()

    def _method(self):  # Private method
        self.method__result = self.sender()

    def __method(self):  # Name mangled method
        self.method___result = self.sender()


class TestMangle(unittest.TestCase):

    def setUp(self):
        QApplication()

    def tearDown(self):
        del QtWidgets.qApp

    def testPrivateMangle(self):
        harness = Harness()
        harness.clicked.emit()
        self.assertEqual(harness.method_result, harness)
        self.assertEqual(harness.method__result, harness)
        self.assertEqual(harness.method___result, harness)
        self.assertTrue("method" in type(harness).__dict__)
        self.assertTrue("_method" in type(harness).__dict__)
        self.assertFalse("__method" in type(harness).__dict__)
        self.assertTrue("_Harness__method" in type(harness).__dict__)

    def testPrivateMangleUnder(self):
        harness = _Under()
        harness.clicked.emit()
        self.assertEqual(harness.method_result, harness)
        self.assertEqual(harness.method__result, harness)
        self.assertEqual(harness.method___result, harness)
        # make sure that we skipped over the underscore in "_Under"
        self.assertTrue("method" in type(harness).__dict__)
        self.assertTrue("_method" in type(harness).__dict__)
        self.assertFalse("__method" in type(harness).__dict__)
        self.assertTrue("_Under__method" in type(harness).__dict__)


if __name__ == '__main__':
    unittest.main()
