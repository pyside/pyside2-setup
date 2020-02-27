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

from __future__ import print_function

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QBuffer, Qt, QTimer
from PySide2.QtWidgets import QApplication, QWidget, QVBoxLayout
from PySide2.QtWebEngine import QtWebEngine
from PySide2.QtWebEngineWidgets import QWebEngineView, QWebEngineProfile
from PySide2.QtWebEngineCore import (QWebEngineUrlScheme,
                                     QWebEngineUrlSchemeHandler)
import py3kcompat as py3k

class TestSchemeHandler(QWebEngineUrlSchemeHandler):
    def requestStarted(self, request):
        if request.requestUrl() == "testpy:hello":
            request.redirect("testpy:goodbye")
            return

        self.buffer = QBuffer()
        self.buffer.setData(py3k.b("Really nice goodbye text."))
        self.buffer.aboutToClose.connect(self.buffer.deleteLater)
        request.reply(py3k.b("text/plain;charset=utf-8"), self.buffer)


class MainTest(unittest.TestCase):
    def test_SchemeHandlerRedirect(self):
        self._loaded = False
        QApplication.setAttribute(Qt.AA_ShareOpenGLContexts);
        QApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
        QtWebEngine.initialize()
        app = QApplication([])

        scheme_name = py3k.b("testpy")
        scheme = QWebEngineUrlScheme(scheme_name)
        scheme.setSyntax(QWebEngineUrlScheme.Syntax.Path)
        QWebEngineUrlScheme.registerScheme(scheme)
        handler = TestSchemeHandler()
        profile = QWebEngineProfile.defaultProfile()
        profile.installUrlSchemeHandler(scheme_name, handler)

        top_level_widget = QWidget()
        top_level_widget.setWindowTitle('web_engine_custom_scheme.py')
        top_level_widget.resize(400, 400)
        layout = QVBoxLayout(top_level_widget)
        view = QWebEngineView()
        layout.addWidget(view)

        view.loadFinished.connect(self._slot_loaded)
        QTimer.singleShot(5000, app.quit)

        top_level_widget.show()
        view.load("testpy:hello")
        app.exec_()

        self.assertTrue(self._loaded)
        self.assertEqual(view.url(), "testpy:goodbye")

    def _slot_loaded(self):
        self._loaded = True
        QApplication.quit()


if __name__ == '__main__':
    unittest.main()
