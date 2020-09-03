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

from enum import Enum
import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import QObject, Signal, Slot


class Colors(Enum):
    red = 1
    green = 2
    blue = 3

class Obj(QObject):
    enum_signal = Signal(Colors)
    object_signal = Signal(object)

    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self.enum_signal.connect(self.get_result)
        self.object_signal.connect(self.get_result)
        self.value = -1

    @Slot()
    def get_result(self, i):
        self.value = i


class SignalEnumTests(unittest.TestCase):
    '''Test Signal with enum.Enum'''

    def testSignal(self):
        o = Obj()
        # Default value
        self.assertEqual(o.value, -1)

        # Enum Signal
        o.enum_signal.emit(Colors.green)
        self.assertEqual(o.value, Colors.green)

        # object Signal
        o.object_signal.emit(Colors.red)
        self.assertEqual(o.value, Colors.red)


if __name__ == '__main__':
    unittest.main()
