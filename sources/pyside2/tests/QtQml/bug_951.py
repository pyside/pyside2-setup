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

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper.helper import adjust_filename
from helper.timedqapplication import TimedQApplication

from PySide2.QtCore import QUrl
from PySide2.QtQml import qmlRegisterType
from PySide2.QtQuick import QQuickItem, QQuickView

class MyItem(QQuickItem):
    COMPONENT_COMPLETE_CALLED = False
    def __init__(self,parent=None):
        super(MyItem, self).__init__(parent)
        self.setObjectName("myitem")

    def componentComplete(self):
        MyItem.COMPONENT_COMPLETE_CALLED = True
        super(MyItem, self).componentComplete()

class TestRegisterQMLType(TimedQApplication):
    def setup(self):
        TimedQApplication.setup(self, 100 * 3) # 3s

    def testSignalEmission(self):
        qmlRegisterType(MyItem, "my.item", 1, 0, "MyItem")

        view = QQuickView()
        view.setSource(QUrl.fromLocalFile(adjust_filename('bug_951.qml', __file__)))

        self.app.exec_()
        self.assertTrue(MyItem.COMPONENT_COMPLETE_CALLED)

if __name__ == '__main__':
    unittest.main()
