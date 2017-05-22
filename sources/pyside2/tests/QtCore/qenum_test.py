#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
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

'''Test cases for QEnum and QFlags'''

import unittest

from PySide2.QtCore import *

class TestEnum(unittest.TestCase):

    def testToInt(self):
        self.assertEqual(QIODevice.NotOpen, 0)
        self.assertEqual(QIODevice.ReadOnly, 1)
        self.assertEqual(QIODevice.WriteOnly, 2)
        self.assertEqual(QIODevice.ReadWrite, 1 | 2)
        self.assertEqual(QIODevice.Append, 4)
        self.assertEqual(QIODevice.Truncate, 8)
        self.assertEqual(QIODevice.Text, 16)
        self.assertEqual(QIODevice.Unbuffered, 32)

    def testToIntInFunction(self):
        self.assertEqual(str(int(QIODevice.WriteOnly)), "2")

class TestQFlags(unittest.TestCase):
    def testToItn(self):
        om = QIODevice.NotOpen

        self.assertEqual(om, QIODevice.NotOpen)
        self.assertTrue(om == 0)

        self.assertTrue(om != QIODevice.ReadOnly)
        self.assertTrue(om != 1)

    def testToIntInFunction(self):
        om = QIODevice.WriteOnly
        self.assertEqual(int(om), 2)

    def testNonExtensibleEnums(self):
        try:
            om = QIODevice.OpenMode(QIODevice.WriteOnly)
            self.assertFail()
        except:
            pass

if __name__ == '__main__':
    unittest.main()
