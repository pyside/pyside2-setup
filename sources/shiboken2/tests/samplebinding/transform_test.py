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

'''Test cases for argument modification with more than nine arguments.'''

import unittest

from sample import Point, applyHomogeneousTransform

class TransformTest(unittest.TestCase):
    '''Test cases for modifying a function with > 9 arguments.'''

    def testTransform_ValidMatrix(self):
        '''Transform applies successfully.'''
        p = Point(3, 4)
        r = applyHomogeneousTransform(p, 0, 1, 0, -1, 0, 0, 0, 0, 1)
        self.assertTrue(type(r) is Point)
        self.assertEqual(r.x(), 4)
        self.assertEqual(r.y(), -3)

    def testTransform_InvalidMatrix(self):
        '''Transform does not apply successfully.'''
        p = Point(3, 4)
        r = applyHomogeneousTransform(p, 1, 0, 0, 0, 1, 0, 0, 0, 0)
        self.assertTrue(r is None)

if __name__ == '__main__':
    unittest.main()
