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

'''Unit tests for QLocale'''

import unittest
import ctypes
import sys

from PySide2.QtCore import QLocale

class QLocaleTestToNumber(unittest.TestCase):
    def testToNumberInt(self):
        obj = QLocale(QLocale.C)
        self.assertEqual((37, True), obj.toInt('37'))

    def testToNumberFloat(self):
        obj = QLocale(QLocale.C)
        self.assertEqual((ctypes.c_float(37.109).value, True),
                         obj.toFloat('37.109'))

    def testToNumberDouble(self):
        obj = QLocale(QLocale.C)
        self.assertEqual((ctypes.c_double(37.109).value, True),
                         obj.toDouble('37.109'))

    def testToNumberShort(self):
        obj = QLocale(QLocale.C)
        self.assertEqual((ctypes.c_short(37).value, True),
                         obj.toShort('37'))

    def testToNumberULongLong(self):
        obj = QLocale(QLocale.C)
        self.assertEqual((ctypes.c_ulonglong(37).value, True),
                         obj.toULongLong('37'))

    def testToNumberULongLongNegative(self):
        obj = QLocale(QLocale.C)
        self.assertTrue(not obj.toULongLong('-37')[1])

if __name__ == '__main__':
    unittest.main()
