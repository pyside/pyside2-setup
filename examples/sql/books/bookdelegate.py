#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

import copy
from PySide2.QtSql import QSqlRelationalDelegate
from PySide2.QtWidgets import QSpinBox, QStyle
from PySide2.QtGui import QPixmap, QPalette
from PySide2.QtCore import QEvent, QSize, Qt


class BookDelegate(QSqlRelationalDelegate):
    """Books delegate to rate the books"""

    def __init__(self, parent=None):
        QSqlRelationalDelegate.__init__(self, parent)
        self.star = QPixmap(":/images/star.png")

    def paint(self, painter, option, index):
        """ Paint the items in the table.

            If the item referred to by <index> is a StarRating, we
            handle the painting ourselves. For the other items, we
            let the base class handle the painting as usual.

            In a polished application, we'd use a better check than
            the column number to find out if we needed to paint the
            stars, but it works for the purposes of this example.
        """
        if index.column() != 5:
            # Since we draw the grid ourselves:
            opt = copy.copy(option)
            opt.rect = option.rect.adjusted(0, 0, -1, -1)
            QSqlRelationalDelegate.paint(self, painter, opt, index)
        else:
            model = index.model()
            if option.state & QStyle.State_Enabled:
                if option.state & QStyle.State_Active:
                    color_group = QPalette.Normal
                else:
                    color_group = QPalette.Inactive
            else:
                color_group = QPalette.Disabled

            if option.state & QStyle.State_Selected:
                painter.fillRect(option.rect,
                    option.palette.color(color_group, QPalette.Highlight))
            rating = model.data(index, Qt.DisplayRole)
            width = self.star.width()
            height = self.star.height()
            x = option.rect.x()
            y = option.rect.y() + (option.rect.height() / 2) - (height / 2)
            for i in range(rating):
                painter.drawPixmap(x, y, self.star)
                x += width

            # Since we draw the grid ourselves:
            self.drawFocus(painter, option, option.rect.adjusted(0, 0, -1, -1))

        pen = painter.pen()
        painter.setPen(option.palette.color(QPalette.Mid))
        painter.drawLine(option.rect.bottomLeft(), option.rect.bottomRight())
        painter.drawLine(option.rect.topRight(), option.rect.bottomRight())
        painter.setPen(pen)

    def sizeHint(self, option, index):
        """ Returns the size needed to display the item in a QSize object. """
        if index.column() == 5:
            size_hint = QSize(5 * self.star.width(), self.star.height()) + QSize(1, 1)
            return size_hint
        # Since we draw the grid ourselves:
        return QSqlRelationalDelegate.sizeHint(self, option, index) + QSize(1, 1)

    def editorEvent(self, event, model, option, index):
        if index.column() != 5:
            return False

        if event.type() == QEvent.MouseButtonPress:
            mouse_pos = event.pos()
            new_stars = int(0.7 + (mouse_pos.x() - option.rect.x()) / self.star.width())
            stars = max(0, min(new_stars, 5))
            model.setData(index, stars)
            # So that the selection can change
            return False

        return True

    def createEditor(self, parent, option, index):
        if index.column() != 4:
            return QSqlRelationalDelegate.createEditor(self, parent, option, index)

        # For editing the year, return a spinbox with a range from -1000 to 2100.
        spinbox = QSpinBox(parent)
        spinbox.setFrame(False)
        spinbox.setMaximum(2100)
        spinbox.setMinimum(-1000)
        return spinbox
