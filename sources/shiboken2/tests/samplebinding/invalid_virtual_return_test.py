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

'''Test case for returning invalid types in a virtual function'''

import unittest
from sample import ObjectModel, ObjectType, ObjectView

import warnings


class MyObject(ObjectType):
    pass


class ListModelWrong(ObjectModel):

    def __init__(self, parent=None):
        ObjectModel.__init__(self, parent)
        self.obj = 0

    def data(self):
        warnings.simplefilter('error')
        # Shouldn't segfault. Must set TypeError
        return self.obj


class ModelWrongReturnTest(unittest.TestCase):

    def testWrongTypeReturn(self):
        model = ListModelWrong()
        view = ObjectView(model)
        self.assertRaises(RuntimeWarning, view.getRawModelData) # calls model.data()


if __name__ == '__main__':
    unittest.main()
