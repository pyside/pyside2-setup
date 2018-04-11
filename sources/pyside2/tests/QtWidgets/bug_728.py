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

from PySide2.QtWidgets import *
from PySide2.QtCore import *

# Periodically check for the file dialog to appear and close it
dialog = None
def timerHandler():
    global dialog
    if dialog is not None:
        dialog.reject()
    else:
        for widget in QApplication.topLevelWidgets():
            if isinstance(widget, QDialog) and widget.isVisible():
               dialog = widget

app = QApplication([])
QTimer.singleShot(30000, app.quit) # emergency
timer = QTimer()
timer.setInterval(50)
timer.timeout.connect(timerHandler)
timer.start()

# This test for a dead lock in QFileDialog.getOpenFileNames, the test fail with a timeout if the dead lock exists.
QFileDialog.getOpenFileNames(None, "caption", QDir.homePath(), None, "", QFileDialog.DontUseNativeDialog)
