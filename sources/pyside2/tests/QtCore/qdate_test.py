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

'''Test cases for QDate'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import *

class TestQDate (unittest.TestCase):
    def testGetDate(self):
        date = QDate(2009, 22, 9)
        tuple_ = date.getDate()
        self.assertEqual(tuple, tuple_.__class__)
        (y, m, d) = tuple_
        self.assertEqual(date.year(), y)
        self.assertEqual(date.month(), m)
        self.assertEqual(date.day(), d)

    def testGetWeekNumber(self):
        date = QDate(2000, 1, 1)
        tuple_ = date.weekNumber()
        self.assertEqual(tuple, tuple_.__class__)
        (week, yearNumber) = tuple_
        self.assertEqual(week, 52)
        self.assertEqual(yearNumber, 1999)

    def testBooleanCast(self):
        today = QDate.currentDate()
        self.assertTrue(today)
        nodate = QDate()
        self.assertFalse(nodate)

if __name__ == '__main__':
    unittest.main()
