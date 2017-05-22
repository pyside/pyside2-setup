#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

'''Unit tests for QUrlQuery'''

import unittest
import ctypes
import sys

from PySide2.QtCore import QUrlQuery

class QUrlQueryTest(unittest.TestCase):
    def testConstructing(self):
        empty = QUrlQuery()
        self.assertTrue(empty.isEmpty())
        self.assertEqual(empty.queryPairDelimiter(), QUrlQuery.defaultQueryPairDelimiter())
        self.assertEqual(empty.queryValueDelimiter(), QUrlQuery.defaultQueryValueDelimiter())

        empty.clear();
        self.assertTrue(empty.isEmpty())

    def testAddRemove(self):
        query = QUrlQuery()

        query.addQueryItem("a", "b");
        self.assertTrue(not query.isEmpty())
        self.assertTrue(query.hasQueryItem("a"))
        self.assertEqual(query.queryItemValue("a"), "b")
        self.assertEqual(query.allQueryItemValues("a"), ["b"])

if __name__ == '__main__':
    unittest.main()
