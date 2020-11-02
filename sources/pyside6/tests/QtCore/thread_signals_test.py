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

''' Test case for QObject.signalsBlocked() and blockSignal()'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QObject, SIGNAL, QFile, QThread, QTimer, Qt
from helper.usesqcoreapplication import UsesQCoreApplication

class MyThread(QThread):

    def run(self):
        self.emit(SIGNAL("test(const QString&)"), "INdT - PySide");

class TestThreadSignal(UsesQCoreApplication):

    __called__ = True
    def _callback(self, msg):
        self.assertEqual(msg, "INdT - PySide")
        self.__called__ = True
        self.app.quit()

    def testThread(self):
        t = MyThread()
        QObject.connect(t, SIGNAL("test(const QString&)"), self._callback);
        t.start()

        self.app.exec_()
        t.wait()
        self.assertTrue(self.__called__);

if __name__ == '__main__':
    unittest.main()
