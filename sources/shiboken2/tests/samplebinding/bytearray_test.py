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

import unittest
from os.path import isdir
from sample import ByteArray
from py3kcompat import b


class ByteArrayBufferProtocolTest(unittest.TestCase):
    '''Tests ByteArray implementation of Python buffer protocol.'''

    def testByteArrayBufferProtocol(self):
        # Tests ByteArray implementation of Python buffer protocol using the os.path.isdir
        # function which an unicode object or other object implementing the Python buffer protocol.
        isdir(str(ByteArray('/tmp')))


class ByteArrayConcatenationOperatorTest(unittest.TestCase):
    '''Test cases for ByteArray concatenation with '+' operator.'''

    def testConcatByteArrayAndPythonString(self):
        # Test concatenation of a ByteArray with a Python string, in this order.
        ba = ByteArray('foo')
        result = ba + '\x00bar'
        self.assertEqual(type(result), ByteArray)
        self.assertEqual(result, 'foo\x00bar')

    def testConcatPythonStringAndByteArray(self):
        # Test concatenation of a Python string with a ByteArray, in this order.
        concat_python_string_add_qbytearray_worked = True
        ba = ByteArray('foo')
        result = 'bar\x00' + ba
        self.assertEqual(type(result), ByteArray)
        self.assertEqual(result, 'bar\x00foo')


class ByteArrayOperatorEqual(unittest.TestCase):
    '''TestCase for operator ByteArray == ByteArray.'''

    def testDefault(self):
        # ByteArray() == ByteArray()
        obj1 = ByteArray()
        obj2 = ByteArray()
        self.assertEqual(obj1, obj2)

    def testSimple(self):
        # ByteArray(some_string) == ByteArray(some_string)
        string = 'egg snakes'
        self.assertEqual(ByteArray(string), ByteArray(string))

    def testPyString(self):
        # ByteArray(string) == string
        string = 'my test string'
        self.assertEqual(ByteArray(string), string)

    def testQString(self):
        # ByteArray(string) == string
        string = 'another test string'
        self.assertEqual(ByteArray(string), string)


class ByteArrayOperatorAt(unittest.TestCase):
    '''TestCase for operator ByteArray[]'''

    def testInRange(self):
        # ByteArray[x] where x is a valid index.
        string = 'abcdefgh'
        obj = ByteArray(string)
        for i in range(len(string)):
            self.assertEqual(obj[i], b(string[i]))

    def testInRangeReverse(self):
        # ByteArray[x] where x is a valid index (reverse order).
        string = 'abcdefgh'
        obj = ByteArray(string)
        for i in range(len(string)-1, 0, -1):
            self.assertEqual(obj[i], b(string[i]))

    def testOutOfRange(self):
        # ByteArray[x] where x is out of index.
        string = '1234567'
        obj = ByteArray(string)
        self.assertRaises(IndexError, lambda :obj[len(string)])

    def testNullStrings(self):
        ba = ByteArray('\x00')
        self.assertEqual(ba.at(0), '\x00')
        self.assertEqual(ba[0], b('\x00'))


class ByteArrayOperatorLen(unittest.TestCase):
    '''Test case for __len__ operator of ByteArray'''

    def testBasic(self):
        '''ByteArray __len__'''
        self.assertEqual(len(ByteArray()), 0)
        self.assertEqual(len(ByteArray('')), 0)
        self.assertEqual(len(ByteArray(' ')), 1)
        self.assertEqual(len(ByteArray('yabadaba')), 8)


class ByteArrayAndPythonStr(unittest.TestCase):
    '''Test case for __str__ operator of ByteArray'''

    def testStrOperator(self):
        '''ByteArray __str__'''
        self.assertEqual(ByteArray().__str__(), '')
        self.assertEqual(ByteArray('').__str__(), '')
        self.assertEqual(ByteArray('aaa').__str__(), 'aaa')

    def testPythonStrAndNull(self):
        s1 = b('123\000321')
        ba = ByteArray(s1)
        s2 = ba.data()
        self.assertEqual(s1, s2)
        self.assertEqual(type(s1), type(s2))
        self.assertEqual(s1, ba)
        self.assertNotEqual(type(s1), type(ba))


if __name__ == '__main__':
    unittest.main()
