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

import sys
import unittest
from PySide2.QtCore import *
from PySide2.QtWidgets import *
from PySide2.QtWebKit import *

class ErrorPage (QWebPage):

    def __init__(self):
        QWebPage.__init__(self)
        self.rcv_extension = None
        self.rcv_url = None
        self.rcv_url_copy = None
        self.rcv_option_type = None
        self.rcv_output_type = None


    def supportsExtension(self, extension):
        return extension == QWebPage.ErrorPageExtension

    def extension(self, extension, option, output):
        self.rcv_extension = extension
        self.rcv_url = option.url
        self.rcv_url_copy = QUrl(option.url)
        self.rcv_option_type = type(option)
        self.rcv_output_type = type(output)
        return True

class TestWebPageExtension(unittest.TestCase):
    def testIt(self):
        app = QApplication([])
        ep = ErrorPage()
        view = QWebView()
        view.setPage(ep)
        view.load("foo://bar") # Some malformmed url
        view.show()

        # If the timeout is 0 the webpage isn't even loaded on Qt4.6-i386, so we use 100 :-)
        QTimer.singleShot(100, app.quit)
        app.exec_()

        self.assertEqual(ep.rcv_extension, QWebPage.ErrorPageExtension)
        self.assertRaises(RuntimeError, ep.rcv_url.__str__)

        self.assertEqual(ep.rcv_url_copy, "foo://bar")
        self.assertEqual(ep.rcv_option_type, QWebPage.ErrorPageExtensionOption)
        self.assertEqual(ep.rcv_output_type, QWebPage.ErrorPageExtensionReturn)

if __name__ == '__main__':
    unittest.main()
