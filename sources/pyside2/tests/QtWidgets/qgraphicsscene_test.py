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

'''Basic test cases for QGraphicsScene'''

import unittest
import gc

from PySide2.QtCore import QPointF
from PySide2.QtGui import QPolygonF, QPixmap, QPainterPath, QTransform, QWindow
from PySide2.QtWidgets import QApplication, QPushButton
from PySide2.QtWidgets import QGraphicsScene
from PySide2.QtWidgets import QGraphicsEllipseItem, QGraphicsLineItem
from PySide2.QtWidgets import QGraphicsPathItem, QGraphicsPixmapItem
from PySide2.QtWidgets import QGraphicsPolygonItem, QGraphicsRectItem
from PySide2.QtWidgets import QGraphicsSimpleTextItem, QGraphicsTextItem
from PySide2.QtWidgets import QGraphicsProxyWidget, QGraphicsView

from helper import UsesQApplication

class Constructor(unittest.TestCase):
    '''QGraphicsScene constructor'''

    def testConstructor(self):
        #QGraphicsScene constructor
        obj = QGraphicsScene()
        self.assertTrue(isinstance(obj, QGraphicsScene))

# Test for PYSIDE-868: Test whether painter.device() can be accessed
# correctly. This was crashing when the underlying QPaintDevice was a
# QWidget due to handling multiple inheritance incorrectly.
class CustomScene(QGraphicsScene):
    def __init__(self, parent = None):
        super(CustomScene, self).__init__(parent)
        self.dpi = 0

    def drawBackground(self, painter, rect):
        self.dpi = painter.device().physicalDpiX()

    def drawForeground(self, painter, rect):
        self.dpi = painter.device().physicalDpiX()

class ConstructorWithRect(unittest.TestCase):
    '''QGraphicsScene qrect constructor and related sizes'''

    def setUp(self):
        #Acquire resources
        # PyQt4 doesn't accept a QRect as argument to constructor
        self.scene = QGraphicsScene(0, 200, 150, 175)

    def tearDown(self):
        #Release resources
        del self.scene

    def testHeight(self):
        #QGraphicsScene.height()
        self.assertEqual(self.scene.height(), 175)

    def testWidth(self):
        #QGraphicsScene.width()
        self.assertEqual(self.scene.width(), 150)


class AddItem(UsesQApplication):
    '''Tests for QGraphicsScene.add*'''

    qapplication = True

    def setUp(self):
        #Acquire resources
        super(AddItem, self).setUp()
        self.scene = QGraphicsScene()
        # While the scene does not inherits from QWidget, requires
        # an application to make the internals work.

    def tearDown(self):
        #Release resources
        del self.scene
        super(AddItem, self).tearDown()

    def testEllipse(self):
        #QGraphicsScene.addEllipse
        item = self.scene.addEllipse(100, 100, 100, 100)
        self.assertTrue(isinstance(item, QGraphicsEllipseItem))

    def testLine(self):
        #QGraphicsScene.addLine
        item = self.scene.addLine(100, 100, 200, 200)
        self.assertTrue(isinstance(item, QGraphicsLineItem))

    def testPath(self):
        #QGraphicsScene.addPath
        item = self.scene.addPath(QPainterPath())
        self.assertTrue(isinstance(item, QGraphicsPathItem))

    def testPixmap(self):
        #QGraphicsScene.addPixmap
        item = self.scene.addPixmap(QPixmap())
        self.assertTrue(isinstance(item, QGraphicsPixmapItem))

    def testPolygon(self):
        #QGraphicsScene.addPolygon
        points = [QPointF(0, 0), QPointF(100, 100), QPointF(0, 100)]
        item = self.scene.addPolygon(QPolygonF(points))
        self.assertTrue(isinstance(item, QGraphicsPolygonItem))

    def testRect(self):
        #QGraphicsScene.addRect
        item = self.scene.addRect(100, 100, 100, 100)
        self.assertTrue(isinstance(item, QGraphicsRectItem))

    def testSimpleText(self):
        #QGraphicsScene.addSimpleText
        item = self.scene.addSimpleText('Monty Python 42')
        self.assertTrue(isinstance(item, QGraphicsSimpleTextItem))

    def testText(self):
        #QGraphicsScene.addText
        item = self.scene.addText('Monty Python 42')
        self.assertTrue(isinstance(item, QGraphicsTextItem))

    def testWidget(self):
        #QGraphicsScene.addWidget
        # XXX: printing some X11 error when using under PyQt4
        item = self.scene.addWidget(QPushButton())
        self.assertTrue(isinstance(item, QGraphicsProxyWidget))


class ItemRetrieve(UsesQApplication):
    '''Tests for QGraphicsScene item retrieval methods'''

    qapplication = True

    def setUp(self):
        #Acquire resources
        super(ItemRetrieve, self).setUp()
        self.scene = QGraphicsScene()

        self.topleft = QGraphicsRectItem(0, 0, 100, 100)
        self.topright = QGraphicsRectItem(100, 0, 100, 100)
        self.bottomleft = QGraphicsRectItem(0, 100, 100, 100)
        self.bottomright = QGraphicsRectItem(100, 100, 100, 100)

        self.items = [self.topleft, self.topright, self.bottomleft,
                        self.bottomright]

        for item in self.items:
            self.scene.addItem(item)

    def tearDown(self):
        #Release resources
        del self.scene
        super(ItemRetrieve, self).tearDown()

    def testItems(self):
        #QGraphicsScene.items()
        items = self.scene.items()
        for i in items:
            self.assertTrue(i in self.items)

    def testItemAt(self):
        #QGraphicsScene.itemAt()
        self.assertEqual(self.scene.itemAt(50, 50, QTransform()), self.topleft)
        self.assertEqual(self.scene.itemAt(150, 50, QTransform()), self.topright)
        self.assertEqual(self.scene.itemAt(50, 150, QTransform()), self.bottomleft)
        self.assertEqual(self.scene.itemAt(150, 150, QTransform()), self.bottomright)

class TestGraphicsGroup(UsesQApplication):
    def testIt(self):
        scene = QGraphicsScene()
        i1 = QGraphicsRectItem()
        scene.addItem(i1)
        i2 = QGraphicsRectItem(i1)
        i3 = QGraphicsRectItem()
        i4 = QGraphicsRectItem()
        group = scene.createItemGroup((i2, i3, i4))
        scene.removeItem(i1)
        del i1 # this shouldn't delete i2
        self.assertEqual(i2.scene(), scene)
        scene.destroyItemGroup(group)
        self.assertRaises(RuntimeError, group.type)

    def testCustomScene(self): # For PYSIDE-868, see above
        scene = CustomScene()
        view = QGraphicsView(scene)
        view.show()
        while scene.dpi == 0:
           QApplication.instance().processEvents()
        view.hide()


if __name__ == '__main__':
    unittest.main()
