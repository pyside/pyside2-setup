#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6 import QtWidgets
from PySide6.support import __feature__

"""
errormessages_with_features_test.py
-----------------------------------

When errors occur while features are switched, we must always produce a
valid error message.

This test is in its own file because combining it with
"snake_prop_feature_test" gave strange interactions with the other tests.
"""

class ErrormessagesWithFeatures(unittest.TestCase):
    probe = "called with wrong argument types"
    probe_miss = "missing signature"

    def setUp(self):
        qApp or QtWidgets.QApplication()
        __feature__.set_selection(0)

    def tearDown(self):
        __feature__.set_selection(0)
        qApp.shutdown()

    def testCorrectErrorMessagesPlain(self):
        with self.assertRaises(TypeError) as cm:
            QtWidgets.QLabel().setFont(42)
        print("\n\n" + cm.exception.args[0])
        self.assertTrue(self.probe in cm.exception.args[0])

    def testCorrectErrorMessagesSnake(self):
        from __feature__ import snake_case
        with self.assertRaises(TypeError) as cm:
            QtWidgets.QLabel().set_font(42)
        print("\n\n" + cm.exception.args[0])
        self.assertTrue(self.probe in cm.exception.args[0])

    def testCorrectErrorMessagesProp(self):
        from __feature__ import true_property
        with self.assertRaises(TypeError) as cm:
            QtWidgets.QLabel().font = 42
        print("\n\n" + cm.exception.args[0])
        self.assertTrue(self.probe in cm.exception.args[0])

    def testCorrectErrorMessagesSnakeProp(self):
        from __feature__ import snake_case, true_property
        with self.assertRaises(TypeError) as cm:
            QtWidgets.QLabel().font = 42
        print("\n\n" + cm.exception.args[0])
        self.assertTrue(self.probe in cm.exception.args[0])

    def testCorrectErrorMessagesClassProp(self):
        from __feature__ import true_property
        with self.assertRaises(TypeError) as cm:
            QtWidgets.QApplication.quitOnLastWindowClosed = object
        print("\n\n" + cm.exception.args[0])
        self.assertTrue(self.probe_miss in cm.exception.args[0])
        with self.assertRaises(TypeError) as cm:
            qApp.quitOnLastWindowClosed = object
        self.assertTrue(self.probe_miss in cm.exception.args[0])

    def testCorrectErrorMessagesClassSnakeProp(self):
        from __feature__ import snake_case, true_property
        with self.assertRaises(TypeError) as cm:
            QtWidgets.QApplication.quit_on_last_window_closed = object
        print("\n\n" + cm.exception.args[0])
        self.assertTrue(self.probe_miss in cm.exception.args[0])
        with self.assertRaises(TypeError) as cm:
            qApp.quit_on_last_window_closed = object
        self.assertTrue(self.probe_miss in cm.exception.args[0])


if __name__ == '__main__':
    unittest.main()
