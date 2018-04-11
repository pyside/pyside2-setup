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

'''Test cases for deep copy of objects'''

import copy
import unittest

try:
    import cPickle as pickle
except ImportError:
    import pickle


from sample import Point


class SimpleCopy(unittest.TestCase):
    '''Simple copy of objects'''

    def testCopy(self):
        point = Point(0.1, 2.4)
        new_point = copy.copy(point)

        self.assertTrue(point is not new_point)
        self.assertEqual(point, new_point)


class DeepCopy(unittest.TestCase):
    '''Deep copy with shiboken objects'''

    def testDeepCopy(self):
        '''Deep copy of value types'''
        point = Point(3.1, 4.2)
        new_point = copy.deepcopy([point])[0]

        self.assertTrue(point is not new_point)
        self.assertEqual(point, new_point)


class PicklingTest(unittest.TestCase):
    '''Support pickling'''

    def testSimple(self):
        '''Simple pickling and unpickling'''

        point = Point(10.2, 43.5)

        data = pickle.dumps(point)
        new_point = pickle.loads(data)

        self.assertEqual(point, new_point)
        self.assertTrue(point is not new_point)


if __name__ == '__main__':
    unittest.main()

