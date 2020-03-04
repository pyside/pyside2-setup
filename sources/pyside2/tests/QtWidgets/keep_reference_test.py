#!/usr/bin/env python

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

from sys import getrefcount
from helper.usesqapplication import UsesQApplication
from PySide2.QtCore import *
from PySide2.QtWidgets import QTableView

class TestModel(QAbstractTableModel):
    def __init__(self, parent=None):
        QAbstractTableModel.__init__(self, parent)
    def rowCount(self, parent):
        return 0
    def columnCount(self, parent):
        return 0
    def data(self, index, role):
        return None

class KeepReferenceTest(UsesQApplication):

    def testModelWithoutParent(self):
        view = QTableView()
        model = TestModel()
        view.setModel(model)
        samemodel = view.model()
        self.assertEqual(model, samemodel)

    def testModelWithParent(self):
        view = QTableView()
        model = TestModel(None)
        view.setModel(model)
        samemodel = view.model()
        self.assertEqual(model, samemodel)

    def testReferenceCounting(self):
        '''Tests reference count of model object referred by view objects.'''
        model1 = TestModel()
        refcount1 = getrefcount(model1)
        view1 = QTableView()
        view1.setModel(model1)
        self.assertEqual(getrefcount(view1.model()), refcount1 + 1)

        view2 = QTableView()
        view2.setModel(model1)
        self.assertEqual(getrefcount(view2.model()), refcount1 + 2)

        model2 = TestModel()
        view2.setModel(model2)
        self.assertEqual(getrefcount(view1.model()), refcount1 + 1)

    def testReferenceCountingWhenDeletingReferrer(self):
        '''Tests reference count of model object referred by deceased view object.'''
        model = TestModel()
        refcount1 = getrefcount(model)
        view = QTableView()
        view.setModel(model)
        self.assertEqual(getrefcount(view.model()), refcount1 + 1)

        del view
        self.assertEqual(getrefcount(model), refcount1)

    def testReferreedObjectSurvivalAfterContextEnd(self):
        '''Model object assigned to a view object must survive after getting out of context.'''
        def createModelAndSetToView(view):
            model = TestModel()
            model.setObjectName('created model')
            view.setModel(model)
        view = QTableView()
        createModelAndSetToView(view)
        model = view.model()

if __name__ == '__main__':
    unittest.main()

