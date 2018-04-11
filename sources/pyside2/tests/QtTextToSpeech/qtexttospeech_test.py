#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

'''Test cases for QTextToSpeech methods'''

from helper import UsesQApplication
import sys
import unittest

from PySide2.QtCore import QTimer

try:
    from PySide2.QtTextToSpeech import QTextToSpeech, QVoice
except ImportError:
    print("Skipping test due to missing QtTextToSpeech module")
    sys.exit(0)

class QTextToSpeechTestCase(UsesQApplication):
    '''Tests related to QTextToSpeech'''
    def testSay(self):
        engines = QTextToSpeech.availableEngines()
        if not engines:
            print('No QTextToSpeech engines available')
        else:
            speech = QTextToSpeech(engines[0])
            speech.stateChanged.connect(self._slotStateChanged)
            speech.say("Hello, PySide2")
            QTimer.singleShot(5000, self.app.quit)
            self.app.exec_()

    def _slotStateChanged(self, state):
        if (state == QTextToSpeech.State.Ready):
            self.app.quit()

if __name__ == '__main__':
    unittest.main()
