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

'''Test cases for a class with only a private constructor.'''

import gc
import sys
import unittest

from sample import PrivateCtor


class PrivateCtorTest(unittest.TestCase):
    '''Test case for PrivateCtor class'''

    def testPrivateCtorInstanciation(self):
        '''Test if instanciation of class with a private constructor raises an exception.'''
        self.assertRaises(TypeError, PrivateCtor)

    def testPrivateCtorInheritance(self):
        '''Test if inheriting from PrivateCtor raises an exception.'''
        def inherit():
            class Foo(PrivateCtor):
                pass
        self.assertRaises(TypeError, inherit)

    def testPrivateCtorInstanceMethod(self):
        '''Test if PrivateCtor.instance() method return the proper singleton.'''
        pd1 = PrivateCtor.instance()
        calls = pd1.instanceCalls()
        self.assertEqual(type(pd1), PrivateCtor)
        pd2 = PrivateCtor.instance()
        self.assertEqual(pd2, pd1)
        self.assertEqual(pd2.instanceCalls(), calls + 1)

    def testPrivateCtorRefCounting(self):
        '''Test refcounting of the singleton returned by PrivateCtor.instance().'''
        pd1 = PrivateCtor.instance()
        calls = pd1.instanceCalls()
        refcnt = sys.getrefcount(pd1)
        pd2 = PrivateCtor.instance()
        self.assertEqual(pd2.instanceCalls(), calls + 1)
        self.assertEqual(sys.getrefcount(pd2), sys.getrefcount(pd1))
        self.assertEqual(sys.getrefcount(pd2), refcnt + 1)
        del pd1
        self.assertEqual(sys.getrefcount(pd2), refcnt)
        del pd2
        gc.collect()
        pd3 = PrivateCtor.instance()
        self.assertEqual(type(pd3), PrivateCtor)
        self.assertEqual(pd3.instanceCalls(), calls + 2)
        self.assertEqual(sys.getrefcount(pd3), refcnt)

if __name__ == '__main__':
    unittest.main()

