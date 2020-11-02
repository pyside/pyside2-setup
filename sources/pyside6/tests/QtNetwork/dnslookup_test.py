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

'''Test cases for QDnsLookup'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QCoreApplication
from PySide6.QtNetwork import QDnsLookup

class DnsLookupTestCase(unittest.TestCase):
    '''Test case for QDnsLookup'''

    def setUp(self):
        self._app = QCoreApplication([])
        self._lookup = QDnsLookup(QDnsLookup.ANY, 'www.qt.io')
        self._lookup.finished.connect(self._finished)

    def tearDown(self):
        del self._lookup

    def _finished(self):
        if self._lookup.error() == QDnsLookup.NoError:
            nameRecords = self._lookup.canonicalNameRecords()
            if nameRecords:
                print(nameRecords[0].name())
        self._app.quit()

    def testLookup(self):
        self._lookup.lookup()
        self._app.exec_()

if __name__ == '__main__':
    unittest.main()
