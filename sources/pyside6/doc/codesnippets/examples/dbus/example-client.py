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

# DBUS Client using PySide integration

import sys
from traceback import print_exc

# import python dbus module
import dbus
# import python dbus GLib mainloop support
import dbus.mainloop.glib
# import QtCore
from PySide6.QtCore import *

# signal handler
def button_clicked():
    print("button clicked")

# main function
if __name__ == '__main__':

    # Enable glib main loop support
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    # Get the session bus
    bus = dbus.SessionBus()

    try:
        # Get the remote object
        remote_object = bus.get_object("com.example.SampleService",
                                       "/DBusWidget")
        # Get the remote interface for the remote object
        iface = dbus.Interface(remote_object, "com.example.SampleWidget")
    except dbus.DBusException:
        print_exc()
        sys.exit(1)

    # Start the application
    app = QCoreApplication([])

    # Call some methods of the remote interface
    iface.show()
    iface.setText("Emit signal")
    # connect the DBus signal clicked to the function button_clicked
    iface.connect_to_signal("clicked", button_clicked)
    iface.connect_to_signal("lastWindowClosed", app.quit)

    # enter in the main loop
    app.exec_()
