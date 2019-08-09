#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import sys
from PySide2.QtCore import QObject, QThread, Signal, Slot
from PySide2.QtWidgets import QApplication, QPushButton, QVBoxLayout, QWidget


# Create a basic window with a layout and a button
class MainForm(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.setWindowTitle("My Form")
        self.layout = QVBoxLayout()
        self.button = QPushButton("Click me!")
        self.button.clicked.connect(self.start_thread)
        self.layout.addWidget(self.button)
        self.setLayout(self.layout)

    # Instantiate and start a new thread
    def start_thread(self):
        instanced_thread = WorkerThread(self)
        instanced_thread.start()

    # Create the Slots that will receive signals
    @Slot(str)
    def update_str_field(self, message):
        print(message)

    @Slot(int)
    def update_int_field(self, value):
        print(value)


# Signals must inherit QObject
class MySignals(QObject):
    signal_str = Signal(str)
    signal_int = Signal(int)


# Create the Worker Thread
class WorkerThread(QThread):
    def __init__(self, parent=None):
        QThread.__init__(self, parent)
        # Instantiate signals and connect signals to the slots
        self.signals = MySignals()
        self.signals.signal_str.connect(parent.update_str_field)
        self.signals.signal_int.connect(parent.update_int_field)

    def run(self):
        # Do something on the worker thread
        a = 1 + 1
        # Emit signals whenever you want
        self.signals.signal_int.emit(a)
        self.signals.signal_str.emit("This text comes to Main thread from our Worker thread.")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainForm()
    window.show()
    sys.exit(app.exec_())
