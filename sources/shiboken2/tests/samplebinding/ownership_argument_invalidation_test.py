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

'''Wrapper validity tests for arguments.'''

import sys
import unittest

from sample import Polygon, Point

class WrapperValidityOfArgumentsTest(unittest.TestCase):
    '''Wrapper validity tests for arguments.'''

    def testInvalidArgumentToMethod(self):
        '''Call to method using invalidated Python wrapper as argument should raise RuntimeError.'''
        poly = Polygon()
        Polygon.stealOwnershipFromPython(poly)
        self.assertRaises(RuntimeError, Polygon.doublePolygonScale, poly)

    def testInvalidArgumentToConstructor(self):
        '''Call to constructor using invalidated Python wrapper as argument should raise RuntimeError.'''
        pt = Point(1, 2)
        Polygon.stealOwnershipFromPython(pt)
        self.assertRaises(RuntimeError, Polygon, pt)

    def testInvalidArgumentWithImplicitConversion(self):
        '''Call to method using invalidated Python wrapper to be implicitly converted should raise RuntimeError.'''
        pt = Point(1, 2)
        Polygon.stealOwnershipFromPython(pt)
        self.assertRaises(RuntimeError, Polygon.doublePolygonScale, pt)

if __name__ == '__main__':
    unittest.main()

