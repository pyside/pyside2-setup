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

from PySide2.QtCore import *
from PySide2.QtNetwork import *

from helper.usesqcoreapplication import UsesQCoreApplication


class HttpSignalsCase(UsesQCoreApplication):
    '''Test case for launching QHttp signals'''
    DATA = bytes("PySide rocks", "UTF-8")

    def onError(self):
        self.assertTrue(False)

    def onNewConnection(self):
        self.serverConnection = self.server.nextPendingConnection()
        self.serverConnection.errorOccurred.connect(self.onError)
        self.serverConnection.write(HttpSignalsCase.DATA)
        self.server.close()

    def onReadReady(self):
        data = self.client.read(100)
        self.assertEqual(len(data), len(HttpSignalsCase.DATA))
        self.assertEqual(data, HttpSignalsCase.DATA)
        self.done()

    def onClientConnect(self):
        self.client.readyRead.connect(self.onReadReady)

    def initServer(self):
        self.server = QTcpServer()
        self.server.newConnection.connect(self.onNewConnection)
        self.assertTrue(self.server.listen())
        self.client = QTcpSocket()
        self.client.connected.connect(self.onClientConnect)
        self.client.connectToHost(QHostAddress(QHostAddress.LocalHost), self.server.serverPort())

    def done(self):
        self.serverConnection.close()
        self.client.close()
        self.app.quit()

    def testRun(self):
        self.initServer()
        self.app.exec_()

if __name__ == '__main__':
    unittest.main()
