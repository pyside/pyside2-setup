# -*- coding:utf-8 -*-
# !/usr/bin/python

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

'''Unit tests for QByteArray'''

import ctypes
import os
import pickle
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)


from PySide2.QtCore import QByteArray, QSettings, QObject, QDataStream, QIODevice

class QByteArrayTestToNumber(unittest.TestCase):
    def testToNumberInt(self):
        obj = QByteArray(bytes('37', "UTF8"))
        self.assertEqual((37, True), obj.toInt())

    def testToNumberUShort(self):
        obj = QByteArray(bytes('37', "UTF8"))
        self.assertEqual((37, True), obj.toUShort())

    def testToNumberFloat(self):
        obj = QByteArray(bytes('37.109', "UTF8"))
        self.assertEqual((ctypes.c_float(37.109).value, True),
                         obj.toFloat())

    def testToNumberDouble(self):
        obj = QByteArray(bytes('37.109', "UTF8"))
        self.assertEqual((ctypes.c_double(37.109).value, True),
                         obj.toDouble())

    def testSetNum(self):
        b = QByteArray()
        b.setNum(int(-124124))
        self.assertEqual(b, "-124124")
        b = QByteArray()
        b.setNum(-124124)
        self.assertEqual(b, "-124124")
        b = QByteArray()
        b.setNum(-0.5)
        self.assertEqual(b, "-0.5")

    def testNumber(self):
        b = QByteArray.number(int(-124124))
        self.assertEqual(b, "-124124")
        b = QByteArray.number(-124124)
        self.assertEqual(b, "-124124")
        b = QByteArray.number(-0.5)
        self.assertEqual(b, "-0.5")

    def testAppend(self):
        b = QByteArray()
        b.append(bytes("A", "UTF8"))
        self.assertEqual(b.size(), 1)
        b.append(bytes("AB", "UTF8"))
        self.assertEqual(b.size(), 3)


class QByteArraySplit(unittest.TestCase):
    '''Test case for QByteArray.split'''

    def testPathSeparator(self):
        #QByteArray.split('/')
        obj = QByteArray(bytes(unittest.__file__, "UTF8"))
        self.assertEqual(obj.split('/'), unittest.__file__.split('/'))

class QByteArrayData(unittest.TestCase):

    '''Test case for QByteArray.data'''

    def testData(self):
        url = QByteArray(bytes("http://pyside.org", "UTF8"))
        self.assertEqual(url.data(), bytes("http://pyside.org", "UTF8"))

    def testDataWithZeros(self):
        s1 = bytes("123\000321", "UTF8")
        ba = QByteArray(s1)
        s2 = ba.data()
        self.assertEqual(s1, s2)
        self.assertEqual(s1, ba)

class QByteArrayOperatorAtSetter(unittest.TestCase):
    '''Test case for operator QByteArray[] - __setitem__'''

    def testSetterString(self):
        '''QByteArray[x] = pythonstring'''
        obj = QByteArray(bytes('123456', "UTF8"))
        obj[1] = bytes('0', "UTF8")
        self.assertEqual(obj, QByteArray(bytes('103456', "UTF8")))

class QByteArrayOnQDataStream(unittest.TestCase):
    '''
    Bug PYSIDE-232
    '''
    def testIt(self):
        a = QByteArray()
        b = QDataStream(a, QIODevice.WriteOnly)
        b.writeUInt16(5000)
        # The __repr__ not suppose to crash anymore
        self.assertNotEqual(repr(b), None)

class TestBug664(unittest.TestCase):
    '''
    QByteArray.data() should return correct data
    '''
    def testIt(self):
        a = QByteArray(bytes('hi 猫', "UTF-8"))
        self.assertEqual(repr(a), "PySide2.QtCore.QByteArray(b'hi \\xe7\\x8c\\xab')")


class QByteArrayOnQVariant(unittest.TestCase):
    def testQByteArrayOnQVariant(self):
        a = QSettings().value("some_prop", QByteArray())
        self.assertEqual(type(a), QByteArray)

class TestBug567(unittest.TestCase):
    '''
    QByteArray should support slices
    '''
    def testIt(self):
        ba = QByteArray(bytes('1234567890', "UTF8"))
        self.assertEqual(ba[2:4], '34')
        self.assertEqual(ba[:4], '1234')
        self.assertEqual(ba[4:], '567890')
        self.assertEqual(len(ba[4:1]), 0)
        self.assertEqual(ba[::-1], '0987654321')
        self.assertEqual(ba[::2], '13579')
        self.assertEqual(ba[::-2], '08642')
        self.assertEqual(ba[2:8:3], '36')


class TestPickler(unittest.TestCase):
    def testIt(self):
        ba = QByteArray(bytes("321\x00123", "UTF8"))
        output = pickle.dumps(str(ba))
        ba2 = pickle.loads(output)
        self.assertEqual(str(ba), str(ba2))

class QByteArrayBug720(unittest.TestCase):
    def testIt(self):
        ba = QByteArray(bytes("32\"1\x00123", "UTF8"))
        self.assertEqual(str(ba), str(bytes("32\"1\x00123", "UTF-8")))
        self.assertEqual(repr(ba), "PySide2.QtCore.QByteArray(b'32\"1\\x00123')")

