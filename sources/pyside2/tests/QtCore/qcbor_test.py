#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

'''Test cases for QCbor'''

import unittest

from PySide2.QtCore import (QByteArray, QCborStreamReader, QCborStreamWriter,
    QCborValue)

class TestCbor(unittest.TestCase):
    def testReader(self):
        ba = QByteArray()
        writer = QCborStreamWriter(ba)
        writer.append(42)
        del writer
        self.assertTrue(not ba.isEmpty())
        reader = QCborStreamReader(ba)
        self.assertTrue(reader.hasNext())
        value = reader.toInteger()
        self.assertEqual(value, 42)

    def testReader(self):
        ba = QByteArray()
        writer = QCborStreamWriter(ba)
        writer.append("hello")
        del writer
        self.assertTrue(not ba.isEmpty())
        reader = QCborStreamReader(ba)
        self.assertTrue(reader.hasNext())
        if (reader.isByteArray()): # Python 2
            value = reader.readByteArray()
            self.assertTrue(value)
            self.assertEqual(value.data, "hello")
        else:
            self.assertTrue(reader.isString())
            value = reader.readString()
            self.assertTrue(value)
            self.assertEqual(value.data, "hello")

    def testValue(self):
        value = QCborValue('hello')
        self.assertTrue(value.isString())
        self.assertEqual(value.toString(), 'hello')

if __name__ == '__main__':
    unittest.main()
