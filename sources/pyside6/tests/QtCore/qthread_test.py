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

'''Test cases for QThread'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QThread, QCoreApplication, QObject, SIGNAL, QMutex, QTimer
from PySide6.QtCore import QEventLoop

from helper.usesqcoreapplication import UsesQCoreApplication

mutex = QMutex()

class Dummy(QThread):
    '''Dummy thread'''
    def __init__(self, *args):
        super(Dummy, self).__init__(*args)
        self.called = False

    def run(self):
        #Start-quit sequence
        self.qobj = QObject()
        mutex.lock()
        self.called = True
        mutex.unlock()

class QThreadSimpleCase(UsesQCoreApplication):

    def setUp(self):
        UsesQCoreApplication.setUp(self)
        self.called = False

    def tearDown(self):
        UsesQCoreApplication.tearDown(self)

    def testThread(self):
        #Basic QThread test
        obj = Dummy()
        obj.start()
        self.assertTrue(obj.wait(100))

        self.assertTrue(obj.called)

    def cb(self, *args):
        self.called = True
        #self.exit_app_cb()

    def abort_application(self):
        if self._thread.isRunning():
            self._thread.terminate()
        self.app.quit()

    def testSignalFinished(self):
        #QThread.finished() (signal)
        obj = Dummy()
        QObject.connect(obj, SIGNAL('finished()'), self.cb)
        mutex.lock()
        obj.start()
        mutex.unlock()

        self._thread = obj
        QTimer.singleShot(1000, self.abort_application)
        self.app.exec_()

        self.assertTrue(self.called)

    def testSignalStarted(self):
        #QThread.started() (signal)
        obj = Dummy()
        QObject.connect(obj, SIGNAL('started()'), self.cb)
        obj.start()

        self._thread = obj
        QTimer.singleShot(1000, self.abort_application)
        self.app.exec_()

        self.assertEqual(obj.qobj.thread(), obj) # test QObject.thread() method
        self.assertTrue(self.called)

if __name__ == '__main__':
    unittest.main()
