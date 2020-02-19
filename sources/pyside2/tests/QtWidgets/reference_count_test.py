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

'''Test cases for Reference count when the object is created in c++ side'''

import gc
import os
import sys
import unittest
import weakref

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import Qt, QPointF
from PySide2.QtGui import QPolygonF
from PySide2.QtWidgets import QApplication, QGraphicsScene, QGraphicsRectItem, QGraphicsPolygonItem, QGraphicsRectItem

from helper.usesqapplication import UsesQApplication

destroyedRect = False
destroyedPol = False

def rect_del(o):
    global destroyedRect
    destroyedRect = True

def pol_del(o):
    global destroyedPol
    destroyedPol = True

class ReferenceCount(UsesQApplication):

    def setUp(self):
        super(ReferenceCount, self).setUp()
        self.scene = QGraphicsScene()

    def tearDown(self):
        super(ReferenceCount, self).tearDown()

    def beforeTest(self):
        points = [QPointF(0, 0), QPointF(100, 100), QPointF(0, 100)]
        pol = self.scene.addPolygon(QPolygonF(points))
        self.assertTrue(isinstance(pol, QGraphicsPolygonItem))
        self.wrp = weakref.ref(pol, pol_del)

        #refcount need be 3 because one ref for QGraphicsScene, and one to rect obj
        self.assertEqual(sys.getrefcount(pol), 3)

    def testReferenceCount(self):
        global destroyedRect
        global destroyedPol

        self.beforeTest()

        rect = self.scene.addRect(10.0, 10.0, 10.0, 10.0)
        self.assertTrue(isinstance(rect, QGraphicsRectItem))

        self.wrr = weakref.ref(rect, rect_del)

        #refcount need be 3 because one ref for QGraphicsScene, and one to rect obj
        self.assertEqual(sys.getrefcount(rect), 3)

        del rect
        #not destroyed because one ref continue in QGraphicsScene
        self.assertEqual(destroyedRect, False)
        self.assertEqual(destroyedPol, False)

        del self.scene

        #QGraphicsScene was destroyed and this destroy internal ref to rect
        self.assertEqual(destroyedRect, True)
        self.assertEqual(destroyedPol, True)

if __name__ == '__main__':
    unittest.main()
