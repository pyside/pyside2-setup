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

'''Test cases for QWebPage'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QObject, SIGNAL, QUrl
from PySide2.QtWebKit import QWebPage
from PySide2.QtNetwork import QNetworkAccessManager

from helper.helper import adjust_filename
from helper.timedqapplication import TimedQApplication

#Define a global timeout because TimedQApplication uses a singleton!
#Use a value big enough to run all the tests.
TIMEOUT = 1000

class TestFindText(TimedQApplication):
    '''Test cases for finding text'''

    def setUp(self):
        TimedQApplication.setUp(self, timeout=TIMEOUT)
        self.page = QWebPage()
        QObject.connect(self.page, SIGNAL('loadFinished(bool)'),
                        self.load_finished)
        self.called = False

    def tearDown(self):
        #Release resources
        del self.page
        self.called = False
        TimedQApplication.tearDown(self)

    def testFindSelectText(self):
        url = QUrl.fromLocalFile(adjust_filename('fox.html', __file__))
        self.page.mainFrame().load(url)
        self.app.exec_()
        self.assertTrue(self.called)

    def load_finished(self, ok):
        #Callback to check if load was successful
        if ok:
            self.called = True
            self.assertTrue(self.page.findText('fox'))
            self.assertEqual(self.page.selectedText(), 'fox')
        self.app.quit()

class SetNetworkAccessManagerCase(TimedQApplication):

    def setUp(self):
        TimedQApplication.setUp(self, timeout=TIMEOUT)

    def testSetNetworkAccessManager(self):
        page = QWebPage()
        manager = QNetworkAccessManager()
        page.setNetworkAccessManager(manager)

    def testNetWorkAccessManager(self):
        page = QWebPage()
        a = page.networkAccessManager()

if __name__ == '__main__':
    unittest.main()
