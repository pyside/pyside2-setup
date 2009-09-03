'''
This file is part of the Shiboken Python Bindings Generator project.

Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

Contact: PySide team <contact@pyside.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
version 2.1 as published by the Free Software Foundation. Please
review the following information to ensure the GNU Lesser General
Public License version 2.1 requirements will be met:
http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
02110-1301 USA
'''

#!/usr/bin/python
# -*- coding: utf-8 -*-

'''Test cases for Python representation of C++ enums'''

import sys
import unittest

from sample import SampleNamespace

class EnumTest(unittest.TestCase):
    '''Test case for Abstract class'''

    def testPassingIntegerOnEnumArgument(self):
        '''Test if replacing an enum argument with an integer raises an exception.'''
        self.assertRaises(TypeError, lambda : SampleNamespace.getNumber(1))

    def testExtendingEnum(self):
        '''Test if can create new items for an enum declared as extensible on the typesystem file.'''
        name, value = 'NewItem', 13
        enumitem = SampleNamespace.Option(name, value)
        self.assert_(type(enumitem), SampleNamespace.Option)
        self.assert_(enumitem.name, name)
        self.assert_(int(enumitem), value)

    def testExtendingNonExtensibleEnum(self):
        '''Test if trying to create a new enum item for an unextensible enum raises an exception.'''
        self.assertRaises(TypeError, lambda : SampleNamespace.InValue(13))

    def testEnumConversionToAndFromPython(self):
        '''Test conversion of enum objects to Python and C++ in both directions.'''
        enumout = SampleNamespace.enumInEnumOut(SampleNamespace.TwoIn)
        self.assert_(enumout, SampleNamespace.TwoOut)

if __name__ == '__main__':
    unittest.main()

