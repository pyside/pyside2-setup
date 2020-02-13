#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

from helper import TimedQApplication
from PySide2.QtCore import QObject, SIGNAL
from PySide2.QtScxml import QScxmlStateMachine

class testDynamicStateMachine(TimedQApplication):
    def setUp(self):
        super(testDynamicStateMachine, self).setUp()
        filePath = os.path.join(os.path.dirname(__file__), 'trafficlight.scxml')
        self.assertTrue(os.path.exists(filePath))
        self._machine = QScxmlStateMachine.fromFile(filePath)
        self._machine.reachedStableState.connect(self._reachedStable())
        self.assertTrue(not self._machine.parseErrors())
        self.assertTrue(self._machine)

    def _reachedStable(self):
        self.app.quit()

    def test(self):
        self._machine.start()

if __name__ == '__main__':
    unittest.main()
