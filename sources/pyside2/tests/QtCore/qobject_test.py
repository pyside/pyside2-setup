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

'''Test cases for QObject methods'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

import py3kcompat as py3k

from PySide2.QtCore import QObject, Signal, Qt

class Obj(QObject):
    signal = Signal()
    def empty(self):
        pass

class ObjectNameCase(unittest.TestCase):
    '''Tests related to QObject object name'''

    def testSimple(self):
        #QObject.objectName(string)
        name = 'object1'
        obj = QObject()
        obj.setObjectName(name)

        self.assertEqual(name, obj.objectName())

    def testEmpty(self):
        #QObject.objectName('')
        name = ''
        obj = QObject()
        obj.setObjectName(name)

        self.assertEqual(name, obj.objectName())

    def testDefault(self):
        #QObject.objectName() default
        obj = QObject()
        self.assertEqual('', obj.objectName())

    def testUnicode(self):
        name = py3k.unicode_('não')
        #FIXME Strange error on upstream when using equal(name, obj)
        obj = QObject()
        obj.setObjectName(name)
        self.assertEqual(obj.objectName(), name)

    def testUniqueConnection(self):
        obj = Obj()
        # On first connect, UniqueConnection returns True, and on the second
        # it must return False, and not a RuntimeError (PYSIDE-34)
        self.assertTrue(obj.signal.connect(obj.empty, Qt.UniqueConnection))
        self.assertFalse(obj.signal.connect(obj.empty, Qt.UniqueConnection))

if __name__ == '__main__':
    unittest.main()
