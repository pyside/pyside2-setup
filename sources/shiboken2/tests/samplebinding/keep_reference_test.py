#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

'''Test case for objects that keep references to other object without owning them (e.g. model/view relationships).'''

import unittest
from sys import getrefcount

from sample import ObjectModel, ObjectView

class TestKeepReference(unittest.TestCase):
    '''Test case for objects that keep references to other object without owning them (e.g. model/view relationships).'''

    def testReferenceCounting(self):
        '''Tests reference count of model-like object referred by view-like objects.'''
        model1 = ObjectModel()
        refcount1 = getrefcount(model1)
        view1 = ObjectView()
        view1.setModel(model1)
        self.assertEqual(getrefcount(view1.model()), refcount1 + 1)

        view2 = ObjectView()
        view2.setModel(model1)
        self.assertEqual(getrefcount(view2.model()), refcount1 + 2)

        model2 = ObjectModel()
        view2.setModel(model2)
        self.assertEqual(getrefcount(view1.model()), refcount1 + 1)

    def testReferenceCountingWhenDeletingReferrer(self):
        '''Tests reference count of model-like object referred by deceased view-like object.'''
        model = ObjectModel()
        refcount1 = getrefcount(model)
        view = ObjectView()
        view.setModel(model)
        self.assertEqual(getrefcount(view.model()), refcount1 + 1)

        del view
        self.assertEqual(getrefcount(model), refcount1)

    def testReferreedObjectSurvivalAfterContextEnd(self):
        '''Model-like object assigned to a view-like object must survive after get out of context.'''
        def createModelAndSetToView(view):
            model = ObjectModel()
            model.setObjectName('created model')
            view.setModel(model)
        view = ObjectView()
        createModelAndSetToView(view)
        model = view.model()

if __name__ == '__main__':
    unittest.main()

