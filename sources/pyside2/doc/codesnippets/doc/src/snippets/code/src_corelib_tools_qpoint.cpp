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
p = QPoint()

p.setX(p.x() + 1)
p += QPoint(1, 0)
//! [0]


//! [1]
p = QPoint(1, 2)
#p.rx()--;   // p becomes (0, 2)
//! [1]


//! [2]
p = QPoint(1, 2)
#p.ry()++;   // p becomes (1, 3)
//! [2]


//! [3]
p = QPoint( 3, 7)
q = QPoint(-1, 4)
p += q    # p becomes (2, 11)
//! [3]


//! [4]
p = QPoint( 3, 7)
q = QPoint(-1, 4)
p -= q    # p becomes (4, 3)
//! [4]


//! [5]
p = QPoint(-1, 4)
p *= 2.5  # p becomes (-3, 10)
//! [5]


//! [6]
p = QPoint(-3, 10)
p /= 2.5  # p becomes (-1, 4)
//! [6]


//! [7]

class MyWidget(QWidget):

    self.oldPosition = QPointer()

    # event : QMouseEvent
    def mouseMoveEvent(QMouseEvent event):
        point = event.pos() - self.oldPosition
        if (point.manhattanLength() > 3):
            # the mouse has moved more than 3 pixels since the oldPosition
            pass
//! [7]


//! [8]
trueLength = sqrt(pow(x(), 2) + pow(y(), 2))
//! [8]


//! [9]
p = QPointF()

p.setX(p.x() + 1.0)
p += QPointF(1.0, 0.0)
#p.rx()++;
//! [9]


//! [10]
 p = QPointF(1.1, 2.5)
 #p.rx()--;   // p becomes (0.1, 2.5)
//! [10]


//! [11]
p = QPointF(1.1, 2.5)
#p.ry()++;   // p becomes (1.1, 3.5)
//! [11]


//! [12]
p = QPointF( 3.1, 7.1)
q = QPointF(-1.0, 4.1)
p += q    # p becomes (2.1, 11.2)
//! [12]


//! [13]
p = QPointF( 3.1, 7.1)
q = QPointF(-1.0, 4.1)
p -= q    # p becomes (4.1, 3.0)
//! [13]


//! [14]
p = QPointF(-1.1, 4.1)
p *= 2.5  # p becomes (-2.75, 10.25)
//! [14]


//! [15]
p = QPointF(-2.75, 10.25)
p /= 2.5  # p becomes (-1.1, 4.1)
//! [15]
