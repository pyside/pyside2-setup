# -*- coding: utf-8 -*-

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

'''Test cases for PySide API2 support'''


import unittest
import sys

from PySide2.QtGui import QIntValidator, QValidator
from PySide2.QtWidgets import QWidget, QSpinBox, QApplication

from helper import UsesQApplication

class WidgetValidatorQInt(QWidget, QIntValidator):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        QIntValidator.__init__(self, parent)

class WidgetValidatorQSpinBox(QSpinBox):
    def __init__(self, parent=None):
        QSpinBox.__init__(self, parent)

    def fixup(self, text):
        print("It was called!")

class DoubleQObjectInheritanceTest(UsesQApplication):

    def testDouble(self):
        '''Double inheritance from QObject classes'''

        obj = WidgetValidatorQInt()

        #QIntValidator methods
        state, string, number = obj.validate('Test', 0)
        self.assertEqual(state, QValidator.Invalid)
        state, string, number = obj.validate('33', 0)
        self.assertEqual(state, QValidator.Acceptable)

    def testQSpinBox(self):
        obj = WidgetValidatorQSpinBox()

        obj.setRange(1, 10)
        obj.setValue(0)
        self.assertEqual(obj.value(), 1)

class QClipboardTest(UsesQApplication):

    def testQClipboard(self):
        #skip this test on MacOS because the clipboard is not available during the ssh session
        #this cause problems in the buildbot
        if sys.platform == 'darwin':
            return
        clip = QApplication.clipboard()
        clip.setText("Testing this thing!")

        text, subtype = clip.text("")
        self.assertEqual(subtype, "plain")
        self.assertEqual(text, "Testing this thing!")

if __name__ == '__main__':
    unittest.main()
