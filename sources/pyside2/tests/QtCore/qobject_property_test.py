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

'''Test cases for QObject property and setProperty'''

import unittest

from PySide2.QtCore import QObject, Property, Signal

class MyObjectWithNotifyProperty(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self.p = 0

    def readP(self):
        return self.p

    def writeP(self, v):
        self.p = v
        self.notifyP.emit()

    notifyP = Signal()
    myProperty = Property(int, readP, fset=writeP, notify=notifyP)

class PropertyWithNotify(unittest.TestCase):
    def called(self):
        self.called_ = True

    def testNotify(self):
        self.called_ = False
        obj = MyObjectWithNotifyProperty()
        obj.notifyP.connect(self.called)
        obj.myProperty = 10
        self.assertTrue(self.called_)

    def testHasProperty(self):
        o = MyObjectWithNotifyProperty()
        o.setProperty("myProperty", 10)
        self.assertEqual(o.myProperty, 10)
        self.assertEqual(o.property("myProperty"), 10)


if __name__ == '__main__':
    unittest.main()
