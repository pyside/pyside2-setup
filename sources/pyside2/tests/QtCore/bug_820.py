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

import functools
import unittest

from PySide2.QtCore import QObject, Slot, Signal, SIGNAL

def log_exception():
    def log_exception_decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwds):
            try:
                return func(*args, **kwds)
            except Exception:
                raise

        return wrapper

    return log_exception_decorator


def log_exception2():
    def log_exception_decorator(func):
        def wrapper(*args, **kwds):
            try:
                return func(*args, **kwds)
            except Exception:
                raise

        return wrapper

    return log_exception_decorator

class MyObject(QObject):

    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self._mySlotcalled = False
        self._mySlot2called = False

    @Slot()
    @log_exception()
    def mySlot(self):
        self._mySlotcalled = True

    @Slot(name="mySlot2")
    @log_exception2()
    def mySlot2(self):
        self._mySlot2called = True

    def poke(self):
        self.events.emit()

    events = Signal()


class SlotWithDecoratorTest(unittest.TestCase):
    def testSlots(self):
        o = MyObject()
        self.assertTrue(o.metaObject().indexOfSlot("mySlot()") > 0)
        self.assertTrue(o.metaObject().indexOfSlot("mySlot2()") > 0)

        o.events.connect(o.mySlot)
        o.events.connect(o.mySlot2)
        o.poke()
        self.assertTrue(o._mySlotcalled)
        self.assertTrue(o._mySlot2called)

if __name__ == '__main__':
    unittest.main()

