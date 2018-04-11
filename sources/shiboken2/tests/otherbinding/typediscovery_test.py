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

'''Test cases for type discovery'''

import unittest

from sample import Abstract, Base1, Derived, MDerived1, MDerived3, SonOfMDerived1
from other import OtherMultipleDerived

class TypeDiscoveryTest(unittest.TestCase):

    def testPureVirtualsOfImpossibleTypeDiscovery(self):
        a = Derived.triggerImpossibleTypeDiscovery()
        self.assertEqual(type(a), Abstract)
        # call some pure virtual method
        a.pureVirtual()

    def testAnotherImpossibleTypeDiscovery(self):
        a = Derived.triggerAnotherImpossibleTypeDiscovery()
        self.assertEqual(type(a), Derived)

    def testMultipleInheritance(self):
        obj = OtherMultipleDerived.createObject("Base1");
        self.assertEqual(type(obj), Base1)
        obj = OtherMultipleDerived.createObject("MDerived1");
        self.assertEqual(type(obj), MDerived1)
        obj = OtherMultipleDerived.createObject("SonOfMDerived1");
        self.assertEqual(type(obj), SonOfMDerived1)
        obj = OtherMultipleDerived.createObject("MDerived3");
        self.assertEqual(type(obj), MDerived3)
        obj = OtherMultipleDerived.createObject("OtherMultipleDerived");
        self.assertEqual(type(obj), OtherMultipleDerived)

if __name__ == '__main__':
    unittest.main()
