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

import sys

from PySide6.QtGui import QStandardItem, QStandardItemModel
from PySide6.QtWidgets import QMainWindow, QTreeView, QAbstractItemView, QApplication, QMessageBox

class Item(QStandardItem):
    def __init__(self, text):
        super(Item, self).__init__()
        self.setText(text)
        self.setDragEnabled(True)
        self.setDropEnabled(True)

    def clone(self):
        ret = Item(self.text())
        return ret

class Project(QStandardItemModel):
    def __init__(self):
        super(Project, self).__init__()
        self.setItemPrototype(Item("Prototype"))
        # add some items so we have stuff to move around
        self.appendRow(Item("ABC"))
        self.appendRow(Item("DEF"))
        self.appendRow(Item("GHI"))

class MainWindow(QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()

        self.model = Project()
        self.view = QTreeView(self)
        self.view.setModel(self.model)
        self.view.setDragEnabled(True)
        self.view.setDragDropMode(QAbstractItemView.InternalMove)
        self.setCentralWidget(self.view)

    def mousePressEvent(self, e):
        print(e.x(), e.y())
        return QMainWindow.mousePressEvent(self, e)

def main():
    app = QApplication(sys.argv)
    w = MainWindow()
    w.show()
    QMessageBox.information(None, "Info", "Just drag and drop the items.")
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
