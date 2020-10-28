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

'''Test cases for a reference to pointer argument type.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import VirtualMethods, Str

class ExtendedVirtualMethods(VirtualMethods):
    def __init__(self):
        VirtualMethods.__init__(self)
        self.prefix = 'Ext'

    def createStr(self, text):
        ext_text = text
        if text is not None:
            ext_text = self.prefix + text
        return VirtualMethods.createStr(self, ext_text)


class ReferenceToPointerTest(unittest.TestCase):
    '''Test cases for a reference to pointer argument type.'''

    def testSimpleCallWithNone(self):
        '''Simple call to createStr method with a None argument.'''
        obj = VirtualMethods()
        ok, string = obj.createStr(None)
        self.assertFalse(ok)
        self.assertEqual(string, None)

    def testSimpleCallWithString(self):
        '''Simple call to createStr method with a Python string argument.'''
        obj = VirtualMethods()
        ok, string = obj.createStr('foo')
        self.assertTrue(ok)
        self.assertEqual(string, Str('foo'))

    def testCallNonReimplementedMethodWithNone(self):
        '''Calls createStr method from C++ with a None argument.'''
        obj = VirtualMethods()
        ok, string = obj.callCreateStr(None)
        self.assertFalse(ok)
        self.assertEqual(string, None)

    def testCallNonReimplementedMethodWithString(self):
        '''Calls createStr method from C++ with a Python string argument.'''
        obj = VirtualMethods()
        ok, string = obj.callCreateStr('foo')
        self.assertTrue(ok)
        self.assertEqual(string, Str('foo'))

    def testCallReimplementedMethodWithNone(self):
        '''Calls reimplemented createStr method from C++ with a None argument.'''
        obj = ExtendedVirtualMethods()
        ok, string = obj.callCreateStr(None)
        self.assertFalse(ok)
        self.assertEqual(string, None)

    def testCallReimplementedMethodWithString(self):
        '''Calls reimplemented createStr method from C++ with a Python string argument.'''
        obj = ExtendedVirtualMethods()
        ok, string = obj.callCreateStr('foo')
        self.assertTrue(ok)
        self.assertEqual(string, Str(obj.prefix + 'foo'))

if __name__ == '__main__':
    unittest.main()

