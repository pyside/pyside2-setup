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

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths()

from PySide6 import QtCore, QtWidgets

class Window(QtWidgets.QMainWindow):
    def childEvent(self, event):
        super(Window, self).childEvent(event)

app = QtWidgets.QApplication([])
window = Window()

dock1 = QtWidgets.QDockWidget()
dock2 = QtWidgets.QDockWidget()
window.addDockWidget(QtCore.Qt.LeftDockWidgetArea, dock1)
window.addDockWidget(QtCore.Qt.LeftDockWidgetArea, dock2)
window.tabifyDockWidget(dock1, dock2)

window.show()
QtCore.QTimer.singleShot(0, window.close)
app.exec_()
