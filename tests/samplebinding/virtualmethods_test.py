#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# This file is part of the Shiboken Python Bindings Generator project.
#
# Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
#
# Contact: PySide team <contact@pyside.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation. Please
# review the following information to ensure the GNU Lesser General
# Public License version 2.1 requirements will be met:
# http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
# #
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA

'''Test cases for virtual methods.'''

import sys
import unittest

from sample import VirtualMethods, Point

class ExtendedVirtualMethods(VirtualMethods):
    def __init__(self):
        VirtualMethods.__init__(self)
        self.virtual_method0_called = False

    def virtualMethod0(self, pt, val, cpx, b):
        self.virtual_method0_called = True
        return VirtualMethods.virtualMethod0(self, pt, val, cpx, b) * -1.0

class VirtualMethodsTest(unittest.TestCase):
    '''Test case for virtual methods'''

    def testReimplementedVirtualMethod0(self):
        '''Test Python override of a virtual method with various different parameters is correctly called from C++.'''
        vm = VirtualMethods()
        evm = ExtendedVirtualMethods()
        pt = Point(1.1, 2.2)
        val = 4
        cpx = complex(3.3, 4.4)
        b = True
        result0 = vm.callVirtualMethod0(pt, val, cpx, b)
        result1 = evm.callVirtualMethod0(pt, val, cpx, b)
        self.assertEqual(result0 * -1.0, result1)

if __name__ == '__main__':
    unittest.main()

