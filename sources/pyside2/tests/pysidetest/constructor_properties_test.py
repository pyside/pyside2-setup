#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

from helper.usesqapplication import UsesQApplication
from PySide2.QtCore import Qt
from PySide2.QtGui import QColor
from PySide2.QtWidgets import QAction, QApplication, QFrame, QLabel


class ConstructorPropertiesTest(UsesQApplication):

    # PYSIDE-1019: First property extension was support by the constructor.
    def testCallConstructor(self):
        label = QLabel(
            frameStyle=QFrame.Panel | QFrame.Sunken,
            text="first line\nsecond line",
            alignment=Qt.AlignBottom | Qt.AlignRight
        )
        self.assertRaises(AttributeError, lambda: QLabel(
            somethingelse=42,
            text="first line\nsecond line",
            alignment=Qt.AlignBottom | Qt.AlignRight
        ))


class DiverseKeywordsTest(UsesQApplication):

    def testDuplicateKeyword(self):
        r, g, b, a = 1, 2, 3, 4
        with self.assertRaises(TypeError) as cm:
            QColor(r, g, b, a, a=0)
        self.assertTrue("multiple" in cm.exception.args[0])

    # PYSIDE-1305: Handle keyword args correctly.
    def testUndefinedKeyword(self):
        r, g, b, a = 1, 2, 3, 4
        # From the jira issue:
        with self.assertRaises(AttributeError) as cm:
            QColor(r, g, b, a, alpha=0)
        self.assertTrue("unsupported" in cm.exception.args[0])

    # PYSIDE-1305: Handle keyword args correctly.
    def testUndefinedConstructorKeyword(self):
        # make sure that the given attribute lands in the constructor
        x = QAction(autoRepeat=False)
        self.assertEqual(x.autoRepeat(), False)
        x = QAction(autoRepeat=True)
        self.assertEqual(x.autoRepeat(), True)
        # QAction derives from QObject, and so the missing attributes
        # in the constructor are reported as AttributeError.
        with self.assertRaises(AttributeError):
            QAction(some_other_name=42)


if __name__ == '__main__':
    unittest.main()
