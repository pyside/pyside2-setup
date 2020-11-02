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

'''Unit tests for QDataStream'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QBitArray, QByteArray, QIODevice, QDataStream, QDate, QTime, QDateTime

def create_bitarray(string):
    array = QBitArray(len(string))
    for i, char in enumerate(string):
        array.setBit(i, char != '0')
    return array


def serialize_bitarray(bit_array):
    buffer = QByteArray()
    stream = QDataStream(buffer, QIODevice.WriteOnly)
    stream << bit_array
    return buffer


class QDataStreamWrite(unittest.TestCase):
    '''Test case for QDatastream write* functions'''

    def setUp(self):
        self.ba = QByteArray()
        self.read = QDataStream(self.ba, QIODevice.ReadOnly)
        self.write = QDataStream(self.ba, QIODevice.WriteOnly)

    def testWriteUInt8(self):
        '''QDataStream.writeUInt8 (accepting str of size 1)'''
        x = 0xFF
        self.write.writeUInt8(x)
        y = self.read.readUInt8()
        self.assertEqual(x, y)

        self.assertRaises(TypeError, self.write.writeUInt8, 'aaaaa')

    def testWriteInt8(self):
        '''QDataStream.writeInt8 (accepting str of size 1)'''
        x = 65
        self.write.writeInt8(chr(x))
        y = self.read.readInt8()
        self.assertEqual(x, y)

        self.assertRaises(TypeError, self.write.writeInt8, 'aaaaa')

    def testWriteUInt8Int(self):
        '''QDataStream.writeUInt8 (accepting integer)'''
        x = 0xFF
        self.write.writeUInt8(x)
        y = self.read.readUInt8()
        self.assertEqual(x, y)

    def testWriteInt8Int(self):
        '''QDataStream.writeInt8 (accepting integer)'''
        x = 65
        self.write.writeInt8(x)
        y = self.read.readInt8()
        self.assertEqual(x, y)

    def testWriteUInt16(self):
        '''QDataStream.writeUInt16'''
        x = 0x4423
        self.write.writeUInt16(x)
        y = int(self.read.readUInt16())
        self.assertEqual(x, y)

    def testWriteUInt32(self):
        '''QDataStream.writeUInt32'''
        x = 0xdeadbeef
        self.write.writeUInt32(x)
        y = int(self.read.readUInt32())
        self.assertEqual(x, y)

class QDataStreamShift(unittest.TestCase):
    '''Test case for << and >> operators'''

    def setUp(self):
        self.ba = QByteArray()
        self.stream = QDataStream(self.ba, QIODevice.WriteOnly)
        self.read_stream = QDataStream(self.ba, QIODevice.ReadOnly)

    def testQCharValid(self):
        '''QDataStream <<>> QChar - valid'''
        self.stream.writeQChar(42)

        res = self.read_stream.readQChar()
        self.assertEqual(res, chr(42))

    def testQCharNull(self):
        '''QDataStream <<>> QChar - null'''
        self.stream.writeQChar(None)

        res = self.read_stream.readQChar()
        self.assertEqual(res, '\x00')

    def testQByteArrayValid(self):
        '''QDataStream <<>> QByteArray - valid'''
        self.stream << QByteArray(bytes("hello", "UTF-8"))

        res = QByteArray()

        self.read_stream >> res
        self.assertEqual(res, QByteArray(bytes("hello", "UTF-8")))

    def testQByteArrayEmpty(self):
        '''QDataStream <<>> QByteArray - empty'''
        self.stream << QByteArray(bytes("", "UTF-8"))

        res = QByteArray()

        self.read_stream >> res
        self.assertEqual(res, QByteArray(bytes("", "UTF-8")))
        self.assertTrue(res.isEmpty())
        self.assertFalse(res.isNull())

    def testQByteArrayNull(self):
        '''QDataStream <<>> QByteArray - null'''
        self.stream << QByteArray()

        res = QByteArray()

        self.read_stream >> res
        self.assertEqual(res, QByteArray())
        self.assertTrue(res.isEmpty())
        self.assertTrue(res.isNull())

    def testQStringValid(self):
        '''QDataStream <<>> QString - valid'''
        self.stream.writeQString('Ka-boom')

        res = self.read_stream.readQString()
        self.assertEqual(res, 'Ka-boom')

    def testQStringEmpty(self):
        '''QDataStream <<>> QString - empty'''
        self.stream.writeQString('')

        res = self.read_stream.readQString()
        self.assertEqual(res, '')

    def testQStringNull(self):
        '''QDataStream <<>> QString - null'''
        self.stream.writeQString(None)

        res = self.read_stream.readQString()
        self.assertEqual(res, '')

    def testQBitArrayNull(self):
        '''QDataStream <<>> QBitArray - null'''
        self.stream << QBitArray()

        res = QBitArray()

        self.read_stream >> res
        self.assertEqual(res, QBitArray())

    def testQBitArrayValid(self):
        '''QDataStream <<>> QBitArray - valid'''
        self.stream << create_bitarray('01010101')

        res = QBitArray()

        self.read_stream >> res
        self.assertEqual(res, create_bitarray('01010101'))

    def testQDateNull(self):
        '''QDataStream <<>> QDate - null'''
        self.stream << QDate()

        res = QDate()

        self.read_stream >> res
        self.assertEqual(res, QDate())
        self.assertFalse(res.isValid())
        self.assertTrue(res.isNull())

    def testQDateValid(self):
        '''QDataStream <<>> QDate - valid'''
        self.stream << QDate(2012, 12, 21)

        res = QDate()

        self.read_stream >> res
        self.assertEqual(res, QDate(2012, 12, 21))
        self.assertTrue(res.isValid())
        self.assertFalse(res.isNull())


    def testQTimeNull(self):
        '''QDataStream <<>> QTime - null'''
        self.stream << QTime()

        res = QTime()

        self.read_stream >> res
        self.assertEqual(res, QTime())
        self.assertFalse(res.isValid())
        self.assertTrue(res.isNull())

    def testQTimeValid(self):
        '''QDataStream <<>> QTime - valid'''
        self.stream << QTime(12, 12, 21)

        res = QTime()

        self.read_stream >> res
        self.assertEqual(res, QTime(12, 12, 21))
        self.assertTrue(res.isValid())
        self.assertFalse(res.isNull())

    def testQDateTimeNull(self):
        '''QDataStream <<>> QDateTime - null'''

        self.stream << QDateTime()

        res = QDateTime()

        self.read_stream >> res
        self.assertEqual(res, QDateTime())
        self.assertFalse(res.isValid())
        self.assertTrue(res.isNull())

    def testQDateTimeValid(self):
        '''QDataStream <<>> QDateTime - valid'''
        time = QTime(23, 23, 23)
        date = QDate(2009, 1, 1)

        self.stream << QDateTime(date, time)

        res = QDateTime()

        self.read_stream >> res
        self.assertEqual(res, QDateTime(date, time))
        self.assertTrue(res.isValid())
        self.assertFalse(res.isNull())


class QDataStreamShiftBitArray(unittest.TestCase):

    def _check_bitarray(self, data_set):
        '''Check the >> operator for the given data set'''

        for data, expectedStatus, expectedString in data_set:
            stream = QDataStream(data, QIODevice.ReadOnly)
            string = QBitArray()
            stream >> string

            self.assertEqual(stream.status(), expectedStatus)
            self.assertEqual(string.size(), expectedString.size())
            self.assertEqual(string, expectedString)

    def testOk(self):
        '''QDataStream with valid QBitArray'''
        test_set = [QBitArray()]
        for i in range(1, 7):
            test_set.append(create_bitarray(i * '1'))
        for s in ['0111111', '0000000', '1001110']:
            test_set.append(create_bitarray(s))

        data = []
        for expected in test_set:
            data.append((serialize_bitarray(expected), QDataStream.Ok, expected))
        self._check_bitarray(data)

    def testPastEnd(self):
        '''QDataStream >> QBitArray reading past the end of the data'''
        serialized = serialize_bitarray(create_bitarray('1001110'))
        serialized.resize(serialized.size() - 2)
        self._check_bitarray([(serialized, QDataStream.ReadPastEnd, QBitArray())])


class QDataStreamRawData(unittest.TestCase):
    def testRawData(self):
        data = QDataStream()
        self.assertEqual(data.readRawData(4), None)

        ba = QByteArray()
        data = QDataStream(ba, QIODevice.WriteOnly)
        data.writeRawData('AB\x00C')
        self.assertEqual(ba.data(), bytes('AB\x00C', "UTF-8"))

        data = QDataStream(ba)
        self.assertEqual(data.readRawData(4), bytes('AB\x00C', "UTF-8"))

if __name__ == '__main__':
    unittest.main()

