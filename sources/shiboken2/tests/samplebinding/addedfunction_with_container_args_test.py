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

'''Test cases for added functions with nested and multi-argument container types.'''

import unittest
from sample import sum2d, sumproduct

class TestAddedFunctionsWithContainerArgs(unittest.TestCase):
    '''Tests added functions with nested and multi-argument container types.'''

    def testNestedContainerType(self):
        '''Test added function with single-argument containers.'''
        values = [[1,2],[3,4,5],[6]]
        self.assertEqual(sum2d(values), 21)

    def testMultiArgContainerType(self):
        '''Test added function with a two-argument container.'''
        values = [(1,2),(3,4),(5,6)]
        self.assertEqual(sumproduct(values), 44)

if __name__ == '__main__':
    unittest.main()
