#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

from __future__ import print_function

import unittest

from PySide2.QtCore import QBuffer, QTimer
from PySide2.QtWidgets import QApplication
from PySide2.QtWebEngineWidgets import QWebEngineView, QWebEngineProfile
from PySide2.QtWebEngineCore import QWebEngineUrlSchemeHandler

class TestSchemeHandler(QWebEngineUrlSchemeHandler):
    def requestStarted(self, request):
        if request.requestUrl() == "testpy:hello":
            request.redirect("testpy:goodbye")
            return

        self.buffer = QBuffer()
        self.buffer.setData("Really nice goodbye text.")
        self.buffer.aboutToClose.connect(self.buffer.deleteLater)
        request.reply("text/plain;charset=utf-8", self.buffer)

class MainTest(unittest.TestCase):
    def test_SchemeHandlerRedirect(self):
        app = QApplication([])
        handler = TestSchemeHandler()
        profile = QWebEngineProfile.defaultProfile()
        profile.installUrlSchemeHandler("testpy", handler)
        view = QWebEngineView()
        view.loadFinished.connect(app.quit)
        QTimer.singleShot(5000, app.quit)
        view.show()
        view.load("testpy:hello")
        app.exec_()
        self.assertEqual(view.url(), "testpy:goodbye")

if __name__ == '__main__':
    unittest.main()
