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

'''Test cases for a class with a private destructor.'''

import gc
import sys
import unittest

import shiboken2 as shiboken
from sample import PrivateDtor


class PrivateDtorTest(unittest.TestCase):
    '''Test case for PrivateDtor class'''

    def testPrivateDtorInstanciation(self):
        '''Test if instanciation of class with a private destructor raises an exception.'''
        self.assertRaises(TypeError, PrivateDtor)

    def testPrivateDtorInheritance(self):
        '''Test if inheriting from PrivateDtor raises an exception.'''
        def inherit():
            class Foo(PrivateDtor):
                pass
        self.assertRaises(TypeError, inherit)

    def testPrivateDtorInstanceMethod(self):
        '''Test if PrivateDtor.instance() method return the proper singleton.'''
        pd1 = PrivateDtor.instance()
        calls = pd1.instanceCalls()
        self.assertEqual(type(pd1), PrivateDtor)
        pd2 = PrivateDtor.instance()
        self.assertEqual(pd2, pd1)
        self.assertEqual(pd2.instanceCalls(), calls + 1)

    def testPrivateDtorRefCounting(self):
        '''Test refcounting of the singleton returned by PrivateDtor.instance().'''
        pd1 = PrivateDtor.instance()
        calls = pd1.instanceCalls()
        refcnt = sys.getrefcount(pd1)
        pd2 = PrivateDtor.instance()
        self.assertEqual(pd2.instanceCalls(), calls + 1)
        self.assertEqual(sys.getrefcount(pd2), sys.getrefcount(pd1))
        self.assertEqual(sys.getrefcount(pd2), refcnt + 1)
        del pd1
        self.assertEqual(sys.getrefcount(pd2), refcnt)
        del pd2
        gc.collect()
        pd3 = PrivateDtor.instance()
        self.assertEqual(type(pd3), PrivateDtor)
        self.assertEqual(pd3.instanceCalls(), calls + 2)
        self.assertEqual(sys.getrefcount(pd3), refcnt)

    def testClassDecref(self):
        # Bug was that class PyTypeObject wasn't decrefed when instance
        # was invalidated

        before = sys.getrefcount(PrivateDtor)

        for i in range(1000):
            obj = PrivateDtor.instance()
            shiboken.invalidate(obj)

        after = sys.getrefcount(PrivateDtor)

        self.assertLess(abs(before - after), 5)

if __name__ == '__main__':
    unittest.main()

