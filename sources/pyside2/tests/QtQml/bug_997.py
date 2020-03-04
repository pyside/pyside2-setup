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

from helper.helper import adjust_filename
from helper.usesqapplication import UsesQApplication

from PySide2 import QtCore, QtQml, QtQuick

class TestBug(UsesQApplication):
    def testQMLFunctionCall(self):
        ownerData = QtQml.QQmlPropertyMap()
        ownerData.insert('name', 'John Smith')
        ownerData.insert('phone', '555-5555')
        ownerData.insert('newValue', '')

        view = QtQuick.QQuickView()
        ctxt = view.rootContext()
        ctxt.setContextProperty('owner', ownerData)
        view.setSource(QtCore.QUrl.fromLocalFile(adjust_filename('bug_997.qml', __file__)))
        view.show()
        QtCore.QTimer.singleShot(1000, self.app.quit)
        self.app.exec_()
        self.assertEqual(ownerData.value('newName'), ownerData.value('name'))

if __name__ == '__main__':
    unittest.main()
