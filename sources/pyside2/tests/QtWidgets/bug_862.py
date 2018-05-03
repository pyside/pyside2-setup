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

from PySide2.QtCore import QObject
from PySide2.QtWidgets import *
import PySide2.QtCore
import unittest

class MyQObject(QObject):
    def __init__(self):
        QObject.__init__(self)

class MyQWidget(QWidget):
    def __init__(self):
        QWidget.__init__(self)

class MyQGraphicsObject(QGraphicsObject):
    def __init__(self):
        QGraphicsObject.__init__(self)

class MyQGraphicsItem(QGraphicsItem):
    def __init__(self):
        QGraphicsItem.__init__(self)

class TestRepr (unittest.TestCase):

    def testIt(self):

        app = QApplication([])

        self.assertEqual("<__main__.MyQObject object at ", repr(MyQObject())[:30])
        self.assertEqual("<__main__.MyQWidget object at ", repr(MyQWidget())[:30])
        self.assertEqual("<__main__.MyQGraphicsObject(0x", repr(MyQGraphicsObject())[:30])
        self.assertEqual("<__main__.MyQGraphicsItem(0x", repr(MyQGraphicsItem())[:28])

        self.assertEqual("<PySide2.QtCore.QObject object at ", repr(QObject())[:34])
        self.assertEqual("<PySide2.QtCore.QObject object at ", repr(PySide2.QtCore.QObject())[:34])
        self.assertEqual("<PySide2.QtWidgets.QWidget object at ", repr(QWidget())[:37])
        self.assertEqual("<PySide2.QtWidgets.QGraphicsWidget(0x", repr(QGraphicsWidget())[:37])

if __name__ == "__main__":
    unittest.main()
