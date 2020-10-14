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

from PySide2 import QtWidgets
from PySide2.support import __feature__

"""
snake_prop_feature_test.py
--------------------------

Test the snake_case and true_property feature.

This works now, including class properties.
"""

class Window(QtWidgets.QWidget):
    def __init__(self):
        super(Window, self).__init__()


class FeatureTest(unittest.TestCase):
    def setUp(self):
        qApp or QtWidgets.QApplication()
        __feature__.set_selection(0)

    def tearDown(self):
        __feature__.set_selection(0)
        qApp.shutdown()

    def testRenamedFunctions(self):
        window = Window()
        window.setWindowTitle('camelCase')

        # and now the same with snake_case enabled
        from __feature__ import snake_case

        # Works with the same window! window = Window()
        window.set_window_title('snake_case')

    def testPropertyAppearVanish(self):
        window = Window()

        self.assertTrue(callable(window.isModal))
        with self.assertRaises(AttributeError):
            window.modal

        from __feature__ import snake_case, true_property

        self.assertTrue(isinstance(QtWidgets.QWidget.modal, property))
        self.assertTrue(isinstance(window.modal, bool))
        with self.assertRaises(AttributeError):
            window.isModal

        # switching back
        __feature__.set_selection(0)

        self.assertTrue(callable(window.isModal))
        with self.assertRaises(AttributeError):
            window.modal

    def testClassProperty(self):
        from __feature__ import snake_case, true_property
        # We check the class...
        self.assertEqual(type(QtWidgets.QApplication.quit_on_last_window_closed), bool)
        x = QtWidgets.QApplication.quit_on_last_window_closed
        QtWidgets.QApplication.quit_on_last_window_closed = not x
        self.assertEqual(QtWidgets.QApplication.quit_on_last_window_closed, not x)
        # ... and now the instance.
        self.assertEqual(type(qApp.quit_on_last_window_closed), bool)
        x = qApp.quit_on_last_window_closed
        qApp.quit_on_last_window_closed = not x
        self.assertEqual(qApp.quit_on_last_window_closed, not x)
        # make sure values are equal
        self.assertEqual(qApp.quit_on_last_window_closed,
                         QtWidgets.QApplication.quit_on_last_window_closed)


if __name__ == '__main__':
    unittest.main()
