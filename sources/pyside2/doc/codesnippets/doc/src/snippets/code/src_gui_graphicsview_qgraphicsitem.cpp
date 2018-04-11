/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt for Python project.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
class SimpleItem(QGraphicsItem):

    def boundingRect(self):
        penWidth = 1.0
        return QRectF(-10 - penWidth / 2, -10 - penWidth / 2,
                      20 + penWidth, 20 + penWidth)

    def paint(self, painter, option, widget):
        painter.drawRoundedRect(-10, -10, 20, 20, 5, 5)
//! [0]


//! [1]
class CustomItem(QGraphicsItem):
   ...
   self.Type = QGraphicsItem.UserType + 1

   def type(self):
       # Enable the use of qgraphicsitem_cast with this item.
       return self.Type
   ...

//! [1]


//! [2]
item.setCursor(Qt.IBeamCursor)
//! [2]


//! [3]
item.setCursor(Qt.IBeamCursor)
//! [3]


//! [4]
rect = QGraphicsRectItem()
rect.setPos(100, 100)

rect.sceneTransform().map(QPointF(0, 0))
# returns QPointF(100, 100)

rect.sceneTransform().inverted().map(QPointF(100, 100))
# returns QPointF(0, 0);
//! [4]


//! [5]
rect = QGraphicsRectItem()
rect.setPos(100, 100)

rect.deviceTransform(view.viewportTransform()).map(QPointF(0, 0))
# returns the item's (0, 0) point in view's viewport coordinates

rect.deviceTransform(view.viewportTransform()).inverted().map(QPointF(100, 100))
# returns view's viewport's (100, 100) coordinate in item coordinates
//! [5]


//! [6]
# Rotate an item 45 degrees around (0, 0)
item.rotate(45)

# Rotate an item 45 degrees around (x, y)
item.setTransform(QTransform().translate(x, y).rotate(45).translate(-x, -y))
//! [6]


//! [7]
# Scale an item by 3x2 from its origin
item.scale(3, 2)

# Scale an item by 3x2 from (x, y)
item.setTransform(QTransform().translate(x, y).scale(3, 2).translate(-x, -y))
//! [7]


//! [8]
def boundingRect(self):
    penWidth = 1.0
    return QRectF(-radius - penWidth / 2, -radius - penWidth / 2,
                  diameter + penWidth, diameter + penWidth)
//! [8]


//! [9]
def shape(self):
    path = QPainterPath()
    path.addEllipse(boundingRect())
    return path
//! [9]


//! [10]
def paint(self, painter, option, widget):
    painter.drawRoundedRect(-10, -10, 20, 20, 5, 5)
//! [10]


//! [11]
ObjectName = 0;

item = scene.itemAt(100, 50)
if len(item.data(ObjectName)) == 0:
    if isinstance(ButtonItem, item):
        item.setData(ObjectName, "Button")
//! [11]


//! [12]
scene = QGraphicsScene()
ellipse = scene.addEllipse(QRectF(-10, -10, 20, 20))
line = scene.addLine(QLineF(-10, -10, 20, 20))

line.installSceneEventFilter(ellipse)
# line's events are filtered by ellipse's sceneEventFilter() function.

ellipse.installSceneEventFilter(line)
# ellipse's events are filtered by line's sceneEventFilter() function.
//! [12]


//! [13]
def contextMenuEvent(self, event):
    menu = QMenu()
    removeAction = menu.addAction("Remove")
    markAction = menu.addAction("Mark")
    selectedAction = menu.exec(event.screenPos())
    // ...
//! [13]


//! [14]
def __init__(self):
    self.setAcceptDrops(true)
    ...

def dragEnterEvent(self, event):
    event.setAccepted(event.mimeData().hasFormat("text/plain"))
//! [14]


//! [15]
def itemChange(self, change, value):
    if change == ItemPositionChange && scene():
        # value is the new position.
        rect = scene().sceneRect()
        if !rect.contains(value):
            # Keep the item inside the scene rect.
            value.setX(qMin(rect.right(), qMax(value.x(), rect.left())))
            value.setY(qMin(rect.bottom(), qMax(value.y(), rect.top())))
            return value
    return QGraphicsItem.itemChange(self, change, value)
//! [15]


//! [16]
def setRadius(self, newRadius):
    if radius != newRadius:
        prepareGeometryChange()
        radius = newRadius
//! [16]


//! [17]
# Group all selected items together
group = scene.createItemGroup(scene.selecteditems())

# Destroy the group, and delete the group item
scene.destroyItemGroup(group)
//! [17]


//! [QGraphicsItem type]
class CustomItem(QGraphicsItem):
    ...
    self.Type = QGraphicsItem.UserType + 1

    def type(self):
       # Enable the use of qgraphicsitem_cast with this item.
       return self.Type
    ...
//! [QGraphicsItem type]

//! [18]
class QGraphicsPathItem (QAbstractGraphicsShapeItem):
    Type = 2

    def type(self):
        return QGraphicsPathItem.Type
# ...
//! [18]

//! [19]
xform = item.deviceTransform(view.viewportTransform())
deviceRect = xform.mapRect(rect).toAlignedRect()
view.viewport().scroll(dx, dy, deviceRect)
//! [19]
