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

import os
import sys
import unittest
import weakref

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QObject, Property

class MyObject(QObject):
    def __init__(self):
        QObject.__init__(self)
        self._value = None

    @Property(int)
    def value(self):
        return self._value

    @value.setter
    # Note: The name of property and setter must be the same, because the
    # object changes its identity all the time. `valueSet` no longer works.
    def value(self, value):
        self._value = value


class PropertyTest(unittest.TestCase):
    def destroyCB(self, obj):
        self._obDestroyed = True

    def testDecorator(self):
        self._obDestroyed = False
        o = MyObject()
        weak = weakref.ref(o, self.destroyCB)
        o.value = 10
        self.assertEqual(o._value, 10)
        self.assertEqual(o.value, 10)
        del o
        self.assertTrue(self._obDestroyed)

if __name__ == '__main__':
    unittest.main()
