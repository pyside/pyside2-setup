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

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import QObject, Signal, Property, Slot

'''Tests that the signal notify id of a property is correct, aka corresponds to the initially set
notify method.'''

class Foo(QObject):
    def __init__(self):
        QObject.__init__(self)
        self._prop = "Empty"

    def getProp(self):
        return self._prop

    def setProp(self, value):
        if value != self._prop:
            self._prop = value
            self.propChanged.emit()

    # Inside the dynamic QMetaObject, the methods have to be sorted, so that this slot comes
    # after any signals. That means the property notify id has to be updated, to have the correct
    # relative method id.
    @Slot()
    def randomSlot():
        pass

    propChanged = Signal()
    prop = Property(str, getProp, setProp, notify=propChanged)

class NotifyIdSignal(unittest.TestCase):
    def setUp(self):
        self.obj = Foo()

    def tearDown(self):
        del self.obj

    def testSignalEmission(self):
        metaObject = self.obj.metaObject()
        propertyIndex = metaObject.indexOfProperty("prop")
        property = metaObject.property(propertyIndex)

        signalIndex = property.notifySignalIndex()
        signal = metaObject.method(signalIndex)
        signalName = signal.name()
        self.assertEqual(signalName, "propChanged")

if __name__ == '__main__':
    unittest.main()
