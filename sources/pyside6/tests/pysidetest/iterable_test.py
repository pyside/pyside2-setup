#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

"""
iterable_test.py

This test checks that the Iterable protocol is implemented correctly.
"""

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

import PySide6
from PySide6 import QtCore, QtGui

try:
    import numpy as np
    have_numpy = True
except ImportError:
    have_numpy = False

class PySequenceTest(unittest.TestCase):

    def test_iterable(self):
        def gen(lis):
            for item in lis:
                if item == "crash":
                    raise IndexError
                yield item
        # testing "pyseq_to_cpplist_conversion"
        testfunc = QtCore.QUrl.fromStringList
        # use a generator (iterable)
        self.assertEqual(testfunc(gen(["asd", "ghj"])),
            [PySide6.QtCore.QUrl('asd'), PySide6.QtCore.QUrl('ghj')])
        # use an iterator
        self.assertEqual(testfunc(iter(["asd", "ghj"])),
            [PySide6.QtCore.QUrl('asd'), PySide6.QtCore.QUrl('ghj')])
        self.assertRaises(IndexError, testfunc, gen(["asd", "crash", "ghj"]))
        # testing QMatrix4x4
        testfunc = QtGui.QMatrix4x4
        self.assertEqual(testfunc(gen(range(16))), testfunc(range(16)))
        # Note: The errormessage needs to be improved!
        # We should better get a ValueError
        self.assertRaises((TypeError, ValueError), testfunc, gen(range(15)))
        # All other matrix sizes:
        testfunc = QtGui.QMatrix2x2
        self.assertEqual(testfunc(gen(range(4))), testfunc(range(4)))
        testfunc = QtGui.QMatrix2x3
        self.assertEqual(testfunc(gen(range(6))), testfunc(range(6)))

    @unittest.skipUnless(have_numpy, "requires numpy")
    def test_iterable_numpy(self):
        # Demo for numpy: We create a unit matrix.
        num_mat = np.eye(4)
        num_mat.shape = 16
        unit = QtGui.QMatrix4x4(num_mat)
        self.assertEqual(unit, QtGui.QMatrix4x4())


if __name__ == '__main__':
    unittest.main()
