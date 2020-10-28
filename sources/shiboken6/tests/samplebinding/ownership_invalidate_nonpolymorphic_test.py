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

'''The BlackBox class has cases of ownership transference between Python and C++.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import Point, BlackBox

class OwnershipInvalidateNonPolymorphicTest(unittest.TestCase):
    '''The BlackBox class has cases of ownership transference between Python and C++.'''

    def testOwnershipTransference(self):
        '''Ownership transference from Python to C++ and back again.'''
        p1 = Point(10, 20)
        bb = BlackBox()
        p1_ticket = bb.keepPoint(p1)
        self.assertRaises(RuntimeError, p1.x)
        p1_ret = bb.retrievePoint(p1_ticket)
        self.assertEqual(p1_ret, Point(10, 20))

if __name__ == '__main__':
    unittest.main()

