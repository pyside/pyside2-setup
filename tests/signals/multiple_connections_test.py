#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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
import unittest
from functools import partial

from PySide2.QtCore import QObject, SIGNAL, QProcess

from helper import BasicPySlotCase, UsesQCoreApplication


class MultipleSignalConnections(unittest.TestCase):
    '''Base class for multiple signal connection testing'''

    def run_many(self, sender, signal, emitter, receivers, args=None):
        """Utility method to connect a list of receivers to a signal.
        sender - QObject that will emit the signal
        signal - string with the signal signature
        emitter - the callable that will trigger the signal
        receivers - list of BasicPySlotCase instances
        args - tuple with the arguments to be sent.
        """

        if args is None:
            args = tuple()
        for rec in receivers:
            rec.setUp()
            self.assertTrue(QObject.connect(sender, SIGNAL(signal), rec.cb))
            rec.args = tuple(args)

        emitter(*args)

        for rec in receivers:
            self.assertTrue(rec.called)


class PythonMultipleSlots(UsesQCoreApplication, MultipleSignalConnections):
    '''Multiple connections to python signals'''

    def testPythonSignal(self):
        """Multiple connections to a python signal (short-circuit)"""

        class Dummy(QObject):
            pass

        sender = Dummy()
        receivers = [BasicPySlotCase() for x in range(10)]
        self.run_many(sender, 'foobar(int)', partial(sender.emit,SIGNAL('foobar(int)')), receivers, (0, ))


class QProcessMultipleSlots(UsesQCoreApplication, MultipleSignalConnections):
    '''Multiple connections to QProcess signals'''

    def testQProcessStarted(self):
        '''Multiple connections to QProcess.started()'''
        sender = QProcess()
        receivers = [BasicPySlotCase() for x in range(10)]

        def start_proc(*args):
            sender.start(sys.executable, ['-c', '""'])
            sender.waitForFinished()

        self.run_many(sender, 'started()', start_proc, receivers)

    def testQProcessFinished(self):
        '''Multiple connections to QProcess.finished(int)'''
        sender = QProcess()
        receivers = [BasicPySlotCase() for x in range(10)]

        def start_proc(*args):
            sender.start(sys.executable, ['-c', '""'])
            sender.waitForFinished()

        self.run_many(sender, 'finished(int)', start_proc, receivers, (0,))

if __name__ == '__main__':
    unittest.main()
