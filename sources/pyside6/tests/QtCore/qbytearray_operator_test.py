#!/usr/bin/python
# -*- coding: utf-8 -*-

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

'''Test cases for QByteArray operators'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QByteArray

class QByteArrayOperatorEqual(unittest.TestCase):
    '''TestCase for operator QByteArray == QByteArray'''

    def testDefault(self):
        #QByteArray() == QByteArray()
        obj1 = QByteArray()
        obj2 = QByteArray()
        self.assertEqual(obj1, obj2)

    def testSimple(self):
        #QByteArray(some_string) == QByteArray(some_string)
        string = bytes('egg snakes', "UTF-8")
        self.assertEqual(QByteArray(string), QByteArray(string))

    def testPyString(self):
        #QByteArray(string) == string
        string = bytes('my test string', "UTF-8")
        self.assertEqual(QByteArray(string), string)

class QByteArrayOperatorAt(unittest.TestCase):
    '''TestCase for operator QByteArray[]'''

    def testInRange(self):
        #QByteArray[x] where x is a valid index
        string = 'abcdefgh'
        obj = QByteArray(bytes(string, "UTF-8"))

        for i in range(len(string)):
            self.assertEqual(obj[i], bytes(string[i], "UTF-8"))

    def testInRangeReverse(self):
        #QByteArray[x] where x is a valid index (reverse order)
        string = 'abcdefgh'
        obj = QByteArray(bytes(string, "UTF-8"))

        for i in range(len(string)-1, 0, -1):
            self.assertEqual(obj[i], bytes(string[i], "UTF-8"))


    def testOutOfRange(self):
        #QByteArray[x] where x is out of index
        string = bytes('1234567', "UTF-8")
        obj = QByteArray(string)
        self.assertRaises(IndexError, lambda :obj[len(string)])

    def testNullStrings(self):
        ba = QByteArray(bytes('\x00', "UTF-8"))
        self.assertEqual(ba.at(0), '\x00')
        self.assertEqual(ba[0], bytes('\x00', "UTF-8"))

class QByteArrayOperatorLen(unittest.TestCase):
    '''Test case for __len__ operator of QByteArray'''

    def testBasic(self):
        '''QByteArray __len__'''
        self.assertEqual(len(QByteArray()), 0)
        self.assertEqual(len(QByteArray(bytes('', "UTF-8"))), 0)
        self.assertEqual(len(QByteArray(bytes(' ', "UTF-8"))), 1)
        self.assertEqual(len(QByteArray(bytes('yabadaba', "UTF-8"))), 8)


class QByteArrayOperatorStr(unittest.TestCase):
    '''Test case for __str__ operator of QByteArray'''

    def testBasic(self):
        '''QByteArray __str__'''
        self.assertEqual(QByteArray().__str__(), str(b''))
        self.assertEqual(QByteArray(bytes('', "UTF-8")).__str__(), str(b''))
        self.assertEqual(QByteArray(bytes('aaa', "UTF-8")).__str__(), str(b'aaa'))


if __name__ == '__main__':
    unittest.main()
