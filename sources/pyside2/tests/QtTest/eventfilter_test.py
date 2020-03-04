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

'''Tests for QKeyEvent'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import Qt, QObject, QEvent
from PySide2.QtGui import QKeyEvent
from PySide2.QtWidgets import QLineEdit
from PySide2.QtTest import QTest

from helper.usesqapplication import UsesQApplication


class KeyEventFilter(QObject):

    def __init__(self, widget, eventType, key):
        QObject.__init__(self)

        self.widget = widget
        self.eventType = eventType
        self.key = key

        self.processed = False

    def eventFilter(self, obj, event):
        if self.widget == obj and event.type() == self.eventType and \
               isinstance(event, QKeyEvent) and event.key() == self.key:
            self.processed = True
            return True

        return False

class EventFilterTest(UsesQApplication):

    def testKeyEvent(self):
        widget = QLineEdit()
        key = Qt.Key_A
        eventFilter = KeyEventFilter(widget, QEvent.KeyPress, key)
        widget.installEventFilter(eventFilter)

        QTest.keyClick(widget, key)

        self.assertTrue(eventFilter.processed)



if __name__ == '__main__':
    unittest.main()
