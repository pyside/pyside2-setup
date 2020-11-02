#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

'''Test case for PYSIDE-1354: Ensure that slots are invoked from the receiver's
thread context when using derived classes (and thus, a global receiver).'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QObject, QThread, QTimer, Slot
from helper.usesqcoreapplication import UsesQCoreApplication


class ReceiverBase(QObject):
    def __init__(self, parent=None):
        super(ReceiverBase, self).__init__(parent)
        self.senderThread = None

    @Slot()
    def slot_function(self):
        self.senderThread = QThread.currentThread()


class Receiver(ReceiverBase):
    pass


class TestThread(QThread):
    def __init__(self, parent=None):
        super(TestThread, self).__init__(parent)

    def run(self):
        pass


class SignalAcrossThreads(UsesQCoreApplication):
    def setUp(self):
        UsesQCoreApplication.setUp(self)
        self._timer_tick = 0
        self._timer = QTimer()
        self._timer.setInterval(20)
        self._timer.timeout.connect(self._control_test)
        self._worker_thread = TestThread()

    def tearDown(self):
        UsesQCoreApplication.tearDown(self)

    @Slot()
    def _control_test(self):
        if self._timer_tick == 0:
            self._worker_thread.start()
        elif self._timer_tick == 1:
            self._worker_thread.wait()
        else:
            self._timer.stop()
            self.app.quit()
        self._timer_tick += 1

    def test(self):
        worker_thread_receiver = Receiver()
        worker_thread_receiver.moveToThread(self._worker_thread)
        self._worker_thread.started.connect(worker_thread_receiver.slot_function)

        main_thread = QThread.currentThread()
        main_thread_receiver = Receiver()
        self._worker_thread.started.connect(main_thread_receiver.slot_function)

        self._timer.start()
        self.app.exec_()

        self.assertEqual(worker_thread_receiver.senderThread, self._worker_thread)
        self.assertEqual(main_thread_receiver.senderThread, main_thread)


if __name__ == '__main__':
    unittest.main()
