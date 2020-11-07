#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

from functools import partial
import os
import sys
import unittest

TEST_DIR = os.path.dirname(os.path.abspath(__file__))

sys.path.append(os.path.dirname(TEST_DIR))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QCoreApplication, QSize, QUrl, Qt
from PySide6.QtWidgets import QApplication, QVBoxLayout, QWidget
from PySide6.QtWebEngineWidgets import QWebEnginePage, QWebEngineView


class MainTest(unittest.TestCase):

    def test_WebEngineView_findText_exists(self):
        QCoreApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
        app = QApplication.instance() or QApplication()
        top_level = QWidget()
        layout = QVBoxLayout(top_level)
        self._view = QWebEngineView()
        self._view.loadFinished.connect(self.loaded)
        self._view.load(QUrl.fromLocalFile(os.path.join(TEST_DIR, "fox.html")))
        self._view.setMinimumSize(QSize(400, 300))
        self._callback_count = 0
        layout.addWidget(self._view)
        top_level.show()
        app.exec_()

    def found_callback(self, found):
        self.assertTrue(found)
        self._callback_count += 1
        if self._callback_count == 2:
            QCoreApplication.quit()

    def javascript_callback(self, result):
        self.assertEqual(result, "Title")
        self._callback_count += 1
        if self._callback_count == 2:
            QCoreApplication.quit()

    def loaded(self, ok):
        self.assertTrue(ok)
        if not ok:
            QCoreApplication.quit()
        self._view.page().runJavaScript("document.title", 1,
                                        partial(self.javascript_callback))
        self._view.findText("fox", QWebEnginePage.FindFlags(),
                            partial(self.found_callback))


if __name__ == '__main__':
    unittest.main()
