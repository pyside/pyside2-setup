#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

from PySide6.QtWidgets import QWidget, QFrame, QPushButton
from PySide6.QtUiTools import loadUiType

class loadUiTypeTester(UsesQApplication):
    def testFunction(self):
        filePath = os.path.join(os.path.dirname(__file__), "minimal.ui")
        loaded = loadUiType(filePath)
        self.assertNotEqual(loaded, None)

        # (<class '__main__.Ui_Form'>, <class 'PySide6.QtWidgets.QFrame'>)
        generated, base = loaded

        # Generated class contains retranslateUi method
        self.assertTrue("retranslateUi" in dir(generated))

        # Base class instance will be QFrame for this example
        self.assertTrue(isinstance(base(), QFrame))

        anotherFileName = os.path.join(os.path.dirname(__file__), "test.ui")
        another = loadUiType(anotherFileName)
        self.assertNotEqual(another, None)

        generated, base = another
        # Base class instance will be QWidget for this example
        self.assertTrue(isinstance(base(), QWidget))

        w = base()
        ui = generated()
        ui.setupUi(w)

        self.assertTrue(isinstance(ui.child_object, QFrame))
        self.assertTrue(isinstance(ui.grandson_object, QPushButton))


if __name__ == '__main__':
    unittest.main()

