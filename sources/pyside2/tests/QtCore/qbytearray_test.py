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

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

import py3kcompat as py3k

from PySide2.QtCore import QByteArray, QSettings, QObject, QDataStream, QIODevice

class QByteArrayTestToNumber(unittest.TestCase):
    def testToNumberInt(self):
        obj = QByteArray(py3k.b('37'))
        self.assertEqual((37, True), obj.toInt())

    def testToNumberUShort(self):
        obj = QByteArray(py3k.b('37'))
        self.assertEqual((37, True), obj.toUShort())

    def testToNumberFloat(self):
        obj = QByteArray(py3k.b('37.109'))
        self.assertEqual((ctypes.c_float(37.109).value, True),
                         obj.toFloat())

    def testToNumberDouble(self):
        obj = QByteArray(py3k.b('37.109'))
        self.assertEqual((ctypes.c_double(37.109).value, True),
                         obj.toDouble())

    def testSetNum(self):
        b = QByteArray()
        b.setNum(py3k.long(-124124))
        self.assertEqual(b, "-124124")
        b = QByteArray()
        b.setNum(-124124)
        self.assertEqual(b, "-124124")
        b = QByteArray()
        b.setNum(-0.5)
        self.assertEqual(b, "-0.5")

    def testAppend(self):
        b = QByteArray()
        b.append(py3k.b("A"))
        self.assertEqual(b.size(), 1)
        b.append(py3k.b("AB"))
        self.assertEqual(b.size(), 3)


class QByteArraySplit(unittest.TestCase):
    '''Test case for QByteArray.split'''

    def testPathSeparator(self):
        #QByteArray.split('/')
        obj = QByteArray(py3k.b(unittest.__file__))
        self.assertEqual(obj.split('/'), unittest.__file__.split('/'))

class QByteArrayData(unittest.TestCase):

    '''Test case for QByteArray.data'''

    def testData(self):
        url = QByteArray(py3k.b("http://pyside.org"))
        self.assertEqual(url.data(), py3k.b("http://pyside.org"))

    def testDataWithZeros(self):
        s1 = py3k.b("123\000321")
        ba = QByteArray(s1)
        s2 = ba.data()
        self.assertEqual(py3k.b(s1), s2)
        self.assertEqual(s1, ba)

class QByteArrayOperatorAtSetter(unittest.TestCase):
    '''Test case for operator QByteArray[] - __setitem__'''

    def testSetterString(self):
        '''QByteArray[x] = pythonstring'''
        obj = QByteArray(py3k.b('123456'))
        obj[1] = py3k.b('0')
        self.assertEqual(obj, QByteArray(py3k.b('103456')))

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
        a = QByteArray(py3k.unicode_('hi 猫').encode('utf-8'))
        if py3k.IS_PY3K:
            self.assertEqual(repr(a), "PySide2.QtCore.QByteArray(b'hi \\xe7\\x8c\\xab')")
        else:
            self.assertEqual(repr(a), "PySide2.QtCore.QByteArray('hi \\xe7\\x8c\\xab')")

class QByteArrayOnQVariant(unittest.TestCase):
    def testQByteArrayOnQVariant(self):
        a = QSettings().value("some_prop", QByteArray())
        self.assertEqual(type(a), QByteArray)

class TestBug567(unittest.TestCase):
    '''
    QByteArray should support slices
    '''
    def testIt(self):
        ba = QByteArray(py3k.b('1234567890'))
        self.assertEqual(ba[2:4], '34')
        self.assertEqual(ba[:4], '1234')
        self.assertEqual(ba[4:], '567890')
        self.assertEqual(len(ba[4:1]), 0)
        self.assertEqual(ba[::-1], '0987654321')
        self.assertEqual(ba[::2], '13579')
        self.assertEqual(ba[::-2], '08642')
        self.assertEqual(ba[2:8:3], '36')

class QByteArrayBug514(unittest.TestCase):
    def testIt(self):
        data = py3k.b("foobar")
        a = QByteArray.fromRawData(data)
        self.assertEqual(type(a), QByteArray)
        self.assertEqual(a.data(), data)

class TestPickler(unittest.TestCase):
    def testIt(self):
        ba = QByteArray(py3k.b("321\x00123"))
        output = pickle.dumps(str(ba))
        ba2 = pickle.loads(output)
        self.assertEqual(str(ba), str(ba2))

class QByteArrayBug720(unittest.TestCase):
    def testIt(self):
        ba = QByteArray(py3k.b("32\"1\x00123"))
        self.assertEqual(str(ba), str(py3k.b("32\"1\x00123")))
        if py3k.IS_PY3K:
            self.assertEqual(repr(ba), "PySide2.QtCore.QByteArray(b'32\"1\\x00123')")
        else:
            self.assertEqual(repr(ba), "PySide2.QtCore.QByteArray('32\"1\\x00123')")

