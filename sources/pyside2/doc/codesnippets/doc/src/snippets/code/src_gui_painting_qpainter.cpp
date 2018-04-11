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
def paintEvent(self, paintEvent):
    painter = QPainter(self)
    painter.setPen(Qt.blue)
    painter.setFont(QFont("Arial", 30))
    painter.drawText(rect(), Qt.AlignCenter, "Qt")
//! [0]


//! [1]
def paintEvent(self, paintEvent):
    p = QPainter()
    p.begin(self)
    p.drawLine(...)         # drawing code
    p.end()
//! [1]


//! [2]
self paintEvent(self, paintEvent):
    p = QPainter(self)
    p.drawLine(...)         # drawing code

//! [2]


//! [3]
painter.begin(0)  # impossible - paint device cannot be 0

image = QPixmap(0, 0)
painter.begin(&image) # impossible - image.isNull() == true

painter.begin(myWidget)
painter2.begin(myWidget) # impossible - only one painter at a time
//! [3]


//! [4]
def rotate(self, angle):
    matrix = QMatrix()
    matrix.rotate(angle)
    setWorldMatrix(matrix, true)
//! [4]


//! [5]
path = QPainterPath()
path.moveTo(20, 80)
path.lineTo(20, 30)
path.cubicTo(80, 0, 50, 50, 80, 80)

painter = QPainter(self)
painter.drawPath(path)
//! [5]


//! [6]
line = QLineF(10.0, 80.0, 90.0, 20.0)

painter = QPainter(self)
painter.drawLine(line)
//! [6]


//! [7]
rectangle = QRectF(10.0, 20.0, 80.0, 60.0)

painter = QPainter(self)
painter.drawRect(rectangle)
//! [7]


//! [8]
rectangle = QRectF(10.0, 20.0, 80.0, 60.0)

painter = QPainter(self)
painter.drawRoundedRect(rectangle, 20.0, 15.0)
//! [8]


//! [9]
rectangle = QRectF(10.0, 20.0, 80.0, 60.0)

painter = QPainter(self)
painter.drawEllipse(rectangle)
//! [9]


//! [10]
rectangle = QRectF(10.0, 20.0, 80.0, 60.0)
startAngle = 30 * 16
spanAngle = 120 * 16

painter = QPainter(self)
painter.drawArc(rectangle, startAngle, spanAngle)
//! [10]


//! [11]
rectangle = QRectF(10.0, 20.0, 80.0, 60.0)
startAngle = 30 * 16
spanAngle = 120 * 16

painter = QPainter(self)
painter.drawPie(rectangle, startAngle, spanAngle)
//! [11]


//! [12]
rectangle = QRectF(10.0, 20.0, 80.0, 60.0)
startAngle = 30 * 16
spanAngle = 120 * 16

painter = QPainter(self)
painter.drawChord(rect, startAngle, spanAngle)
//! [12]


//! [13]
points = [
    QPointF(10.0, 80.0),
    QPointF(20.0, 10.0),
    QPointF(80.0, 30.0),
]

painter = QPainter(self)
painter.drawPolyline(points, 3)
//! [13]


//! [14]
points = [
    QPointF(10.0, 80.0),
    QPointF(20.0, 10.0),
    QPointF(80.0, 30.0),
    QPointF(90.0, 70.0)
]

painter = QPainter(self)
painter.drawPolygon(points, 4)
//! [14]


//! [15]
points = [
    QPointF(10.0, 80.0),
    QPointF(20.0, 10.0),
    QPointF(80.0, 30.0),
    QPointF(90.0, 70.0)
]

painter = QPainter(self)
painter.drawConvexPolygon(points, 4)
//! [15]


//! [16]
target = QRectF(10.0, 20.0, 80.0, 60.0)
source = QRectF(0.0, 0.0, 70.0, 40.0)
pixmap = QPixmap(":myPixmap.png")

painter = QPainter(self)
painter.drawPixmap(target, image, source)
//! [16]


//! [17]
painter = QPainter(self)
painter.drawText(rect, Qt.AlignCenter, tr("Qt\nProject"))
//! [17]


//! [18]
picture = QPicture()
point = QPointF(10.0, 20.0)
picture.load("drawing.pic")

painter = QPainter(self)
painter.drawPicture(0, 0, picture)
//! [18]


//! [19]
fillRect(rectangle, background())
//! [19]


//! [20]
target = QRectF(10.0, 20.0, 80.0, 60.0)
source = QRectF(0.0, 0.0, 70.0, 40.0)
image = QImage(":/images/myImage.png")

painter = QPainter(self)
painter.drawImage(target, image, source)
//! [20]

//! [21]
painter = QPainter(self)
painter.fillRect(0, 0, 128, 128, Qt.green)
painter.beginNativePainting()

glEnable(GL_SCISSOR_TEST)
glScissor(0, 0, 64, 64)

glClearColor(1, 0, 0, 1)
glClear(GL_COLOR_BUFFER_BIT)

glDisable(GL_SCISSOR_TEST)

painter.endNativePainting()
//! [21]
