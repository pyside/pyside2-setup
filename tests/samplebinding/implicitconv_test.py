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

'''Test cases for implicit conversions'''

import unittest

from sample import ImplicitConv, ObjectType

class ImplicitConvTest(unittest.TestCase):
    '''Test case for implicit conversions'''

    def testImplicitConversions(self):
        '''Test if overloaded function call decisor takes implicit conversions into account.'''
        ic = ImplicitConv.implicitConvCommon(ImplicitConv())
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorNone)

        ic = ImplicitConv.implicitConvCommon(3)
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorOne)
        self.assertEqual(ic.objId(), 3)

        ic = ImplicitConv.implicitConvCommon(ImplicitConv.CtorThree)
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorThree)

        obj = ObjectType()
        ic = ImplicitConv.implicitConvCommon(obj)
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorObjectTypeReference)

        ic = ImplicitConv.implicitConvCommon(42.42)
        self.assertEqual(ic.value(), 42.42)

        ic = ImplicitConv(None)
        self.assertEqual(ic.ctorEnum(), ImplicitConv.CtorPrimitiveType)


if __name__ == '__main__':
    unittest.main()

