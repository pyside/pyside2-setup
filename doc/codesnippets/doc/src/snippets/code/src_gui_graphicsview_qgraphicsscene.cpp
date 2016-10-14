/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of PySide2.
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
scene = QGraphicsScene()
scene.addText("Hello, world!")

view = QGraphicsView(&scene)
view.show()
//! [0]


//! [1]
scene = QGraphicsScene
scene.addItem(...
...
printer = QPrinter(QPrinter.HighResolution)
printer.setPaperSize(QPrinter.A4)

painter = QPainter(printer)
scene.render(&painter)
//! [1]


//! [2]
segmentSize = sceneRect().size() / math.pow(2, depth - 1)
//! [2]


//! [3]
scene = QGraphicsScene()
view = QGraphicsView(scene)
view.show()

# a blue background
scene.setBackgroundBrush(Qt.blue)

# a gradient background
gradient = QRadialGradient(0, 0, 10)
gradient.setSpread(QGradient.RepeatSpread)
scene.setBackgroundBrush(gradient)
//! [3]


//! [4]
scene = QGraphicsScene()
view = QGraphicsView(scene)
view.show()

# a white semi-transparent foreground
scene.setForegroundBrush(QColor(255, 255, 255, 127))

# a grid foreground
scene.setForegroundBrush(QBrush(Qt.lightGray, Qt.CrossPattern))
//! [4]


//! [5]
class TileScene (QGraphicsScene):
    # ...
    def rectForTile(x, y):
        # Return the rectangle for the tile at position (x, y).
        return QRectF(x * self.tileWidth, y * self.tileHeight, self.tileWidth, self.tileHeight)

    def setTile(x, y, pixmap):
        # Sets or replaces the tile at position (x, y) with pixmap.
        if x >= 0 && x < self.numTilesH && y >= 0 && y < self.numTilesV:
            self.tiles[y][x] = pixmap
            invalidate(rectForTile(x, y), BackgroundLayer)

    def drawBackground(painter, exposed):
        # Draws all tiles that intersect the exposed area.
        for y in range(0, self.numTilesV:
            for x in range(0, self.numTilesH:
                rect = rectForTile(x, y)
                if exposed.intersects(rect):
                    painter.drawPixmap(rect.topLeft(), tiles[y][x])
//! [5]
