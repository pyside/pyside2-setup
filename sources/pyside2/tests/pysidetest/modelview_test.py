#!/usr/bin/python

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
import py3kcompat as py3k
from testbinding import TestView
from PySide2.QtCore import QAbstractListModel, QObject, QModelIndex

'''Tests model/view relationship.'''

object_name = 'test object'

class MyObject(QObject):
    pass

class ListModelKeepsReference(QAbstractListModel):
    def __init__(self, parent=None):
        QAbstractListModel.__init__(self, parent)
        self.obj = MyObject()
        self.obj.setObjectName(object_name)

    def rowCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role):
        return self.obj

class ListModelDoesntKeepsReference(QAbstractListModel):
    def rowCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role):
        obj = MyObject()
        obj.setObjectName(object_name)
        return obj

class ListModelThatReturnsString(QAbstractListModel):
    def rowCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role):
        self.obj = 'string'
        return self.obj


class ModelViewTest(unittest.TestCase):

    def testListModelDoesntKeepsReference(self):
        model = ListModelDoesntKeepsReference()
        view = TestView(model)
        obj = view.getData()
        self.assertEqual(type(obj), MyObject)
        self.assertEqual(obj.objectName(), object_name)
        obj.metaObject()

    def testListModelKeepsReference(self):
        model = ListModelKeepsReference()
        view = TestView(model)
        obj = view.getData()
        self.assertEqual(type(obj), MyObject)
        self.assertEqual(obj.objectName(), object_name)

    def testListModelThatReturnsString(self):
        model = ListModelThatReturnsString()
        view = TestView(model)
        obj = view.getData()
        self.assertEqual(type(obj), py3k.unicode)
        self.assertEqual(obj, 'string')

if __name__ == '__main__':
    unittest.main()

