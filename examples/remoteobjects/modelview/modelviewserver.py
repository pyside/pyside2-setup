#############################################################################
##
## Copyright (C) 2017 Ford Motor Company
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

"""PySide2 port of the remoteobjects/modelviewserver example from Qt v5.x"""

import sys

from PySide2.QtCore import (Qt, QByteArray, QModelIndex, QObject, QTimer, QUrl)
from PySide2.QtGui import (QColor, QStandardItemModel, QStandardItem)
from PySide2.QtWidgets import (QApplication, QTreeView)
from PySide2.QtRemoteObjects import QRemoteObjectHost, QRemoteObjectRegistryHost

class TimerHandler(QObject):
    def __init__(self, model):
        super(TimerHandler, self).__init__()
        self._model = model

    def change_data(self):
        for i in range(10, 50):
            self._model.setData(self._model.index(i, 1),
                                QColor(Qt.blue), Qt.BackgroundRole)

    def insert_data(self):
        self._model.insertRows(2, 9)
        for i in range(2, 11):
            self._model.setData(self._model.index(i, 1),
                                QColor(Qt.green), Qt.BackgroundRole)
            self._model.setData(self._model.index(i, 1),
                                "InsertedRow", Qt.DisplayRole)

    def remove_data(self):
        self._model.removeRows(2, 4)

    def change_flags(self):
        item = self._model.item(0, 0)
        item.setEnabled(False)
        item = item.child(0, 0)
        item.setFlags(item.flags() & Qt.ItemIsSelectable)

    def move_data(self):
        self._model.moveRows(QModelIndex(), 2, 4, QModelIndex(), 10)


def add_child(num_children, nesting_level):
    result = []
    if nesting_level == 0:
        return result
    for i in range(num_children):
        child = QStandardItem("Child num {}, nesting Level {}".format(i + 1,
                              nesting_level))
        if i == 0:
            child.appendRow(add_child(num_children, nesting_level -1))
        result.append(child)
    return result

if __name__ == '__main__':
    app = QApplication(sys.argv)
    model_size = 100000
    list = []
    source_model = QStandardItemModel()
    horizontal_header_list = ["First Column with spacing",
                              "Second Column with spacing"]
    source_model.setHorizontalHeaderLabels(horizontal_header_list)
    for i in range(model_size):
        first_item = QStandardItem("FancyTextNumber {}".format(i))
        if i == 0:
            first_item.appendRow(add_child(2, 2))
        second_item = QStandardItem("FancyRow2TextNumber {}".format(i))
        if i % 2 == 0:
            first_item.setBackground(Qt.red)
        row = [first_item, second_item]
        source_model.invisibleRootItem().appendRow(row)
        list.append("FancyTextNumber {}".format(i))

    # Needed by QMLModelViewClient
    role_names = {
        Qt.DisplayRole : QByteArray(b'_text'),
        Qt.BackgroundRole : QByteArray(b'_color')
    }
    source_model.setItemRoleNames(role_names)

    roles = [Qt.DisplayRole, Qt.BackgroundRole]

    print("Creating registry host")
    node = QRemoteObjectRegistryHost(QUrl("local:registry"))

    node2 = QRemoteObjectHost(QUrl("local:replica"), QUrl("local:registry"))
    node2.enableRemoting(source_model, "RemoteModel", roles)

    view = QTreeView()
    view.setWindowTitle("SourceView")
    view.setModel(source_model)
    view.show()
    handler = TimerHandler(source_model)
    QTimer.singleShot(5000, handler.change_data)
    QTimer.singleShot(10000, handler.insert_data)
    QTimer.singleShot(11000, handler.change_flags)
    QTimer.singleShot(12000, handler.remove_data)
    QTimer.singleShot(13000, handler.move_data)

    sys.exit(app.exec_())
