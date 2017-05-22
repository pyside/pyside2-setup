#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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

import unittest
from PySide2.QtCore import QObject
from testbinding import TestObject

class Receiver(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.called = False

    def ReceiverDecorator(func):
        def decoratedFunction(self, *args, **kw):
            func(self, *args, **kw)
        return decoratedFunction

    # This method with the same name of the internal decorated function
    # is here to test the binding capabilities.
    def decoratedFunction(self):
        pass

    @ReceiverDecorator
    def slot(self):
        self.called = True


class DecoratedSlotTest(unittest.TestCase):

    def testCallingOfDecoratedSlot(self):
        obj = TestObject(0)
        receiver = Receiver()
        obj.staticMethodDouble.connect(receiver.slot)
        obj.emitStaticMethodDoubleSignal()
        self.assertTrue(receiver.called)


if __name__ == '__main__':
    unittest.main()

