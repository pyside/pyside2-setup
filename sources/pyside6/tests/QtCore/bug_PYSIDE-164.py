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

from __future__ import print_function

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QCoreApplication, QEventLoop, QObject, Qt, QThread, QTimer, SIGNAL

class Emitter(QThread):
    def __init__(self):
        QThread.__init__(self)

    def run(self):
        print("Before emit.")
        self.emit(SIGNAL("signal(int)"), 0)
        print("After emit.")

class Receiver(QObject):
    def __init__(self, eventloop):
        QObject.__init__(self)
        self.eventloop = eventloop

    def receive(self, number):
        print("Received number: %d" % number)
        self.eventloop.exit(0)


class TestBugPYSIDE164(unittest.TestCase):

    def testBlockingSignal(self):
        app = QCoreApplication.instance() or QCoreApplication([])
        eventloop = QEventLoop()
        emitter = Emitter()
        receiver = Receiver(eventloop)
        emitter.connect(emitter, SIGNAL("signal(int)"),
                        receiver.receive, Qt.BlockingQueuedConnection)
        emitter.start()
        retval = eventloop.exec_()
        emitter.wait(2000)
        self.assertEqual(retval, 0)

if __name__ == '__main__':
    unittest.main()
