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

from helper.helper import adjust_filename

from PySide2.QtCore import Property, QObject, QUrl
from PySide2.QtGui import QGuiApplication
from PySide2.QtQml import qmlRegisterUncreatableType, QQmlEngine, QQmlComponent

noCreationReason = 'Cannot create an item of type: Uncreatable (expected)';

class Uncreatable(QObject):
    def __init__(self, parent = None):
        QObject.__init__(self, parent)
        self._name = 'uncreatable'

    def getName(self):
        return self._name

    def setName(self, value):
        self._name = value

    name = Property(str, getName, setName)

class TestQmlSupport(unittest.TestCase):

    def testIt(self):
        app = QGuiApplication([])

        self.assertTrue(qmlRegisterUncreatableType(Uncreatable, 'Charts', 1, 0,
                                                   'Uncreatable', noCreationReason) != -1);

        engine = QQmlEngine()
        component = QQmlComponent(engine, QUrl.fromLocalFile(adjust_filename('registeruncreatable.qml', __file__)))

        # Check that the uncreatable item produces the correct error
        self.assertEqual(component.status(), QQmlComponent.Error)
        errorFound = False
        for e in component.errors():
            if noCreationReason in e.toString():
                errorFound = True
                break
        self.assertTrue(errorFound)


if __name__ == '__main__':
    unittest.main()
