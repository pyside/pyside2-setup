#!/usr/bin/python

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

import unittest
from testbinding import TestObject
from PySide2.QtWidgets import QApplication
from PySide2 import __all__ as all

class QApplicationInstance(unittest.TestCase):

    def appDestroyed(self):
        self.assertTrue(False)

    def testInstanceObject(self):
        self.assertEqual(type(qApp), type(None))
        TestObject.createApp()
        app1 = QApplication.instance()
        app2 = QApplication.instance()
        app1.setObjectName("MyApp")
        self.assertEqual(app1, app2)
        self.assertEqual(app2.objectName(), app1.objectName())
        # We no longer support qApp when embedding
        # if len(all) > 3:
        #     # an import triggers qApp initialization
        #     __import__("PySide2." + all[-1])
        #     self.assertEqual(app1, qApp)
        app1.destroyed.connect(self.appDestroyed)

if __name__ == '__main__':
    unittest.main()

