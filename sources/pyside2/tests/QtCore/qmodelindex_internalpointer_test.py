# -*- coding: utf-8 -*-

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

''' Test case for QAbstractListModel.createIndex and QModelIndex.internalPointer'''

import sys
import unittest
from PySide2.QtCore import *

class MyModel (QAbstractListModel):
    pass

class TestQModelIndexInternalPointer(unittest.TestCase):
    ''' Test case for QAbstractListModel.createIndex and QModelIndex.internalPointer'''

    def setUp(self):
        #Acquire resources
        self.model = MyModel()

    def tearDown(self):
        #Release resources
        del self.model

    def testInternalPointer(self):
        #Test QAbstractListModel.createIndex and
        #QModelIndex.internalPointer with regular
        #Python objects
        idx = self.model.createIndex(0, 0, "Hello")
        self.assertEqual("Hello", idx.internalPointer())
        a = [1, 2, 3]
        idx = self.model.createIndex(0, 0, a)
        self.assertEqual(a, idx.internalPointer())

    def testReferenceCounting(self):
        #Test reference counting when retrieving data with
        #QModelIndex.internalPointer
        a = [1, 2, 3]
        a_refcnt = sys.getrefcount(a)
        idx = self.model.createIndex(0, 0, a)
        ptr = idx.internalPointer()
        self.assertEqual(sys.getrefcount(a), a_refcnt + 1)

    def testIndexForDefaultDataArg(self):
        #Test QAbstractListModel.createIndex with a default
        #value for data argument
        idx = self.model.createIndex(0, 0)
        self.assertEqual(None, idx.internalPointer())

if __name__ == '__main__':
    unittest.main()

