#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
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

'''Test cases for QQmlNetwork'''

from __future__ import print_function

import unittest

from PySide2.QtCore import QUrl
from PySide2.QtQuick import QQuickView
from PySide2.QtQml import QQmlNetworkAccessManagerFactory
from PySide2.QtNetwork import QNetworkAccessManager

from helper import adjust_filename, TimedQApplication

class CustomManager(QNetworkAccessManager):
    def createRequest(self, op, req, data = None):
        print(">> createRequest ", self, op, req.url(), data)
        return QNetworkAccessManager.createRequest(self, op, req, data)

class CustomFactory(QQmlNetworkAccessManagerFactory):
    def create(self, parent = None):
        return CustomManager()

class TestQQmlNetworkFactory(TimedQApplication):
    def setUp(self):
        TimedQApplication.setUp(self, timeout=1000)

    def testQQuickNetworkFactory(self):
        view = QQuickView()
        self.factory = CustomFactory()
        view.engine().setNetworkAccessManagerFactory(self.factory)

        url = QUrl.fromLocalFile(adjust_filename('hw.qml', __file__))

        view.setSource(url)
        view.show()

        self.assertEqual(view.status(), QQuickView.Ready)

        self.app.exec_()

if __name__ == '__main__':
    unittest.main()
