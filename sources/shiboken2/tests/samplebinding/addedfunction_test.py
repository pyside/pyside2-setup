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

'''Test cases for added functions.'''

import unittest
from sample import SampleNamespace, ObjectType, Point

class TestAddedFunctionsWithSimilarTypes(unittest.TestCase):
    '''Adds new signatures very similar to already existing ones.'''

    def testValueTypeReferenceAndValue(self):
        '''In C++ we have "function(const ValueType&, double)",
        in Python we add "function(ValueType)".'''
        point = Point(10, 20)
        multiplier = 4.0
        control = (point.x() + point.y()) * multiplier
        self.assertEqual(SampleNamespace.passReferenceToValueType(point, multiplier), control)
        control = point.x() + point.y()
        self.assertEqual(SampleNamespace.passReferenceToValueType(point), control)

    def testObjectTypeReferenceAndPointer(self):
        '''In C++ we have "function(const ObjectType&, int)",
        in Python we add "function(ValueType)".'''
        obj = ObjectType()
        obj.setObjectName('sbrubbles')
        multiplier = 3.0
        control = len(obj.objectName()) * multiplier
        self.assertEqual(SampleNamespace.passReferenceToObjectType(obj, multiplier), control)
        control = len(obj.objectName())
        self.assertEqual(SampleNamespace.passReferenceToObjectType(obj), control)

if __name__ == '__main__':
    unittest.main()
