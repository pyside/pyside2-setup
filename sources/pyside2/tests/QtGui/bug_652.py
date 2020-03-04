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

from PySide2.QtGui import *

class MyData(QTextBlockUserData):
    def __init__(self, data):
        QTextBlockUserData.__init__(self)
        self.data = data

    def getMyNiceData(self):
        return self.data

class TestBug652(unittest.TestCase):
    """Segfault when using QTextBlock::setUserData due to missing ownership transfer"""
    def testIt(self):
        td = QTextDocument()
        tc = QTextCursor(td)
        tc.insertText("Hello world")
        heyHo = "hey ho!"
        tc.block().setUserData(MyData(heyHo))
        self.assertEqual(type(tc.block().userData()), MyData)
        self.assertEqual(tc.block().userData().getMyNiceData(), heyHo)

        del tc
        tc = QTextCursor(td)
        blk = tc.block()
        self.assertEqual(type(blk.userData()), MyData)
        self.assertEqual(blk.userData().getMyNiceData(), heyHo)

if __name__ == "__main__":
    unittest.main()
