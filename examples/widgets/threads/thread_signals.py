
#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
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
