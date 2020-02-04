#############################################################################
##
## Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
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


import os
import sys

from PySide2.QtWidgets import QApplication
from PySide2.QtGui import QDesktopServices
from PySide2.QtNetwork import QHostAddress, QSslSocket
from PySide2.QtCore import (QFile, QFileInfo, QUrl)
from PySide2.QtWebChannel import QWebChannel
from PySide2.QtWebSockets import QWebSocketServer

from dialog import Dialog
from core import Core
from websocketclientwrapper import WebSocketClientWrapper


if __name__ == '__main__':
    app = QApplication(sys.argv)
    if not QSslSocket.supportsSsl():
        print('The example requires SSL support.')
        sys.exit(-1)
    cur_dir = os.path.dirname(os.path.abspath(__file__))
    jsFileInfo = QFileInfo(cur_dir + "/qwebchannel.js")
    if not jsFileInfo.exists():
        QFile.copy(":/qtwebchannel/qwebchannel.js",
                   jsFileInfo.absoluteFilePath())

    # setup the QWebSocketServer
    server = QWebSocketServer("QWebChannel Standalone Example Server",
                              QWebSocketServer.NonSecureMode)
    if not server.listen(QHostAddress.LocalHost, 12345):
        print("Failed to open web socket server.")
        sys.exit(-1)

    # wrap WebSocket clients in QWebChannelAbstractTransport objects
    clientWrapper = WebSocketClientWrapper(server)

    # setup the channel
    channel = QWebChannel()
    clientWrapper.clientConnected.connect(channel.connectTo)

    # setup the UI
    dialog = Dialog()

    # setup the core and publish it to the QWebChannel
    core = Core(dialog)
    channel.registerObject("core", core)

    # open a browser window with the client HTML page
    url = QUrl.fromLocalFile(cur_dir + "/index.html")
    QDesktopServices.openUrl(url)

    message = "Initialization complete, opening browser at {}.".format(
              url.toDisplayString())
    dialog.displayMessage(message)
    dialog.show()

    sys.exit(app.exec_())
