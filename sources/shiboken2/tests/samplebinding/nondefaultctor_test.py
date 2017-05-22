#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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

'''Test cases for ...'''

import sys
import unittest

from sample import NonDefaultCtor

class DerivedNonDefaultCtor (NonDefaultCtor):
    def returnMyselfVirtual(self):
        return NonDefaultCtor(self.value()+1)

class AnotherDerivedNonDefaultCtor (NonDefaultCtor):
    def __init__(self, some_string):
        pass

class NonDefaultCtorTest(unittest.TestCase):

    def testNonDefaultCtor(self):
        c = NonDefaultCtor(2)
        # these functions returns NonDefaultCtor by value, so a PyObjecy  is created every time
        self.assertNotEqual(c.returnMyself(), c)
        self.assertEqual(c.returnMyself().value(), 2)
        self.assertNotEqual(c.returnMyself(3), c)
        self.assertEqual(c.returnMyself(3).value(), 2)
        self.assertNotEqual(c.returnMyself(4, c), c)
        self.assertEqual(c.returnMyself(4, c).value(), 2)

    def testVirtuals(self):
        c = DerivedNonDefaultCtor(3)
        # these functions returns NonDefaultCtor by value, so a PyObjecy  is created every time
        self.assertNotEqual(c.returnMyselfVirtual(), c)
        self.assertEqual(c.returnMyselfVirtual().value(), 4)
        self.assertEqual(c.callReturnMyselfVirtual().value(), 4)

    def testCtorOverload(self):
        c = AnotherDerivedNonDefaultCtor("testing")

if __name__ == '__main__':
    unittest.main()

