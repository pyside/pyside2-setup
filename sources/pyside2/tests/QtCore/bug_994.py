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

import unittest
import py3kcompat as py3k
from PySide2.QtCore import QIODevice, QTextStream


class MyIODevice (QIODevice):
    def readData(self, amount):
        return py3k.b("\0a" * int(amount/2))

    def readLineData(self, maxSize):
        return py3k.b("\0b" * 4)

    def atEnd(self):
        return False

class TestBug944 (unittest.TestCase):

    def testIt(self):
        device = MyIODevice()
        device.open(QIODevice.ReadOnly)
        s = QTextStream(device)
        self.assertEqual(s.read(4), "\0a\0a")
        self.assertEqual(device.readLine(), "\0b\0b\0b\0b")

if __name__ == "__main__":
    unittest.main()
