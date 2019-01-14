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

'''Test cases for the method overload decisor.'''

import unittest

from sample import SampleNamespace, Point, ObjectType, ObjectModel

class DecisorTest(unittest.TestCase):
    '''Test cases for the method overload decisor.'''

    def testCallWithInvalidParametersSideA(self):
        '''Call a method missing with the last argument missing.
        This can trigger the bug #262, which means using an argument
        not provided by the user.'''
        pt = Point()
        # This exception may move from a TypeError to a ValueError.
        self.assertRaises((TypeError, ValueError), SampleNamespace.forceDecisorSideA, pt)

    def testCallWithInvalidParametersSideB(self):
        '''Same as the previous test, but with an integer as first argument,
        just to complicate things for the overload method decisor.'''
        pt = Point()
        # This exception may move from a TypeError to a ValueError.
        self.assertRaises((TypeError, ValueError), SampleNamespace.forceDecisorSideB, 1, pt)

    def testDecideCallWithInheritance(self):
        '''Call methods overloads that receive parent and inheritor classes' instances.'''
        objecttype = ObjectType()
        objectmodel = ObjectModel()
        self.assertEqual(ObjectModel.receivesObjectTypeFamily(objecttype), ObjectModel.ObjectTypeCalled)
        self.assertNotEqual(ObjectModel.receivesObjectTypeFamily(objecttype), ObjectModel.ObjectModelCalled)
        self.assertEqual(ObjectModel.receivesObjectTypeFamily(objectmodel), ObjectModel.ObjectModelCalled)
        self.assertNotEqual(ObjectModel.receivesObjectTypeFamily(objectmodel), ObjectModel.ObjectTypeCalled)

if __name__ == '__main__':
    unittest.main()

