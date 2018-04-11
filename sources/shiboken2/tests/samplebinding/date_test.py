#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

'''Test cases for python conversions types '''

import sys
import unittest
from datetime import date

from sample import SbkDate

class DateConversionTest(unittest.TestCase):

    def testConstructorWithDateObject(self):
        pyDate = date(2010, 12, 12)
        cDate = SbkDate(pyDate)
        self.assertTrue(cDate.day(), pyDate.day)
        self.assertTrue(cDate.month(), pyDate.month)
        self.assertTrue(cDate.year(), pyDate.year)

    def testToPythonFunction(self):
        cDate = SbkDate(2010, 12, 12)
        pyDate = cDate.toPython()
        self.assertTrue(cDate.day(), pyDate.day)
        self.assertTrue(cDate.month(), pyDate.month)
        self.assertTrue(cDate.year(), pyDate.year)

if __name__ == '__main__':
    unittest.main()

