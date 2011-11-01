#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# This file is part of the Shiboken Python Bindings Generator project.
#
# Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

'''Tests various usages of the type converters.'''

import unittest
import sample

class GetPythonTypeByNameTest(unittest.TestCase):

    '''Uses an added function with inject code that uses the libshiboken
    function "Shiboken::Conversions::getPythonTypeObject(typeName)".'''

    def testGetObjectType(self):
        pyType1 = sample.getPythonType('ObjectType')
        self.assertEqual(pyType1, sample.ObjectType)
        pyType2 = sample.getPythonType('ObjectType*')
        self.assertEqual(pyType2, sample.ObjectType)
        self.assertEqual(pyType1, pyType2)

    def testGetValueType(self):
        pyType1 = sample.getPythonType('Point')
        self.assertEqual(pyType1, sample.Point)
        pyType2 = sample.getPythonType('Point*')
        self.assertEqual(pyType2, sample.Point)
        self.assertEqual(pyType1, pyType2)

    def testGetUsersPrimitiveType(self):
        pyType = sample.getPythonType('OddBool')
        self.assertEqual(pyType, bool)

    def testGetUsersPrimitiveTypeWithoutTargetLangApiName(self):
        '''If the primitive type attribute "target-lang-api-name" is not set
        there'll be no Python type associated with the C++ type.'''
        pyType = sample.getPythonType('PStr')
        self.assertEqual(pyType, None)

    def testPrimitiveTypeAndTypedef(self):
        pyType = sample.getPythonType('double')
        self.assertEqual(pyType, float)
        pyTypedef = sample.getPythonType('real')
        self.assertEqual(pyType, pyTypedef)

    def testPairContainerType(self):
        pyType = sample.getPythonType('std::pair<Complex, int >')
        self.assertEqual(pyType, list)

    def testListContainerType(self):
        pyType = sample.getPythonType('std::list<int >')
        self.assertEqual(pyType, list)

    def testMapContainerType(self):
        pyType = sample.getPythonType('std::map<std::string, int >')
        self.assertEqual(pyType, dict)

    def testGlobalEnumType(self):
        pyType = sample.getPythonType('GlobalEnum')
        self.assertEqual(pyType, sample.GlobalEnum)

    def testScopedEnumType(self):
        pyType = sample.getPythonType('Abstract::Type')
        self.assertEqual(pyType, sample.Abstract.Type)


class CheckValueAndObjectTypeByNameTest(unittest.TestCase):

    '''Uses an added function with inject code that uses the libshiboken
    functions that check if a type is Object or Value, based on its converter.'''

    def testErrors(self):
        '''not existent type'''
        self.assertRaises(ValueError, sample.cppTypeIsValueType, 'NotExistentType')
        self.assertRaises(ValueError, sample.cppTypeIsObjectType, 'NotExistentType')

    def testObjectType1(self):
        self.assertTrue(sample.cppTypeIsObjectType('ObjectType'))
        self.assertFalse(sample.cppTypeIsValueType('ObjectType'))

    def testObjectType2(self):
        self.assertTrue(sample.cppTypeIsObjectType('ObjectType*'))
        self.assertFalse(sample.cppTypeIsValueType('ObjectType*'))

    def testValueType1(self):
        self.assertTrue(sample.cppTypeIsValueType('Point'))
        self.assertFalse(sample.cppTypeIsObjectType('Point'))

    def testValueType2(self):
        self.assertTrue(sample.cppTypeIsValueType('Point*'))
        self.assertFalse(sample.cppTypeIsObjectType('Point*'))

    def testUsersPrimitiveType(self):
        self.assertFalse(sample.cppTypeIsValueType('Complex'))
        self.assertFalse(sample.cppTypeIsObjectType('Complex'))

    def testContainerType(self):
        self.assertFalse(sample.cppTypeIsValueType('std::list<int >'))
        self.assertFalse(sample.cppTypeIsObjectType('std::list<int >'))


if __name__ == '__main__':
    unittest.main()
