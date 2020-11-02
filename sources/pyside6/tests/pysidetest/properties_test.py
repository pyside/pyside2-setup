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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QObject, QStringListModel, Signal, Property, Slot

"""Tests PySide6.QtCore.Property()"""


class TestObject(QObject):

    valueChanged = Signal()

    def __init__(self, parent=None):
        super(TestObject, self).__init__(parent)
        self._value = -1
        self.valueChanged.connect(self._changed)
        self.getter_called = 0
        self.setter_called = 0
        self.changed_emitted = 0

    @Slot(int)
    def _changed(self):
        self.changed_emitted += 1

    def getValue(self):
        self.getter_called += 1
        return self._value

    def setValue(self, value):
        self.setter_called += 1
        if (self._value != value):
            self._value = value
            self.valueChanged.emit()

    value = Property(int, fget=getValue, fset=setValue,
                     notify=valueChanged)


class TestDerivedObject(QStringListModel):

    valueChanged = Signal()

    def __init__(self, parent=None):
        super(TestDerivedObject, self).__init__(parent)
        self._value = -1
        self.valueChanged.connect(self._changed)
        self.getter_called = 0
        self.setter_called = 0
        self.changed_emitted = 0

    @Slot(int)
    def _changed(self):
        self.changed_emitted += 1

    def getValue(self):
        self.getter_called += 1
        return self._value

    def setValue(self, value):
        self.setter_called += 1
        if (self._value != value):
            self._value = value
            self.valueChanged.emit()

    value = Property(int, fget=getValue, fset=setValue,
                     notify=valueChanged)


class PropertyTest(unittest.TestCase):

    def test1Object(self):
        """Basic property test."""
        testObject = TestObject()
        v = testObject.value
        self.assertEqual(v, -1)
        self.assertEqual(testObject.getter_called, 1)
        testObject.value = 42
        v = testObject.value
        self.assertEqual(v, 42)
        self.assertEqual(testObject.changed_emitted, 1)
        self.assertEqual(testObject.setter_called, 1)
        self.assertEqual(testObject.getter_called, 2)

    def test2DerivedObject(self):
        """PYSIDE-1255: Run the same test for a class inheriting QObject."""
        testObject = TestDerivedObject()
        v = testObject.value
        self.assertEqual(v, -1)
        self.assertEqual(testObject.getter_called, 1)
        testObject.value = 42
        v = testObject.value
        self.assertEqual(v, 42)
        self.assertEqual(testObject.changed_emitted, 1)
        self.assertEqual(testObject.setter_called, 1)
        self.assertEqual(testObject.getter_called, 2)


if __name__ == '__main__':
    unittest.main()
