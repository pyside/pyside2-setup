#!/usr/bin/python

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

'''Test cases for QLineF'''

import unittest
import datetime

from PySide2.QtCore import QTime, QDateTime, QDate

class TestDateTimeConversions (unittest.TestCase):
    def testQDate(self):
        date = datetime.date(2010, 4, 23)
        other = QDate(date)
        self.assertEqual(date.year, other.year())
        self.assertEqual(date.month, other.month())
        self.assertEqual(date.day, other.day())

        self.assertEqual(date, other.toPython())

    def testQTime(self):
        time = datetime.time(11, 14, 0, 1000)
        other = QTime(time)
        self.assertEqual(time.hour, other.hour())
        self.assertEqual(time.minute, other.minute())
        self.assertEqual(time.second, other.second())
        self.assertEqual(time.microsecond/1000, other.msec())

        self.assertEqual(time, other.toPython())

    def testQDateTime(self):
        dateTime = datetime.datetime(2010, 4, 23, 11, 14, 0, 1000)
        other = QDateTime(dateTime)

        otherDate = other.date()
        self.assertEqual(dateTime.year, otherDate.year())
        self.assertEqual(dateTime.month, otherDate.month())
        self.assertEqual(dateTime.day, otherDate.day())

        otherTime = other.time()
        self.assertEqual(dateTime.hour, otherTime.hour())
        self.assertEqual(dateTime.minute, otherTime.minute())
        self.assertEqual(dateTime.second, otherTime.second())
        self.assertEqual(dateTime.microsecond/1000, otherTime.msec())

        self.assertEqual(dateTime, other.toPython())

    def testQDateTime6arg(self):
        dateTime = datetime.datetime(2010, 4, 23, 11, 14, 7)
        other = QDateTime(dateTime)

        otherDate = other.date()
        self.assertEqual(dateTime.year, otherDate.year())
        self.assertEqual(dateTime.month, otherDate.month())
        self.assertEqual(dateTime.day, otherDate.day())

        otherTime = other.time()
        self.assertEqual(dateTime.hour, otherTime.hour())
        self.assertEqual(dateTime.minute, otherTime.minute())
        self.assertEqual(dateTime.second, otherTime.second())
        self.assertEqual(dateTime.microsecond/1000, otherTime.msec())

        self.assertEqual(dateTime, other.toPython())

if __name__ == '__main__':
    unittest.main()