class QByteArrayImplicitConvert(unittest.TestCase):
    def testString(self):
        # No implicit conversions from QByteArray to python string
        ba = QByteArray(bytes("object name", "UTF8"))
        obj = QObject()
        self.assertRaises(TypeError, obj.setObjectName, ba)


class QByteArraySliceAssignment(unittest.TestCase):
    def testIndexAssignment(self):
        a = QByteArray(bytes('abc', "UTF8"))
        a[0] = bytes('x', "UTF8")
        self.assertEqual(a[0], bytes('x', "UTF8"))

        def test_1():
            a[0] = bytes('xy', "UTF8")
        self.assertRaises(ValueError, test_1)

    def testSliceAssignmentBytes(self):
        b = QByteArray(bytes('0123456789', "UTF8"))
        b[2:8] = bytes('abcdef', "UTF8")
        self.assertEqual(b[2:8], bytes('abcdef', "UTF8"))
        # Delete behavior
        b[2:8] = None
        self.assertEqual(b, bytes('0189', "UTF8"))

        # number of slots and number of values doesn't match
        def test_2():
            b[2:8:2] = bytes('', "UTF8")
        self.assertRaises(ValueError, test_2)
        b = QByteArray(bytes('0123456789', "UTF8"))
        # reverse slice
        b[5:2:-1] = bytes('ABC', "UTF8")
        self.assertEqual(b, bytes('012CBA6789', "UTF8"))
        # step is not 1
        b[2:9:3] = bytes('XYZ', "UTF8")
        self.assertEqual(b, bytes('01XCBY67Z9', "UTF8"))
        b = QByteArray(bytes('0123456789', "UTF8"))
        b[9:2:-3] = bytes('XYZ', "UTF8")
        self.assertEqual(b, bytes('012Z45Y78X', "UTF8"))

    def testSliceAssignmentQByteArray(self):
        b = QByteArray(bytes('0123456789', "UTF8"))
        b[2:8] = QByteArray(bytes('abcdef', "UTF8"))
        self.assertEqual(b[2:8], bytes('abcdef', "UTF8"))
        # shrink
        b[2:8] = QByteArray(bytes('aaa', "UTF8"))
        self.assertEqual(b, bytes('01aaa89', "UTF8"))
        # expanse
        b[2:5] = QByteArray(bytes('uvwxyz', "UTF8"))
        self.assertEqual(b, bytes('01uvwxyz89', "UTF8"))
        # Delete behavior
        b[2:8] = QByteArray()
        self.assertEqual(b, bytes('0189', "UTF8"))

        b = QByteArray(bytes('0123456789', "UTF8"))
        # reverse assginment
        b[5:2:-1] = QByteArray(bytes('ABC', "UTF8"))
        self.assertEqual(b, bytes('012CBA6789', "UTF8"))
        # step is not 1
        b[2:9:3] = QByteArray(bytes('XYZ', "UTF8"))
        self.assertEqual(b, bytes('01XCBY67Z9', "UTF8"))
        b = QByteArray(bytes('0123456789', "UTF8"))
        b[9:2:-3] = QByteArray(bytes('XYZ', "UTF8"))
        self.assertEqual(b, bytes('012Z45Y78X', "UTF8"))

    def testSliceAssignmentByteArray(self):
        b = QByteArray(bytes('0123456789', "UTF8"))
        # replace
        b[2:8] = bytearray(bytes('abcdef', "UTF8"))
        self.assertEqual(b[2:8], bytes('abcdef', "UTF8"))
        # shrink
        b[2:8] = bytearray(bytes('aaa', "UTF8"))
        self.assertEqual(b, bytes('01aaa89', "UTF8"))
        # expanse
        b[2:5] = bytearray(bytes('uvwxyz', "UTF8"))
        self.assertEqual(b, bytes('01uvwxyz89', "UTF8"))
        # Delete behavior
        b[2:8] = bytearray(bytes('', "UTF8"))
        self.assertEqual(b, bytes('0189', "UTF8"))

        b = QByteArray(bytes('0123456789', "UTF8"))
        # reverse assginment
        b[5:2:-1] = bytearray(bytes('ABC', "UTF8"))
        self.assertEqual(b, bytes('012CBA6789', "UTF8"))
        # step is not 1
        b[2:9:3] = bytearray(bytes('XYZ', "UTF8"))
        self.assertEqual(b, bytes('01XCBY67Z9', "UTF8"))
        b = QByteArray(bytes('0123456789', "UTF8"))
        b[9:2:-3] = bytearray(bytes('XYZ', "UTF8"))
        self.assertEqual(b, bytes('012Z45Y78X', "UTF8"))

    def testBufferProtocol(self):
        orig_bytes = bytes('0123456789', "UTF8")
        byte_array = QByteArray(orig_bytes)
        actual_bytes = bytes(byte_array)
        self.assertEqual(orig_bytes, actual_bytes)


if __name__ == '__main__':
    unittest.main()
