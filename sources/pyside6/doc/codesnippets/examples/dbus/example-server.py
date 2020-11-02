#!/usr/bin/env python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the documentation of Qt for Python.
##
## $QT_BEGIN_LICENSE:BSD$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## BSD License Usage
## Alternatively, you may use this file under the terms of the BSD license
## as follows:
##
## "Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
##   # Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##   # Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in
##     the documentation and/or other materials provided with the
##     distribution.
##   # Neither the name of The Qt Company Ltd nor the names of its
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

# -*- coding: utf-8 -*-

# DBUS Server Example of use PySide with PyDBus library

import dbus
import dbus.service
import dbus.mainloop.glib
import random

from PySide6.QtCore import *
from PySide6.QtGui import QPushButton, QApplication

# The adaptor, MUST inherit dbus.service.Object
class DBusWidget(dbus.service.Object):
    def __init__(self, name, session):
        # export this object to dbus
        dbus.service.Object.__init__(self, name, session)

        # create a simple widget
        self.widget = QPushButton()
        self.widget.resize(200, 50)

        # To export a Qt signal as a DBus-signal, you need to connect it to a method in this class.
        # The method MUST have the signal annotation, so python-dbus will export it as a dbus-signal
        QObject.connect(self.widget, SIGNAL("clicked()"), self.clicked)
        QObject.connect(QApplication.instance(), SIGNAL("lastWindowClosed()"), self.lastWindowClosed)

    # You can export methods to dbus like you do in python-dbus.
    @dbus.service.method("com.example.SampleWidget", in_signature='', out_signature='')
    def show(self):
        self.widget.show()

    # Another method... now with a parameter
    @dbus.service.method("com.example.SampleWidget", in_signature='s', out_signature='')
    def setText(self, value):
        self.widget.setText(value)

    # Another one...
    @dbus.service.method("com.example.SampleWidget", in_signature='', out_signature='')
    def exit(self):
        qApp().quit()

    # A signal that will be exported to dbus
    @dbus.service.signal("com.example.SampleWidget", signature='')
    def clicked(self):
        pass

    # Another signal that will be exported to dbus
    @dbus.service.signal("com.example.SampleWidget", signature='')
    def lastWindowClosed(self):
        pass


if __name__ == '__main__':
    app = QApplication([])
    # Use qt/glib mainloop integration to get dbus mainloop working
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    session_bus = dbus.SessionBus()
    # Export the service
    name = dbus.service.BusName("com.example.SampleService", session_bus)
    # Export the object
    widget = DBusWidget(session_bus, '/DBusWidget')

    print "Running example service."
    app.exec_()

