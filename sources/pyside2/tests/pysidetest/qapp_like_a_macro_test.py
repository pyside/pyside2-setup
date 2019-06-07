#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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
import PySide2

# This test tests the new "macro" feature of qApp.
# It also uses the qApp variable to finish the instance and start over.

class qAppMacroTest(unittest.TestCase):
    def test_qApp_is_like_a_macro_and_can_restart(self):
        from PySide2 import QtCore
        try:
            from PySide2 import QtGui, QtWidgets
        except ImportError:
            QtWidgets = QtGui = QtCore
        # qApp is in the builtins
        self.assertEqual(bool(qApp), False)
        # and also in certain PySide modules
        QtCore.qApp, QtGui.qApp, QtWidgets.qApp
        # and they are all the same
        self.assertTrue(qApp is QtCore.qApp is QtGui.qApp is QtWidgets.qApp)
        # and the type is NoneType, but it is not None (cannot work)
        self.assertTrue(type(qApp) is type(None))
        self.assertTrue(qApp is not None)
        # now we create an application for all cases
        classes = (QtCore.QCoreApplication,
                   QtGui.QGuiApplication,
                   QtWidgets.QApplication)
        for klass in classes:
            print("created", klass([]))
            del __builtins__.qApp
            print("deleted qApp")
        # creating without deletion raises:
        QtCore.QCoreApplication([])
        with self.assertRaises(RuntimeError):
            QtCore.QCoreApplication([])
        # assigning qApp is obeyed
        QtCore.qApp = 42
        del __builtins__.qApp
        self.assertEqual(QtCore.qApp, 42)
        self.assertNotEqual(__builtins__, 42)
        # delete it and it re-appears
        del QtCore.qApp
        QtCore.QCoreApplication([])
        self.assertEqual(QtCore.QCoreApplication.instance(), QtCore.qApp)
        # and they are again all the same
        self.assertTrue(qApp is QtCore.qApp is QtGui.qApp is QtWidgets.qApp)

if __name__ == '__main__':
    unittest.main()
