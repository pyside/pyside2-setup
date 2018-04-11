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

'''Tests calling NoImplicitConversion using a ExtendsNoImplicitConversion parameter,
   being that the latter defines a new conversion operator for the former, and this one
   has no implicit conversions.'''

import unittest

from sample import NoImplicitConversion
from other import ExtendsNoImplicitConversion

class ConversionOperatorForClassWithoutImplicitConversionsTest(unittest.TestCase):
    '''Tests calling NoImplicitConversion constructor using a ExtendsNoImplicitConversion parameter.'''

    def testNoImplicitConversion(self):
        '''Basic test to see if the NoImplicitConversion is Ok.'''
        obj = NoImplicitConversion(123)
        # NoImplicitConversion.receivesNoImplicitConversionByValue(NoImplicitConversion)
        self.assertEqual(obj.objId(), NoImplicitConversion.receivesNoImplicitConversionByValue(obj))
        # NoImplicitConversion.receivesNoImplicitConversionByPointer(NoImplicitConversion*)
        self.assertEqual(obj.objId(), NoImplicitConversion.receivesNoImplicitConversionByPointer(obj))
        # NoImplicitConversion.receivesNoImplicitConversionByReference(NoImplicitConversion&)
        self.assertEqual(obj.objId(), NoImplicitConversion.receivesNoImplicitConversionByReference(obj))

    def testPassingExtendsNoImplicitConversionAsNoImplicitConversionByValue(self):
        '''Gives an ExtendsNoImplicitConversion object to a function expecting a NoImplicitConversion, passing by value.'''
        obj = ExtendsNoImplicitConversion(123)
        self.assertEqual(obj.objId(), NoImplicitConversion.receivesNoImplicitConversionByValue(obj))

    def testPassingExtendsNoImplicitConversionAsNoImplicitConversionByReference(self):
        '''Gives an ExtendsNoImplicitConversion object to a function expecting a NoImplicitConversion, passing by reference.'''
        obj = ExtendsNoImplicitConversion(123)
        self.assertEqual(obj.objId(), NoImplicitConversion.receivesNoImplicitConversionByReference(obj))

    def testPassingExtendsNoImplicitConversionAsNoImplicitConversionByPointer(self):
        '''Gives an ExtendsNoImplicitConversion object to a function expecting a NoImplicitConversion, passing by pointer.
           This should not be accepted, since pointers should not be converted.'''
        obj = ExtendsNoImplicitConversion(123)
        self.assertRaises(TypeError, NoImplicitConversion.receivesNoImplicitConversionByPointer, obj)


if __name__ == '__main__':
    unittest.main()

