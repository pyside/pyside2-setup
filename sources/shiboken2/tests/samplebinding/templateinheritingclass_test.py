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

import unittest
from sample import Photon

'''This tests classes that inherit from template classes,
simulating a situation found in Qt's phonon module.'''

class TemplateInheritingClassTest(unittest.TestCase):
    def testClassBasics(self):
        self.assertEqual(Photon.ValueIdentity.classType(), Photon.IdentityType)
        self.assertEqual(Photon.ValueDuplicator.classType(), Photon.DuplicatorType)

    def testInstanceBasics(self):
        value = 123
        samer = Photon.ValueIdentity(value)
        self.assertEqual(samer.multiplicator(), 1)
        doubler = Photon.ValueDuplicator(value)
        self.assertEqual(doubler.multiplicator(), 2)
        self.assertEqual(samer.value(), doubler.value())
        self.assertEqual(samer.calculate() * 2, doubler.calculate())

    def testPassToFunctionAsPointer(self):
        obj = Photon.ValueDuplicator(123)
        self.assertEqual(Photon.callCalculateForValueDuplicatorPointer(obj), obj.calculate())

    def testPassToFunctionAsReference(self):
        obj = Photon.ValueDuplicator(321)
        self.assertEqual(Photon.callCalculateForValueDuplicatorReference(obj), obj.calculate())

    def testPassToMethodAsValue(self):
        value1, value2 = 123, 321
        one = Photon.ValueIdentity(value1)
        other = Photon.ValueIdentity(value2)
        self.assertEqual(one.sumValueUsingPointer(other), value1 + value2)

    def testPassToMethodAsReference(self):
        value1, value2 = 123, 321
        one = Photon.ValueDuplicator(value1)
        other = Photon.ValueDuplicator(value2)
        self.assertEqual(one.sumValueUsingReference(other), value1 + value2)

    def testPassPointerThrough(self):
        obj1 = Photon.ValueIdentity(123)
        self.assertEqual(obj1, obj1.passPointerThrough(obj1))
        obj2 = Photon.ValueDuplicator(321)
        self.assertEqual(obj2, obj2.passPointerThrough(obj2))
        self.assertRaises(TypeError, obj1.passPointerThrough, obj2)

if __name__ == '__main__':
    unittest.main()
