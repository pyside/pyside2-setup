#!/usr/bin/python

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

from helper.timedqapplication import TimedQApplication
from PySide2.QtCore import QSignalTransition, QState, Qt, QStateMachine
from PySide2.QtWidgets import QCheckBox

class CheckedTransition(QSignalTransition):
    def __init__(self, check):
        QSignalTransition.__init__(self, check.stateChanged[int])
        self.eventTested = False

    def eventTest(self, event):
        self.eventTested = True
        if not QSignalTransition.eventTest(self, event):
            return False
        return event.arguments()[0] == Qt.Checked

class TestBug(TimedQApplication):
    def testCase(self):
        check = QCheckBox()
        check.setTristate(True)

        s1 = QState()
        s2 = QState()

        t1 = CheckedTransition(check)
        t1.setTargetState(s2)
        s1.addTransition(t1)

        machine = QStateMachine()
        machine.addState(s1)
        machine.addState(s2)
        machine.setInitialState(s1)
        machine.start()

        check.stateChanged[int].emit(1)
        check.show()
        self.app.exec_()
        self.assertTrue(t1.eventTested)

if __name__ == '__main__':
    unittest.main()
