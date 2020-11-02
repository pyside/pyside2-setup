#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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
import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtWidgets import QWidget, QMainWindow
from helper.usesqapplication import UsesQApplication

class QWidgetInherit(QMainWindow):
    def __init__(self):
        QWidget.__init__(self)

class NativeEventTestWidget(QWidget):

    nativeEventCount = 0

    def __init__(self):
        QWidget.__init__(self)

    def nativeEvent(self, eventType, message):
        self.nativeEventCount = self.nativeEventCount + 1
        return [False, 0]

class QWidgetTest(UsesQApplication):

    def testInheritance(self):
        self.assertRaises(TypeError, QWidgetInherit)

class QWidgetVisible(UsesQApplication):

    def testBasic(self):
        # Also related to bug #244, on existence of setVisible'''
        widget = NativeEventTestWidget()
        self.assertTrue(not widget.isVisible())
        widget.setVisible(True)
        self.assertTrue(widget.isVisible())
        self.assertTrue(widget.winId() is not 0)
        # skip this test on macOS since no native events are received
        if sys.platform == 'darwin':
            return
        for i in range(10):
            if widget.nativeEventCount > 0:
                break
            self.app.processEvents()
        self.assertTrue(widget.nativeEventCount > 0)

if __name__ == '__main__':
    unittest.main()
