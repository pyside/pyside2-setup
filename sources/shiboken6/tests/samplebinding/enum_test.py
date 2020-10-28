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

'''Test cases for Python representation of C++ enums.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

import shiboken6
# This is needed after the introduction of BUILD_DIR.

import sample
from sample import SampleNamespace, ObjectType, Event

def createTempFile():
    import tempfile
    return tempfile.SpooledTemporaryFile(mode='rw')

class EnumTest(unittest.TestCase):
    '''Test case for Python representation of C++ enums.'''

    def testEnumRepr(self):
        enum = SampleNamespace.Option(1)
        self.assertEqual(eval(repr(enum)), enum)

        enum = SampleNamespace.Option(999)
        self.assertEqual(eval(repr(enum)), enum)

    def testHashability(self):
        self.assertEqual(hash(SampleNamespace.TwoIn), hash(SampleNamespace.TwoOut))
        self.assertNotEqual(hash(SampleNamespace.TwoIn), hash(SampleNamespace.OneIn))

    def testEnumValuesInsideEnum(self):
        '''Enum values should be accessible inside the enum as well as outside.'''
        for value_name in SampleNamespace.Option.values:
            enum_item1 = getattr(SampleNamespace.Option, value_name)
            enum_item2 = getattr(SampleNamespace, value_name)
            self.assertEqual(enum_item1, enum_item2)

    def testPassingIntegerOnEnumArgument(self):
        '''Tries to use an integer in place of an enum argument.'''
        self.assertRaises(TypeError, SampleNamespace.getNumber, 1)

    def testBuildingEnumFromIntegerValue(self):
        '''Tries to build the proper enum using an integer.'''
        SampleNamespace.getNumber(SampleNamespace.Option(1))

    def testBuildingEnumWithDefaultValue(self):
        '''Enum constructor with default value'''
        enum = SampleNamespace.Option()
        self.assertEqual(enum, SampleNamespace.None_)

    def testEnumConversionToAndFromPython(self):
        '''Conversion of enum objects from Python to C++ back again.'''
        enumout = SampleNamespace.enumInEnumOut(SampleNamespace.TwoIn)
        self.assertTrue(enumout, SampleNamespace.TwoOut)
        self.assertEqual(repr(enumout), repr(SampleNamespace.TwoOut))

    def testEnumConstructorWithTooManyParameters(self):
        '''Calling the constructor of non-extensible enum with the wrong number of parameters.'''
        self.assertRaises(TypeError, SampleNamespace.InValue, 13, 14)

    def testEnumConstructorWithNonNumberParameter(self):
        '''Calling the constructor of non-extensible enum with a string.'''
        self.assertRaises(TypeError, SampleNamespace.InValue, '1')

    def testEnumItemAsDefaultValueToIntArgument(self):
        '''Calls function with an enum item as default value to an int argument.'''
        self.assertEqual(SampleNamespace.enumItemAsDefaultValueToIntArgument(), SampleNamespace.ZeroIn)
        self.assertEqual(SampleNamespace.enumItemAsDefaultValueToIntArgument(SampleNamespace.ZeroOut), SampleNamespace.ZeroOut)
        self.assertEqual(SampleNamespace.enumItemAsDefaultValueToIntArgument(123), 123)

    def testAnonymousGlobalEnums(self):
        '''Checks availability of anonymous global enum items.'''
        self.assertEqual(sample.AnonymousGlobalEnum_Value0, 0)
        self.assertEqual(sample.AnonymousGlobalEnum_Value1, 1)

    def testAnonymousClassEnums(self):
        '''Checks availability of anonymous class enum items.'''
        self.assertEqual(SampleNamespace.AnonymousClassEnum_Value0, 0)
        self.assertEqual(SampleNamespace.AnonymousClassEnum_Value1, 1)

    def testEnumClasses(self):
        # C++ 11: values of enum classes need to be fully qualified to match C++
        sum = Event.EventTypeClass.Value1 + Event.EventTypeClass.Value2
        self.assertEqual(sum, 1)

    def testSetEnum(self):
        event = Event(Event.ANY_EVENT)
        self.assertEqual(event.eventType(), Event.ANY_EVENT)
        event.setEventType(Event.BASIC_EVENT)
        self.assertEqual(event.eventType(), Event.BASIC_EVENT)
        event.setEventTypeByConstRef(Event.SOME_EVENT)
        self.assertEqual(event.eventType(), Event.SOME_EVENT)

    def testEnumTpPrintImplementation(self):
        '''Without SbkEnum.tp_print 'print' returns the enum represented as an int.'''
        tmpfile = createTempFile()
        print(Event.ANY_EVENT, file=tmpfile)
        tmpfile.seek(0)
        text = tmpfile.read().strip()
        tmpfile.close()
        self.assertEqual(text, str(Event.ANY_EVENT))
        self.assertEqual(text, repr(Event.ANY_EVENT))

    def testEnumArgumentWithDefaultValue(self):
        '''Option enumArgumentWithDefaultValue(Option opt = UnixTime);'''
        self.assertEqual(SampleNamespace.enumArgumentWithDefaultValue(), SampleNamespace.UnixTime)
        self.assertEqual(SampleNamespace.enumArgumentWithDefaultValue(SampleNamespace.RandomNumber), SampleNamespace.RandomNumber)


class MyEvent(Event):
    def __init__(self):
        Event.__init__(self, Event.EventType(999))

class OutOfBoundsTest(unittest.TestCase):
    def testValue(self):
        e = MyEvent()
        self.assertEqual(repr(e.eventType()), 'sample.Event.EventType(999)')

    def testNoneName(self):
        e = MyEvent()
        t = e.eventType()
        self.assertEqual(t.name, None)

class EnumOverloadTest(unittest.TestCase):
    '''Test case for overloads involving enums'''

    def testWithInt(self):
        '''Overload with Enums and ints with default value'''
        o = ObjectType()

        self.assertEqual(o.callWithEnum('', Event.ANY_EVENT, 9), 81)
        self.assertEqual(o.callWithEnum('', 9), 9)

class EnumOperators(unittest.TestCase):
    '''Test case for operations on enums'''

    def testInequalitySameObject(self):
        self.assertFalse(Event.ANY_EVENT != Event.ANY_EVENT)


if __name__ == '__main__':
    unittest.main()

