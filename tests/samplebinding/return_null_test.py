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

'''Test case for functions that could return a NULL pointer.'''

import sys
import unittest

from sample import returnNullPrimitivePointer, returnNullValueTypePointer, returnNullObjectTypePointer

class ReturnNullTest(unittest.TestCase):
    '''Test case for functions that could return a NULL pointer.'''

    def testReturnNull(self):
        '''Function returns a NULL pointer to a primitive type.'''
        o = returnNullPrimitivePointer()
        self.assertEqual(o, None)

    def testReturnNullObjectType(self):
        '''Function returns a NULL pointer to an object-type.'''
        o = returnNullObjectTypePointer()
        self.assertEqual(o, None)

    def testReturnNullValueType(self):
        '''Function returns a NULL pointer to a value-type.'''
        o = returnNullValueTypePointer()
        self.assertEqual(o, None)

if __name__ == '__main__':
    unittest.main()