class QByteArrayImplicitConvert(unittest.TestCase):
    def testString(self):
        # No implicit conversions from QByteArray to python string
        ba = QByteArray(py3k.b("object name"))
        obj = QObject()
        self.assertRaises(TypeError, obj.setObjectName, ba)


class QByteArraySliceAssignment(unittest.TestCase):
    def testIndexAssignment(self):
        a = QByteArray(py3k.b('abc'))
        a[0] = py3k.b('x')
        self.assertEqual(a[0], py3k.b('x'))

        def test_1():
            a[0] = py3k.b('xy')
        self.assertRaises(ValueError, test_1)

    def testSliceAssignmentBytes(self):
        b = QByteArray(py3k.b('0123456789'))
        b[2:8] = py3k.b('abcdef')
        self.assertEqual(b[2:8], py3k.b('abcdef'))
        # Delete behavior
        b[2:8] = None
        self.assertEqual(b, py3k.b('0189'))

        # number of slots and number of values doesn't match
        def test_2():
            b[2:8:2] = py3k.b('')
        self.assertRaises(ValueError, test_2)
        b = QByteArray(py3k.b('0123456789'))
        # reverse slice
        b[5:2:-1] = py3k.b('ABC')
        self.assertEqual(b, py3k.b('012CBA6789'))
        # step is not 1
        b[2:9:3] = py3k.b('XYZ')
        self.assertEqual(b, py3k.b('01XCBY67Z9'))
        b = QByteArray(py3k.b('0123456789'))
        b[9:2:-3] = py3k.b('XYZ')
        self.assertEqual(b, py3k.b('012Z45Y78X'))

    def testSliceAssignmentQByteArray(self):
        b = QByteArray(py3k.b('0123456789'))
        b[2:8] = QByteArray(py3k.b('abcdef'))
        self.assertEqual(b[2:8], py3k.b('abcdef'))
        # shrink
        b[2:8] = QByteArray(py3k.b('aaa'))
        self.assertEqual(b, py3k.b('01aaa89'))
        # expanse
        b[2:5] = QByteArray(py3k.b('uvwxyz'))
        self.assertEqual(b, py3k.b('01uvwxyz89'))
        # Delete behavior
        b[2:8] = QByteArray()
        self.assertEqual(b, py3k.b('0189'))

        b = QByteArray(py3k.b('0123456789'))
        # reverse assginment
        b[5:2:-1] = QByteArray(py3k.b('ABC'))
        self.assertEqual(b, py3k.b('012CBA6789'))
        # step is not 1
        b[2:9:3] = QByteArray(py3k.b('XYZ'))
        self.assertEqual(b, py3k.b('01XCBY67Z9'))
        b = QByteArray(py3k.b('0123456789'))
        b[9:2:-3] = QByteArray(py3k.b('XYZ'))
        self.assertEqual(b, py3k.b('012Z45Y78X'))

    def testSliceAssignmentByteArray(self):
        b = QByteArray(py3k.b('0123456789'))
        # replace
        b[2:8] = bytearray(py3k.b('abcdef'))
        self.assertEqual(b[2:8], py3k.b('abcdef'))
        # shrink
        b[2:8] = bytearray(py3k.b('aaa'))
        self.assertEqual(b, py3k.b('01aaa89'))
        # expanse
        b[2:5] = bytearray(py3k.b('uvwxyz'))
        self.assertEqual(b, py3k.b('01uvwxyz89'))
        # Delete behavior
        b[2:8] = bytearray(py3k.b(''))
        self.assertEqual(b, py3k.b('0189'))

        b = QByteArray(py3k.b('0123456789'))
        # reverse assginment
        b[5:2:-1] = bytearray(py3k.b('ABC'))
        self.assertEqual(b, py3k.b('012CBA6789'))
        # step is not 1
        b[2:9:3] = bytearray(py3k.b('XYZ'))
        self.assertEqual(b, py3k.b('01XCBY67Z9'))
        b = QByteArray(py3k.b('0123456789'))
        b[9:2:-3] = bytearray(py3k.b('XYZ'))
        self.assertEqual(b, py3k.b('012Z45Y78X'))

    def testBufferProtocol(self):
        orig_bytes = py3k.b('0123456789')
        byte_array = QByteArray(orig_bytes)
        actual_bytes = bytes(byte_array)
        self.assertEqual(orig_bytes, actual_bytes)


if __name__ == '__main__':
    unittest.main()
