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
from PySide2.QtCore import Qt, Slot, QSize
from PySide2.QtGui import QPainter
from PySide2.QtWidgets import (QAction, QApplication, QHeaderView, QHBoxLayout, QLabel, QLineEdit,
                               QMainWindow, QPushButton, QTableWidget, QTableWidgetItem,
                               QVBoxLayout, QWidget)
from PySide2.QtCharts import QtCharts

from __feature__ import snake_case, true_property


class Widget(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.items = 0

        # Example data
        self._data = {"Water": 24.5, "Electricity": 55.1, "Rent": 850.0,
                      "Supermarket": 230.4, "Internet": 29.99, "Bars": 21.85,
                      "Public transportation": 60.0, "Coffee": 22.45, "Restaurants": 120}

        # Left
        self.table = QTableWidget()
        self.table.column_count = 2
        self.table.horizontal_header_labels = ["Description", "Price"]
        self.table.horizontal_header().section_resize_mode = QHeaderView.Stretch

        # Chart
        self.chart_view = QtCharts.QChartView()
        self.chart_view.render_hint = QPainter.Antialiasing

        # Right
        self.description = QLineEdit()
        self.price = QLineEdit()
        self.add = QPushButton("Add")
        self.clear = QPushButton("Clear")
        self.quit = QPushButton("Quit")
        self.plot = QPushButton("Plot")

        # Disabling 'Add' button
        self.add.enabled = False

        self.right = QVBoxLayout()
        self.right.margin = 10
        self.right.add_widget(QLabel("Description"))
        self.right.add_widget(self.description)
        self.right.add_widget(QLabel("Price"))
        self.right.add_widget(self.price)
        self.right.add_widget(self.add)
        self.right.add_widget(self.plot)
        self.right.add_widget(self.chart_view)
        self.right.add_widget(self.clear)
        self.right.add_widget(self.quit)

        # QWidget Layout
        self.layout = QHBoxLayout()

        #self.table_view.setSizePolicy(size)
        self.layout.add_widget(self.table)
        self.layout.add_layout(self.right)

        # Set the layout to the QWidget
        self.set_layout(self.layout)

        # Signals and Slots
        self.add.clicked.connect(self.add_element)
        self.quit.clicked.connect(self.quit_application)
        self.plot.clicked.connect(self.plot_data)
        self.clear.clicked.connect(self.clear_table)
        self.description.textChanged[str].connect(self.check_disable)
        self.price.textChanged[str].connect(self.check_disable)

        # Fill example data
        self.fill_table()

    @Slot()
    def add_element(self):
        des = self.description.text
        price = self.price.text

        self.table.insert_row(self.items)
        description_item = QTableWidgetItem(des)
        price_item = QTableWidgetItem("{:.2f}".format(float(price)))
        price_item.text_alignment = Qt.AlignRight

        self.table.set_item(self.items, 0, description_item)
        self.table.set_item(self.items, 1, price_item)

        self.description.text = ""
        self.price.text = ""

        self.items += 1

    @Slot()
    def check_disable(self, s):
        if not self.description.text or not self.price.text:
            self.add.enabled = False
        else:
            self.add.enabled = True

    @Slot()
    def plot_data(self):
        # Get table information
        series = QtCharts.QPieSeries()
        for i in range(self.table.row_count):
            text = self.table.item(i, 0).text()
            number = float(self.table.item(i, 1).text())
            series.append(text, number)

        chart = QtCharts.QChart()
        chart.add_series(series)
        chart.legend().alignment = Qt.AlignLeft
        self.chart_view.set_chart(chart)

    @Slot()
    def quit_application(self):
        QApplication.quit()

    def fill_table(self, data=None):
        data = self._data if not data else data
        for desc, price in data.items():
            description_item = QTableWidgetItem(desc)
            price_item = QTableWidgetItem("{:.2f}".format(price))
            price_item.text_alignment = Qt.AlignRight
            self.table.insert_row(self.items)
            self.table.set_item(self.items, 0, description_item)
            self.table.set_item(self.items, 1, price_item)
            self.items += 1

    @Slot()
    def clear_table(self):
        self.table.row_count = 0
        self.items = 0


class MainWindow(QMainWindow):
    def __init__(self, widget):
        QMainWindow.__init__(self)
        self.window_title = "Tutorial"

        # Menu
        self.menu = self.menu_bar()
        self.file_menu = self.menu.add_menu("File")

        # Exit QAction
        exit_action = QAction("Exit", self)
        exit_action.shortcut = "Ctrl+Q"
        exit_action.triggered.connect(self.exit_app)

        self.file_menu.add_action(exit_action)
        self.set_central_widget(widget)

    @Slot()
    def exit_app(self, checked):
        QApplication.quit()


if __name__ == "__main__":
    # Qt Application
    app = QApplication(sys.argv)
    # QWidget
    widget = Widget()
    # QMainWindow using QWidget as central widget
    window = MainWindow(widget)
    window.size = QSize(800, 600)
    window.show()

    # Execute application
    sys.exit(app.exec_())
