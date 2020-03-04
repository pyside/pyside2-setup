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

from PySide2.QtCore import SIGNAL, QTimer
from PySide2.QtScript import QScriptEngine
from PySide2.QtScriptTools import QScriptEngineDebugger

from helper.usesqapplication import UsesQApplication

class DebuggerTest(UsesQApplication):

    def setUp(self):
        UsesQApplication.setUp(self)
        self.engine = QScriptEngine()
        self.debugger = QScriptEngineDebugger()
        self.has_suspended = 0
        self.has_resumed = 0
        self.count = 3

    def suspended(self):
        self.has_suspended += 1
        # Will emit evaluationResumed until there are more instructions to be run
        QTimer.singleShot(100, self.debugger.action(QScriptEngineDebugger.StepIntoAction).trigger)

    def resumed(self):
        # Will be called when debugger.state() change from Suspended to Running
        # except for the first time.
        self.has_resumed += 1

    def testBasic(self):
        '''Interrupt and resume evaluation with QScriptEngineDebugger'''

        self.debugger.attachTo(self.engine)
        self.debugger.setAutoShowStandardWindow(False)
        self.debugger.connect(SIGNAL('evaluationSuspended()'), self.suspended)
        self.debugger.connect(SIGNAL('evaluationResumed()'), self.resumed)

        # For some reason StepIntoAction does not actually continue execution, and thus interrupting
        # causes the test to hang. The same behavior is present in a Qt5.6 C++ code equivalent. It
        # seems like a bug in QtScript, thus the interruption is commented out for now, which will
        # force the test to fail.
        #self.debugger.action(QScriptEngineDebugger.InterruptAction).trigger()
        self.engine.evaluate("3+4\n2+1\n5+1")
        self.assertTrue(self.has_resumed >= 1)
        self.assertTrue(self.has_suspended >= 1)

if __name__ == '__main__':
    unittest.main()
