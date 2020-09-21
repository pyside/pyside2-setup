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

from helper.usesqapplication import UsesQApplication

from PySide2.QtWidgets import QWidget
from PySide2.QtUiTools import QUiLoader


class OverridingLoader(QUiLoader):
    def createWidget(self, class_name, parent=None, name=''):
        if class_name == 'QWidget':
            w = QWidget(parent)
            w.setObjectName(name)
            return w
        return QUiLoader.createWidget(self, class_name, parent, name)


class QUiLoaderTester(UsesQApplication):
    def setUp(self):
        UsesQApplication.setUp(self)
        self._filePath = os.path.join(os.path.dirname(__file__), 'test.ui')

    def testLoadFile(self):
        loader = QUiLoader()
        parent = QWidget()
        w = loader.load(self._filePath, parent)
        self.assertNotEqual(w, None)

        self.assertEqual(len(parent.children()), 1)

        child = w.findChild(QWidget, "child_object")
        self.assertNotEqual(child, None)
        self.assertEqual(w.findChild(QWidget, "grandson_object"), child.findChild(QWidget, "grandson_object"))


    def testLoadFileOverride(self):
        # PYSIDE-1070, override QUiLoader::createWidget() with parent=None crashes
        loader = OverridingLoader()
        w = loader.load(self._filePath)
        self.assertNotEqual(w, None)


if __name__ == '__main__':
    unittest.main()

