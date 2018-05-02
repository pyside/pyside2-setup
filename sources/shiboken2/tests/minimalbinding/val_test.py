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

import unittest
from minimal import Val


class ExtVal(Val):
    def __init__(self, valId):
        Val.__init__(self, valId)

    def passValueType(self, val):
        return ExtVal(val.valId() + 1)

    def passValueTypePointer(self, val):
        val.setValId(val.valId() + 1)
        return val

    def passValueTypeReference(self, val):
        val.setValId(val.valId() + 1)
        return val


class ValTest(unittest.TestCase):

    def testNormalMethod(self):
        valId = 123
        val = Val(valId)
        self.assertEqual(val.valId(), valId)

    def testPassValueType(self):
        val = Val(123)
        val1 = val.passValueType(val)
        self.assertNotEqual(val, val1)
        self.assertEqual(val1.valId(), 123)
        val2 = val.callPassValueType(val)
        self.assertNotEqual(val, val2)
        self.assertEqual(val2.valId(), 123)

    def testPassValueTypePointer(self):
        val = Val(0)
        self.assertEqual(val, val.passValueTypePointer(val))
        self.assertEqual(val, val.callPassValueTypePointer(val))

    def testPassValueTypeReference(self):
        val = Val(0)
        self.assertEqual(val, val.passValueTypeReference(val))
        self.assertEqual(val, val.callPassValueTypeReference(val))

    def testPassAndReceiveEnumValue(self):
        val = Val(0)
        self.assertEqual(val.oneOrTheOtherEnumValue(Val.One), Val.Other)
        self.assertEqual(val.oneOrTheOtherEnumValue(Val.Other), Val.One)

    def testPassValueTypeFromExtendedClass(self):
        val = ExtVal(0)
        val1 = val.passValueType(val)
        self.assertNotEqual(val, val1)
        self.assertEqual(val1.valId(), val.valId() + 1)
        val2 = val.callPassValueType(val)
        self.assertNotEqual(val, val2)
        self.assertEqual(val2.valId(), val.valId() + 1)

    def testPassValueTypePointerFromExtendedClass(self):
        val = ExtVal(0)
        self.assertEqual(val.valId(), 0)
        sameVal = val.passValueTypePointer(val)
        self.assertEqual(val, sameVal)
        self.assertEqual(sameVal.valId(), 1)
        sameVal = val.callPassValueTypePointer(val)
        self.assertEqual(val, sameVal)
        self.assertEqual(sameVal.valId(), 2)

    def testPassValueTypeReferenceFromExtendedClass(self):
        val = ExtVal(0)
        self.assertEqual(val.valId(), 0)
        sameVal = val.passValueTypeReference(val)
        self.assertEqual(val, sameVal)
        self.assertEqual(sameVal.valId(), 1)
        sameVal = val.callPassValueTypeReference(val)
        self.assertEqual(val, sameVal)
        self.assertEqual(sameVal.valId(), 2)


if __name__ == '__main__':
    unittest.main()

