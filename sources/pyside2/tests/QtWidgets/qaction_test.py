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

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtWidgets import QAction, QWidget
from helper.usesqapplication import UsesQApplication

class QPainterDrawText(UsesQApplication):

    def _cb(self):
        self._called = True

    def testSignal(self):
        o = QWidget()
        act = QAction(o)
        self._called = False
        act.triggered.connect(self._cb)
        act.trigger()
        self.assertTrue(self._called)

    def testNewCtor(self):
        o = QWidget()
        self._called = False
        myAction = QAction("&Quit", o, triggered=self._cb)
        myAction.trigger()
        self.assertTrue(self._called)


class SetShortcutTest(UsesQApplication):

    def testSetShortcut(self):
        # Somehow an exception was leaking from the constructor
        # and appearing in setShortcut.
        o = QWidget()
        action = QAction('aaaa', o)
        shortcut = 'Ctrl+N'
        action.setShortcut(shortcut)
        s2 = action.shortcut()
        self.assertEqual(s2, shortcut)

if __name__ == '__main__':
    unittest.main()

