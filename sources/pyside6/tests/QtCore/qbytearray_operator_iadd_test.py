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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QByteArray
from helper.docmodifier import DocModifier


class BaseQByteArrayOperatorIAdd(object):
    '''Base class for QByteArray += operator tests.

    Implementing classes should inherit from unittest.TestCase and implement
    setUp, setting self.obj and self.orig_obj to the target QByteArray and original
    one, respectively'''

    __metaclass__ = DocModifier

    def testSingleString(self):
        '''QByteArray += bytes of size 1'''
        s = bytes('0', "UTF-8")
        self.obj += s
        self.assertEqual(self.obj, self.orig_obj + s)
        self.assertEqual(self.obj.size(), self.orig_obj.size() + len(s))

    def testString(self):
        '''QByteArray += bytes of size > 1'''
        s = bytearray(bytes('dummy', "UTF-8"))
        self.obj += s # XXx iadd support abytearray
        self.assertEqual(self.obj, self.orig_obj + s)
        self.assertEqual(self.obj.size(), self.orig_obj.size() + len(s))

    def testQByteArray(self):
        '''QByteArray += QByteArray'''
        s = QByteArray(bytes('array', "UTF-8"))
        self.obj += s
        self.assertEqual(self.obj, self.orig_obj + s)


class NullQByteArrayOperatorIAdd(unittest.TestCase, BaseQByteArrayOperatorIAdd):
    '''Test case for operator QByteArray += on null QByteArrays'''

    doc_prefix = 'Null object'
    doc_filter = lambda x: x.startswith('test')

    def setUp(self):
        self.obj = QByteArray()
        self.orig_obj = QByteArray()


class ValidQByteArrayOperatorIAdd(unittest.TestCase, BaseQByteArrayOperatorIAdd):
    '''Test case for operator QByteArray += on valid QByteArrays'''

    doc_prefix = 'Valid object'
    doc_filter = lambda x: x.startswith('test')

    def setUp(self):
        self.obj = QByteArray(bytes('some byte array', "UTF-8"))
        self.orig_obj = QByteArray(bytes('some byte array', "UTF-8"))

if __name__ == '__main__':
    unittest.main()
