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

'''Test cases for OddBool user's primitive type conversion.'''

import unittest

from sample import OddBoolUser

class DerivedOddBoolUser (OddBoolUser):
    def returnMyselfVirtual(self):
        return OddBoolUser()
    pass

class OddBoolTest(unittest.TestCase):

    def testOddBoolUser(self):
        obuTrue = OddBoolUser()
        obuFalse = OddBoolUser()
        obuTrue.setOddBool(True)
        self.assertEqual(obuFalse.oddBool(), False)
        self.assertEqual(obuTrue.oddBool(), True)
        self.assertEqual(obuTrue.callInvertedOddBool(), False)

        self.assertEqual(obuTrue.oddBool() == True, True)
        self.assertEqual(False == obuFalse.oddBool(), True)
        self.assertEqual(obuTrue.oddBool() == obuFalse.oddBool(), False)

        self.assertEqual(obuFalse.oddBool() != True, True)
        self.assertEqual(True != obuFalse.oddBool(), True)
        self.assertEqual(obuTrue.oddBool() != obuFalse.oddBool(), True)

    def testVirtuals(self):
        dobu = DerivedOddBoolUser()
        self.assertEqual(dobu.invertedOddBool(), True)

    def testImplicitConversionWithUsersPrimitiveType(self):
        obu = OddBoolUser(True)
        self.assertTrue(obu.oddBool())
        obu = OddBoolUser(False)
        self.assertFalse(obu.oddBool())
        cpx = complex(1.0, 0.0)
        obu = OddBoolUser(cpx)
        self.assertTrue(obu.oddBool())
        cpx = complex(0.0, 0.0)
        obu = OddBoolUser(cpx)
        self.assertFalse(obu.oddBool())

if __name__ == '__main__':
    unittest.main()
