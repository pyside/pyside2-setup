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

from PySide2.QtCore import QObject, Property, QCoreApplication
from PySide2.QtScript import  QScriptEngine

class MyObject(QObject):
    def __init__(self, parent = None):
        QObject.__init__(self, parent)
        self._p = 100

    def setX(self, value):
        self._p = value

    def getX(self):
        return self._p

    def resetX(self):
        self._p = 100

    def delX(self):
        self._p = 0

    x = Property(int, getX, setX, resetX, delX)


class QPropertyTest(unittest.TestCase):

    def testSimple(self):
        o = MyObject()
        self.assertEqual(o.x, 100)
        o.x = 42
        self.assertEqual(o.x, 42)

    def testHasProperty(self):
        o = MyObject()
        o.setProperty("x", 10)
        self.assertEqual(o.x, 10)
        self.assertEqual(o.property("x"), 10)

    def testMetaProperty(self):
        o = MyObject()
        m = o.metaObject()
        found = False
        for i in range(m.propertyCount()):
            mp = m.property(i)
            if mp.name() == "x":
                found = True
                break
        self.assertTrue(found)

    def testScriptQProperty(self):
        qapp = QCoreApplication([])
        myEngine = QScriptEngine()
        obj = MyObject()
        scriptObj = myEngine.newQObject(obj)
        myEngine.globalObject().setProperty("obj", scriptObj)
        myEngine.evaluate("obj.x = 42")
        self.assertEqual(scriptObj.property("x").toInt32(), 42)
        self.assertEqual(obj.property("x"), 42)

if __name__ == '__main__':
    unittest.main()
