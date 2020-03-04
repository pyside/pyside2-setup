#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

from helper.helper import adjust_filename
from helper.timedqapplication import TimedQApplication

from PySide2.QtQuick import QQuickView
from PySide2.QtCore import QObject, Signal, Slot, QUrl, QTimer, Property

class Obj(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.value = 0

    sumResult = Signal(int, name="sumResult", arguments=['sum'])

    @Slot(int, int)
    def sum(self, arg1, arg2):
        self.sumResult.emit(arg1 + arg2)

    @Slot(str)
    def sendValue(self, s):
        self.value = int(s)


class TestConnectionWithQml(TimedQApplication):

    def testSignalArguments(self):
        view = QQuickView()
        obj = Obj()

        context = view.rootContext()
        context.setContextProperty("o", obj)
        view.setSource(QUrl.fromLocalFile(adjust_filename('signal_arguments.qml', __file__)))
        root = view.rootObject()
        self.assertTrue(root)
        button = root.findChild(QObject, "button")
        view.show()
        button.clicked.emit()
        self.assertEqual(obj.value, 42)

if __name__ == '__main__':
    unittest.main()
