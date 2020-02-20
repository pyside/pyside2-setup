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

'''Test case for a class that holds a void pointer.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import VoidHolder, Point
import shiboken2 as shiboken

class VoidHolderTest(unittest.TestCase):
    '''Test case for void pointer manipulation.'''

    def testGetVoidPointerFromCppAndPutsOnVoidHolder(self):
        '''Passes a void pointer created in C++ to be kept by VoidHolder.'''
        voidptr = VoidHolder.gimmeMeSomeVoidPointer()
        voidholder = VoidHolder(voidptr)
        self.assertEqual(voidptr, voidholder.voidPointer())

    def testPassVoidPointerAsArgument(self):
        '''Passes a void pointer created in C++ as an argument to a function.'''
        voidptr = VoidHolder.gimmeMeSomeVoidPointer()
        voidHolder = VoidHolder()
        returnValue = voidHolder.takeVoidPointer(voidptr)
        self.assertEqual(returnValue, voidptr)

    def testPutRandomObjectInsideVoidHolder(self):
        '''Passes a C++ pointer for an object created in Python to be kept by VoidHolder.'''
        obj = Point(1, 2)
        voidholder = VoidHolder(obj)
        self.assertEqual(shiboken.getCppPointer(obj)[0], int(voidholder.voidPointer()))

    def testGetNoneObjectFromVoidHolder(self):
        '''A VoidHolder created without parameters returns a NULL pointer
           that should be converted to a Python None.'''
        voidholder = VoidHolder()
        self.assertEqual(voidholder.voidPointer(), None)

if __name__ == '__main__':
    unittest.main()

