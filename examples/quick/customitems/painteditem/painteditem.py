#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: http://www.qt.io/licensing/
##
## This file is part of the Qt for Python examples of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:BSD$
## You may use this file under the terms of the BSD license as follows:
##
## "Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
##   * Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in
##     the documentation and/or other materials provided with the
##     distribution.
##   * Neither the name of The Qt Company Ltd nor the names of its
##     contributors may be used to endorse or promote products derived
##     from this software without specific prior written permission.
##
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
##
## $QT_END_LICENSE$
##
#############################################################################

import sys

from PySide2.QtGui import QPainter, QBrush, QColor
from PySide2.QtWidgets import QApplication
from PySide2.QtQml import qmlRegisterType
from PySide2.QtCore import QUrl, Property, Signal, Qt, QPointF
from PySide2.QtQuick import QQuickPaintedItem, QQuickView


class TextBalloon(QQuickPaintedItem):

    rightAlignedChanged = Signal()

    def __init__(self, parent=None):
        self._rightAligned = False
        super().__init__(parent)

    @Property(bool, notify=rightAlignedChanged)
    def rightAligned(self):
        return self._rightAligned

    @rightAligned.setter
    def rightAlignedSet(self, value):
        self._rightAligned = value
        self.rightAlignedChanged.emit()

    def paint(self, painter: QPainter):

        brush = QBrush(QColor("#007430"))

        painter.setBrush(brush)
        painter.setPen(Qt.NoPen)
        painter.setRenderHint(QPainter.Antialiasing)

        itemSize = self.size()

        painter.drawRoundedRect(0, 0, itemSize.width(), itemSize.height() - 10, 10, 10)

        if self.rightAligned:
            points = [
                QPointF(itemSize.width() - 10.0, itemSize.height() - 10.0),
                QPointF(itemSize.width() - 20.0, itemSize.height()),
                QPointF(itemSize.width() - 30.0, itemSize.height() - 10.0),
            ]
        else:
            points = [
                QPointF(10.0, itemSize.height() - 10.0),
                QPointF(20.0, itemSize.height()),
                QPointF(30.0, itemSize.height() - 10.0),
            ]
        painter.drawConvexPolygon(points)


if __name__ == "__main__":

    app = QApplication(sys.argv)
    view = QQuickView()
    view.setResizeMode(QQuickView.SizeRootObjectToView)
    qmlRegisterType(TextBalloon, "TextBalloonPlugin", 1, 0, "TextBalloon")
    view.setSource(QUrl.fromLocalFile("main.qml"))

    if view.status() == QQuickView.Error:
        sys.exit(-1)
    view.show()

    sys.exit(app.exec_())
