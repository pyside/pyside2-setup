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

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import *
from PySide2.QtGui import *
from PySide2.QtWidgets import *
from PySide2.QtTest import *

from helper.usesqapplication import UsesQApplication

class MyValidator1(QValidator):
    def fixup(self, input):
        return "fixed"

    def validate(self, input, pos):
        return (QValidator.Acceptable, "fixed", 1)

class MyValidator2(QValidator):
    def fixup(self, input):
        return "fixed"

    def validate(self, input, pos):
        return (QValidator.Acceptable, "fixed")

class MyValidator3(QValidator):
    def fixup(self, input):
        return "fixed"

    def validate(self, input, pos):
        return (QValidator.Acceptable,)

class MyValidator4(QValidator):
    def fixup(self, input):
        return "fixed"

    def validate(self, input, pos):
        return QValidator.Acceptable

class MyValidator5(QValidator):
    def validate(self, input, pos):
        if input.islower():
            return (QValidator.Intermediate, input, pos)
        else:
            return (QValidator.Acceptable, input, pos)

    def fixup(self, input):
        return "22"

class QValidatorTest(UsesQApplication):
    def testValidator1(self):
        line = QLineEdit()
        line.setValidator(MyValidator1())
        line.show()
        line.setText("foo")

        QTimer.singleShot(0, line.close)
        self.app.exec_()

        self.assertEqual(line.text(), "fixed")
        self.assertEqual(line.cursorPosition(), 1)

    def testValidator2(self):
        line = QLineEdit()
        line.setValidator(MyValidator2())
        line.show()
        line.setText("foo")

        QTimer.singleShot(0, line.close)
        self.app.exec_()

        self.assertEqual(line.text(), "fixed")
        self.assertEqual(line.cursorPosition(), 3)

    def testValidator3(self):
        line = QLineEdit()
        line.setValidator(MyValidator3())
        line.show()
        line.setText("foo")

        QTimer.singleShot(0, line.close)
        self.app.exec_()

        self.assertEqual(line.text(), "foo")
        self.assertEqual(line.cursorPosition(), 3)

    def testValidator4(self):
        line = QLineEdit()
        line.setValidator(MyValidator4())
        line.show()
        line.setText("foo")

        QTimer.singleShot(0, line.close)
        self.app.exec_()

        self.assertEqual(line.text(), "foo")
        self.assertEqual(line.cursorPosition(), 3)

    def testValidator5(self):
        line = QLineEdit()
        line.show()
        line.setValidator(MyValidator5())
        line.setText("foo")
        QTest.keyClick(line, Qt.Key_Return)
        self.assertEqual(line.text(), "22")

if __name__ == '__main__':
    unittest.main()
