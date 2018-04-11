#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

import os, sys
sys.path.insert(0, os.path.join("..", "pysidetest"))

import unittest
from PySide2.QtCore import *
from testbinding import TestObject


class Foo(QObject):
    bar = Signal()

class TestDisconnect(unittest.TestCase):
    def theSlot1(self):
        self.called1 = True

    def theSlot2(self):
        self.called2 = True

    def testIt(self):
        self.called1 = False
        self.called2 = False
        f = Foo()
        f.bar.connect(self.theSlot1)
        f.bar.connect(self.theSlot2)
        f.bar.emit()
        self.assertTrue(self.called1)
        self.assertTrue(self.called2)

        self.called1 = False
        self.called2 = False
        f.bar.disconnect()
        f.bar.emit()
        self.assertFalse(self.called1)
        self.assertFalse(self.called2)

    def testDuringCallback(self):
        """ Test to see if the C++ object for a connection is accessed after the
        method returns.  This causes a segfault if the memory that was used by the
        C++ object has been reused. """

        self.called = False
        obj = TestObject(0)
        def callback():
            obj.signalWithDefaultValue.disconnect(callback)

            # Connect more callbacks to try to overwrite memory
            for i in range(1000):
                obj.signalWithDefaultValue.connect(lambda: None)

            self.called = True

            # A non-None return value is needed
            return True
        obj.signalWithDefaultValue.connect(callback)
        obj.signalWithDefaultValue.emit()
        self.assertTrue(self.called)


if __name__ == '__main__':
    unittest.main()
