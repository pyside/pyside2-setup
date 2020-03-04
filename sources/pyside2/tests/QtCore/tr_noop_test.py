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

from PySide2.QtCore import QT_TR_NOOP, QT_TR_NOOP_UTF8
from PySide2.QtCore import QT_TRANSLATE_NOOP, QT_TRANSLATE_NOOP3, QT_TRANSLATE_NOOP_UTF8

class QtTrNoopTest(unittest.TestCase):

    def setUp(self):
        self.txt = 'Cthulhu fhtag!'

    def tearDown(self):
        del self.txt

    def testQtTrNoop(self):
        refcnt = sys.getrefcount(self.txt)
        result = QT_TR_NOOP(self.txt)
        self.assertEqual(result, self.txt)
        self.assertEqual(sys.getrefcount(result), refcnt + 1)

    def testQtTrNoopUtf8(self):
        refcnt = sys.getrefcount(self.txt)
        result = QT_TR_NOOP_UTF8(self.txt)
        self.assertEqual(result, self.txt)
        self.assertEqual(sys.getrefcount(result), refcnt + 1)

    def testQtTranslateNoop(self):
        refcnt = sys.getrefcount(self.txt)
        result = QT_TRANSLATE_NOOP(None, self.txt)
        self.assertEqual(result, self.txt)
        self.assertEqual(sys.getrefcount(result), refcnt + 1)

    def testQtTranslateNoopUtf8(self):
        refcnt = sys.getrefcount(self.txt)
        result = QT_TRANSLATE_NOOP_UTF8(self.txt)
        self.assertEqual(result, self.txt)
        self.assertEqual(sys.getrefcount(result), refcnt + 1)

    def testQtTranslateNoop3(self):
        refcnt = sys.getrefcount(self.txt)
        result = QT_TRANSLATE_NOOP3(None, self.txt, None)
        self.assertEqual(result, self.txt)
        self.assertEqual(sys.getrefcount(result), refcnt + 1)


if __name__ == '__main__':
    unittest.main()

