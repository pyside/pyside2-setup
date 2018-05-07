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

'''Test cases for QWebView'''

import unittest
import py3kcompat as py3k
import sys

from PySide2.QtCore import QObject, SIGNAL, QUrl
from PySide2.QtWebKit import QWebPage, QWebView
from PySide2.QtNetwork import QNetworkRequest

from helper import adjust_filename, TimedQApplication


class testWebPage(QWebPage):
    def sayMyName(self):
        return 'testWebPage'

class TestLoadFinished(TimedQApplication):
    '''Test case for signal QWebView.loadFinished(bool)'''

    def setUp(self):
        #Acquire resources
        TimedQApplication.setUp(self, timeout=1000)
        self.view = QWebView()
        QObject.connect(self.view, SIGNAL('loadFinished(bool)'),
                        self.load_finished)
        self.called = False

    def tearDown(self):
        #Release resources
        del self.view
        self.called = False
        TimedQApplication.tearDown(self)

    def testLoadFinishedFromFile(self):
        url = QUrl.fromLocalFile(adjust_filename('fox.html', __file__))
        self.view.setUrl(url)
        self.app.exec_()

        self.assertTrue(self.called)

    def testSetPageAndGetPage(self):
        twp = testWebPage()
        self.view.setPage(twp)
        del twp
        p = self.view.page()
        self.assertEqual(p.sayMyName(), 'testWebPage')

        # Setting the same webpage should not incref the python obj
        refCount = sys.getrefcount(p)
        self.view.setPage(p)
        self.assertEqual(sys.getrefcount(p), refCount)

        # Changing the webpage obj should decref the old one
        twp2 = testWebPage()
        self.view.setPage(twp2)
        self.assertEqual(sys.getrefcount(p), refCount - 1)

    def load_finished(self, ok):
        #Callback to check if load was successful
        self.app.quit()
        if ok:
            self.called = True

if __name__ == '__main__':
    unittest.main()
