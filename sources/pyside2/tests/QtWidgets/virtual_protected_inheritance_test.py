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

'''Test cases for overriding inherited protected virtual methods'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QTimerEvent
from PySide2.QtWidgets import QApplication, QSpinBox

from helper.usesqapplication import UsesQApplication

class MySpinButton(QSpinBox):
    '''Simple example class of overriding QObject.timerEvent'''

    def __init__(self, max_runs=5, app=None):
        #Creates a new spinbox that will run <max_runs> and quit <app>
        super(MySpinButton, self).__init__()

        if app is None:
            app = QApplication([])

        self.app = app
        self.max_runs = max_runs
        self.runs = 0

    def timerEvent(self, event):
        #Timer event method
        self.runs += 1

        self.setValue(self.runs)

        if self.runs == self.max_runs:
            self.app.quit()

        if not isinstance(event, QTimerEvent):
            raise TypeError('Invalid event type. Must be TimerEvent')

class TimerEventTest(UsesQApplication):
    '''Test case for running QObject.timerEvent from inherited class'''

    qapplication = True

    def setUp(self):
        #Acquire resources
        super(TimerEventTest, self).setUp()
        self.widget = MySpinButton(app=self.app)

    def tearDown(self):
        #Release resources
        del self.widget
        super(TimerEventTest, self).tearDown()

    def testMethod(self):
        #QWidget.timerEvent overrinding (protected inherited)
        timer_id = self.widget.startTimer(0)

        self.app.exec_()

        self.widget.killTimer(timer_id)

        self.assertTrue(self.widget.runs >= self.widget.max_runs)


if __name__ == '__main__':
    unittest.main()
    #app = QApplication([])
    #widget  = MySpinButton(app=app)
    #widget.startTimer(500)
    #widget.show()
    #app.exec_()

