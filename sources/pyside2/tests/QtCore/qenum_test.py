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

'''Test cases for QEnum and QFlags'''

import gc
import os
import sys
import pickle
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import Qt, QIODevice, QObject, QEnum, QFlag


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
            a = k + 2.0

        with self.assertRaises(TypeError):
            a = k - 2.0

        with self.assertRaises(TypeError):
            a = k * 2.0

    @unittest.skipUnless(getattr(sys, "getobjects", None), "requires --with-trace-refs")
    @unittest.skipUnless(getattr(sys, "gettotalrefcount", None), "requires --with-pydebug")
    def testEnumNew_NoLeak(self):
        gc.collect()
        total = sys.gettotalrefcount()
        for idx in range(1000):
            ret = Qt.Key(42)

        gc.collect()
        delta = sys.gettotalrefcount() - total
        print("delta total refcount =", delta)
        if abs(delta) >= 10:
            all = [(sys.getrefcount(x), x) for x in sys.getobjects(0)]
            all.sort(key=lambda x: x[0], reverse=True)
            for ob in all[:10]:
                print(ob)
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

# PYSIDE-957: The QEnum macro

try:
    import enum
    HAVE_ENUM = True
except ImportError:
    HAVE_ENUM = False
    QEnum = QFlag = lambda x: x
    import types
    class Enum: pass
    enum = types.ModuleType("enum")
    enum.Enum = enum.Flag = enum.IntEnum = enum.IntFlag = Enum
    Enum.__module__ = "enum"
    Enum.__members__ = {}
    del Enum
    enum.auto = lambda: 42

HAVE_FLAG = hasattr(enum, "Flag")

@QEnum
class OuterEnum(enum.Enum):
    A = 1
    B = 2

class SomeClass(QObject):

    @QEnum
    class SomeEnum(enum.Enum):
        A = 1
        B = 2
        C = 3

    @QEnum
    class OtherEnum(enum.IntEnum):
        A = 1
        B = 2
        C = 3

    class InnerClass(QObject):

        @QEnum
        class InnerEnum(enum.Enum):
            X = 42

    class SomeEnum(enum.Enum):
        A = 4
        B = 5
        C = 6

    QEnum(SomeEnum)     # works even without the decorator assignment


@unittest.skipUnless(HAVE_ENUM, "requires 'enum' module (use 'pip install enum34' for Python 2)")
class TestQEnumMacro(unittest.TestCase):
    def testTopLevel(self):
        self.assertEqual(type(OuterEnum).__module__, "enum")
        self.assertEqual(type(OuterEnum).__name__, "EnumMeta")
        self.assertEqual(len(OuterEnum.__members__), 2)

    def testSomeClass(self):
        self.assertEqual(type(SomeClass.SomeEnum).__module__, "enum")
        self.assertEqual(type(SomeClass.SomeEnum).__name__, "EnumMeta")
        self.assertEqual(len(SomeClass.SomeEnum.__members__), 3)
        with self.assertRaises(TypeError):
            int(SomeClass.SomeEnum.C) == 6
        self.assertEqual(SomeClass.OtherEnum.C, 3)

    @unittest.skipIf(sys.version_info[0] < 3, "we cannot support nested classes in Python 2")
    def testInnerClass(self):
        self.assertEqual(SomeClass.InnerClass.InnerEnum.__qualname__,
            "SomeClass.InnerClass.InnerEnum")
        with self.assertRaises(TypeError):
            int(SomeClass.InnerClass.InnerEnum.X) == 42

    @unittest.skipUnless(HAVE_FLAG, "some older Python versions have no 'Flag'")
    def testEnumFlag(self):
        with self.assertRaises(TypeError):
            class WrongFlagForEnum(QObject):
                @QEnum
                class Bad(enum.Flag):
                    pass
        with self.assertRaises(TypeError):
            class WrongEnuForFlag(QObject):
                @QFlag
                class Bad(enum.Enum):
                    pass

    def testIsRegistered(self):
        mo = SomeClass.staticMetaObject
        self.assertEqual(mo.enumeratorCount(), 2)
        self.assertEqual(mo.enumerator(0).name(), "OtherEnum")
        self.assertEqual(mo.enumerator(0).scope(), "SomeClass")
        self.assertEqual(mo.enumerator(1).name(), "SomeEnum")
        moi = SomeClass.InnerClass.staticMetaObject
        self.assertEqual(moi.enumerator(0).name(), "InnerEnum")
        ## Question: Should that scope not better be  "SomeClass.InnerClass"?
        ## But we have __qualname__ already:
        self.assertEqual(moi.enumerator(0).scope(), "InnerClass")


if __name__ == '__main__':
    unittest.main()
