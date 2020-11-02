#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

import shiboken6 as shiboken
from PySide6.support import VoidPtr
from PySide6.QtCore import QByteArray

class PySide6Support(unittest.TestCase):

    def testVoidPtr(self):
        # Creating a VoidPtr object requires an address of
        # a C++ object, a wrapped Shiboken Object type,
        # an object implementing the Python Buffer interface,
        # or another VoidPtr object.

        # Original content
        b = b"Hello world"
        ba = QByteArray(b)
        vp = VoidPtr(ba, ba.size())
        self.assertIsInstance(vp, shiboken.VoidPtr)

        # Create QByteArray from voidptr byte interpretation
        nba = QByteArray(vp.toBytes())
        # Compare original bytes to toBytes()
        self.assertTrue(b, vp.toBytes())
        # Compare original with new QByteArray data
        self.assertTrue(b, nba.data())
        # Convert original and new to str
        self.assertTrue(str(b), str(nba))

        # Modify nba through a memoryview of vp
        mv = memoryview(vp)
        self.assertFalse(mv.readonly)
        mv[6:11] = b'void*'
        self.assertEqual(str(ba), str(b"Hello void*"))

if __name__ == '__main__':
    unittest.main()
