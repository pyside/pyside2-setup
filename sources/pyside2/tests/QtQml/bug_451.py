#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

'''
Test bug 451: http://bugs.openbossa.org/show_bug.cgi?id=451

An archive of said bug:
https://srinikom.github.io/pyside-bz-archive/451.html
'''

import sys
import unittest

from helper import adjust_filename

from PySide2 import QtCore, QtGui, QtQuick

class PythonObject(QtCore.QObject):
    def __init__(self):
        QtCore.QObject.__init__(self, None)
        self._called = ""
        self._arg1 = None
        self._arg2 = None

    def setCalled(self, v):
        self._called = v

    def setArg1(self, v):
        self._arg1 = v

    def setArg2(self, v):
        self._arg2 = v

    def getCalled(self):
        return self._called

    def getArg1(self):
        return self._arg1

    def getArg2(self):
        return self._arg2

    called = QtCore.Property(str, getCalled, setCalled)
    arg1 = QtCore.Property(int, getArg1, setArg1)
    arg2 = QtCore.Property('QVariant', getArg2, setArg2)

class TestBug(unittest.TestCase):
    def testQMLFunctionCall(self):
        app = QtGui.QGuiApplication(sys.argv)
        view = QtQuick.QQuickView()

        obj = PythonObject()
        context = view.rootContext()
        context.setContextProperty("python", obj)
        view.setSource(QtCore.QUrl.fromLocalFile(adjust_filename('bug_451.qml', __file__)))
        root = view.rootObject()
        root.simpleFunction()
        self.assertEqual(obj.called, "simpleFunction")

        root.oneArgFunction(42)
        self.assertEqual(obj.called, "oneArgFunction")
        self.assertEqual(obj.arg1, 42)

        root.twoArgFunction(10, app)
        self.assertEqual(obj.called, "twoArgFunction")
        self.assertEqual(obj.arg1, 10)
        self.assertEqual(obj.arg2, app)

        rvalue = root.returnFunction()
        self.assertEqual(obj.called, "returnFunction")
        self.assertEqual(rvalue, 42)


if __name__ == '__main__':
    unittest.main()
