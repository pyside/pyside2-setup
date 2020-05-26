#!/usr/bin/python

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

'''Test cases for QEnum and QFlags'''

import gc
import os
import sys
import pickle
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import Qt, QIODevice


class TestEnum(unittest.TestCase):

    def testToInt(self):
        self.assertEqual(QIODevice.NotOpen, 0)
        self.assertEqual(QIODevice.ReadOnly, 1)
        self.assertEqual(QIODevice.WriteOnly, 2)
        self.assertEqual(QIODevice.ReadWrite, 1 | 2)
        self.assertEqual(QIODevice.Append, 4)
        self.assertEqual(QIODevice.Truncate, 8)
        self.assertEqual(QIODevice.Text, 16)
        self.assertEqual(QIODevice.Unbuffered, 32)

    def testToIntInFunction(self):
        self.assertEqual(str(int(QIODevice.WriteOnly)), "2")

    def testOperations(self):
        k = Qt.Key.Key_1

        # Integers
        self.assertEqual(k + 2, 2 + k)
        self.assertEqual(k - 2, -(2 - k))
        self.assertEqual(k * 2, 2 * k)

        # Floats
        with self.assertRaises(TypeError):
            a = k+2.0

        with self.assertRaises(TypeError):
            a = k-2.0

        with self.assertRaises(TypeError):
            a = k*2.0

    @unittest.skipUnless(getattr(sys, "getobjects", None), "requires debug build")
    def testEnumNew_NoLeak(self):
        gc.collect()
        total = sys.gettotalrefcount()
        for idx in range(1000):
            ret = Qt.Key(42)
        gc.collect()
        delta = sys.gettotalrefcount() - total
        print("delta total refcount =", delta)
        if abs(delta) >= 10:
            all = sys.getobjects(0)
            all.sort(key=lambda x: sys.getrefcount(x), reverse=True)
            for ob in all[:10]:
                print(sys.getrefcount(ob), ob)
        self.assertTrue(abs(delta) < 10)


class TestQFlags(unittest.TestCase):
    def testToItn(self):
        om = QIODevice.NotOpen

        self.assertEqual(om, QIODevice.NotOpen)
        self.assertTrue(om == 0)

        self.assertTrue(om != QIODevice.ReadOnly)
        self.assertTrue(om != 1)

    def testToIntInFunction(self):
        om = QIODevice.WriteOnly
        self.assertEqual(int(om), 2)

    def testNonExtensibleEnums(self):
        try:
            om = QIODevice.OpenMode(QIODevice.WriteOnly)
            self.assertFail()
        except:
            pass


# PYSIDE-15: Pickling of enums
class TestEnumPickling(unittest.TestCase):
    def testPickleEnum(self):

        # Pickling of enums with different depth works.
        ret = pickle.loads(pickle.dumps(QIODevice.Append))
        self.assertEqual(ret, QIODevice.Append)

        ret = pickle.loads(pickle.dumps(Qt.Key.Key_Asterisk))
        self.assertEqual(ret, Qt.Key.Key_Asterisk)
        self.assertEqual(ret, Qt.Key(42))

        # We can also pickle the whole enum class (built in):
        ret = pickle.loads(pickle.dumps(QIODevice))

        # This works also with nested classes for Python 3, after we
        # introduced the correct __qualname__ attribute.

        # Note: For Python 2, we would need quite strange patches.
        func = lambda: pickle.loads(pickle.dumps(Qt.Key))
        if sys.version_info[0] < 3:
            with self.assertRaises(pickle.PicklingError):
                func()
        else:
            func()


if __name__ == '__main__':
    unittest.main()
