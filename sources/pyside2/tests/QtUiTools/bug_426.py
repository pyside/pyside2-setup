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

import sys
import os
from PySide2 import QtCore, QtWidgets
from PySide2.QtUiTools import QUiLoader

class Window(object):
    def __init__(self):
        loader = QUiLoader()
        filePath = os.path.join(os.path.dirname(__file__), 'bug_426.ui')
        self.widget = loader.load(filePath)
        self.group = QtWidgets.QActionGroup(self.widget)
        self.widget.show()
        QtCore.QTimer.singleShot(0, self.widget.close)

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    win = Window()
    sys.exit(app.exec_())
