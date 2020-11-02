#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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
PYSIDE-68: Test that signals have a `__get__` function after all.

We supply a `tp_descr_get` slot for the signal type.
That creates the `__get__` method via `PyType_Ready`.

The original test script was converted to a unittest.
See https://bugreports.qt.io/browse/PYSIDE-68 .

Created:    16 May '12 21:25
Updated:    17 Sep '20 17:02

This fix was over 8 years late. :)
"""

from __future__ import print_function

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6 import QtCore


def emit_upon_success(signal):
    def f_(f):
        def f__(self):
            result = f(self)
            s = signal.__get__(self)
            print(result)
            return result
        return f__
    return f_


class Foo(QtCore.QObject):
    SIG = QtCore.Signal()

    @emit_upon_success(SIG)
    def do_something(self):
        print("hooka, it worrrks")
        return 42


class UnderUnderGetUnderUnderTest(unittest.TestCase):
    def test_tp_descr_get(self):
        foo = Foo()
        ret = foo.do_something()
        self.assertEqual(ret, 42)


if __name__ == "__main__":
    unittest.main()
