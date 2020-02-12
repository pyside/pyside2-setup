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

''' PYSIDE-315: https://bugreports.qt.io/browse/PYSIDE-315
    Test that all signals and slots of a class (including any mixin classes)
    are registered at type parsing time. Also test that the signal and slot
    indices do not change after signal connection or emission. '''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2 import QtCore

class Mixin(object):
    mixinSignal = QtCore.Signal()
    def __init__(self, *args, **kwargs):
        super(Mixin,self).__init__(*args, **kwargs)

class MixinTwo(Mixin):
    mixinTwoSignal = QtCore.Signal()

    def __init__(self, *args, **kwargs):
        super(MixinTwo,self).__init__(*args, **kwargs)
        self.mixinTwoSlotCalled = False

    @QtCore.Slot()
    def mixinTwoSlot(self):
        self.mixinTwoSlotCalled = True

class MixinThree(object):
    mixinThreeSignal = QtCore.Signal()

    def __init__(self, *args, **kwargs):
        super(MixinThree,self).__init__(*args, **kwargs)
        self.mixinThreeSlotCalled = False

    @QtCore.Slot()
    def mixinThreeSlot(self):
        self.mixinThreeSlotCalled = True

class Derived(Mixin, QtCore.QObject):
    derivedSignal = QtCore.Signal(str)

    def __init__(self):
        super(Derived,self).__init__()
        self.derivedSlotCalled = False
        self.derivedSlotString = ''

    @QtCore.Slot(str)
    def derivedSlot(self, theString):
        self.derivedSlotCalled = True
        self.derivedSlotString = theString

class MultipleDerived(MixinTwo, MixinThree, Mixin, QtCore.QObject):
    derivedSignal = QtCore.Signal(str)

    def __init__(self):
        super(MultipleDerived,self).__init__()
        self.derivedSlotCalled = False
        self.derivedSlotString = ''

    @QtCore.Slot(str)
    def derivedSlot(self, theString):
        self.derivedSlotCalled = True
        self.derivedSlotString = theString


class MixinTest(unittest.TestCase):
    def testMixinSignalSlotRegistration(self):
        obj = Derived()
        m = obj.metaObject()

        # Should contain 2 signals and 1 slot immediately after type parsing
        self.assertEqual(m.methodCount() - m.methodOffset(), 3)

        # Save method indices to check that they do not change
        methodIndices = {}
        for i in range(m.methodOffset(), m.methodCount()):
            signature = m.method(i).methodSignature()
            methodIndices[signature] = i

        # Check derivedSignal emission
        obj.derivedSignal.connect(obj.derivedSlot)
        obj.derivedSignal.emit('emit1')
        self.assertTrue(obj.derivedSlotCalled)
        obj.derivedSlotCalled = False

        # Check derivedSignal emission after mixingSignal connection
        self.outsideSlotCalled = False
        @QtCore.Slot()
        def outsideSlot():
            self.outsideSlotCalled = True

        obj.mixinSignal.connect(outsideSlot)
        obj.derivedSignal.emit('emit2')
        self.assertTrue(obj.derivedSlotCalled)
        self.assertFalse(self.outsideSlotCalled)
        obj.derivedSlotCalled = False

        # Check mixinSignal emission
        obj.mixinSignal.emit()
        self.assertTrue(self.outsideSlotCalled)
        self.assertFalse(obj.derivedSlotCalled)
        self.outsideSlotCalled = False

        # Check that method indices haven't changed.
        # Make sure to requery for the meta object, to check that a new one was not
        # created as a child of the old one.
        m = obj.metaObject()
        self.assertEqual(m.methodCount() - m.methodOffset(), 3)
        for i in range(m.methodOffset(), m.methodCount()):
            signature = m.method(i).methodSignature()
            self.assertEqual(methodIndices[signature], i)


    def testMixinSignalSlotRegistrationWithMultipleInheritance(self):
        obj = MultipleDerived()
        m = obj.metaObject()

        # Should contain 4 signals and 3 slots immediately after type parsing
        self.assertEqual(m.methodCount() - m.methodOffset(), 7)

        # Save method indices to check that they do not change
        methodIndices = {}
        for i in range(m.methodOffset(), m.methodCount()):
            signature = m.method(i).methodSignature()
            methodIndices[signature] = i

        # Check derivedSignal emission
        obj.derivedSignal.connect(obj.derivedSlot)
        obj.derivedSignal.emit('emit1')
        self.assertTrue(obj.derivedSlotCalled)
        self.assertFalse(obj.mixinTwoSlotCalled)
        self.assertFalse(obj.mixinThreeSlotCalled)
        obj.derivedSlotCalled = False

        # Check derivedSignal emission after mixinThreeSignal connection
        self.outsideSlotCalled = False
        @QtCore.Slot()
        def outsideSlot():
            self.outsideSlotCalled = True

        obj.mixinThreeSignal.connect(obj.mixinThreeSlot)
        obj.mixinThreeSignal.connect(outsideSlot)
        obj.derivedSignal.emit('emit2')
        self.assertTrue(obj.derivedSlotCalled)
        self.assertFalse(obj.mixinTwoSlotCalled)
        self.assertFalse(obj.mixinThreeSlotCalled)
        self.assertFalse(self.outsideSlotCalled)
        obj.derivedSlotCalled = False

        # Check mixinThreeSignal emission
        obj.mixinThreeSignal.emit()
        self.assertTrue(self.outsideSlotCalled)
        self.assertTrue(obj.mixinThreeSlotCalled)
        self.assertFalse(obj.derivedSlotCalled)
        self.assertFalse(obj.mixinTwoSlotCalled)
        self.outsideSlotCalled = False
        obj.mixinThreeSlotCalled = False

        # Check mixinTwoSignal emission
        obj.mixinTwoSignal.connect(obj.mixinTwoSlot)
        obj.mixinTwoSignal.emit()
        self.assertTrue(obj.mixinTwoSlotCalled)
        self.assertFalse(obj.mixinThreeSlotCalled)
        self.assertFalse(obj.derivedSlotCalled)
        self.assertFalse(self.outsideSlotCalled)
        obj.mixinTwoSlotCalled = False

        # Check mixinSignal emission
        obj.mixinSignal.connect(outsideSlot)
        obj.mixinSignal.emit()
        self.assertTrue(self.outsideSlotCalled)
        self.assertFalse(obj.mixinTwoSlotCalled)
        self.assertFalse(obj.mixinThreeSlotCalled)
        self.assertFalse(obj.derivedSlotCalled)
        self.outsideSlotCalled = False

        # Check that method indices haven't changed.
        # Make sure to requery for the meta object, to check that a new one was not
        # created as a child of the old one.
        m = obj.metaObject()
        self.assertEqual(m.methodCount() - m.methodOffset(), 7)
        for i in range(m.methodOffset(), m.methodCount()):
            signature = m.method(i).methodSignature()
            self.assertEqual(methodIndices[signature], i)


if __name__ == '__main__':
    unittest.main()

