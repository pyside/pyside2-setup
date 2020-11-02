#!/usr/bin/python

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

from PySide6.QtCore import QDate, QDateTime, QTime, QUrl
from PySide6.QtCore import QLine, QPoint, QRect, QSize

class HashTest(unittest.TestCase):
    def testInsert(self):
        myHash = {}
        qdate = QDate.currentDate()
        qdatetime = QDateTime.currentDateTime()
        qtime = QTime.currentTime()
        qurl = QUrl("http://www.pyside.org")
        qpoint = QPoint(12, 42)

        myHash[qdate] = "QDate"
        myHash[qdatetime] = "QDateTime"
        myHash[qtime] = "QTime"
        myHash[qurl] = "QUrl"
        myHash[qpoint] = "QPoint"

        self.assertEqual(myHash[qdate], "QDate")
        self.assertEqual(myHash[qdatetime], "QDateTime")
        self.assertEqual(myHash[qtime], "QTime")
        self.assertEqual(myHash[qurl], "QUrl")
        self.assertEqual(myHash[qpoint], "QPoint")

    def testQPointHash(self):
        p1 = QPoint(12, 34)
        p2 = QPoint(12, 34)
        self.assertFalse(p1 is p2)
        self.assertEqual(p1, p2)
        self.assertEqual(hash(p1), hash(p2))

    def testQSizeHash(self):
        s1 = QSize(12, 34)
        s2 = QSize(12, 34)
        self.assertFalse(s1 is s2)
        self.assertEqual(s1, s2)
        self.assertEqual(hash(s1), hash(s2))

    def testQRectHash(self):
        r1 = QRect(12, 34, 56, 78)
        r2 = QRect(12, 34, 56, 78)
        self.assertFalse(r1 is r2)
        self.assertEqual(r1, r2)
        self.assertEqual(hash(r1), hash(r2))

    def testQLineHash(self):
        l1 = QLine(12, 34, 56, 78)
        l2 = QLine(12, 34, 56, 78)
        self.assertFalse(l1 is l2)
        self.assertEqual(l1, l2)
        self.assertEqual(hash(l1), hash(l2))

if __name__ == '__main__':
    unittest.main()

