#############################################################################
##
## Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
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

from PySide2.QtCore import QObject, Signal, Slot

from websockettransport import WebSocketTransport


class WebSocketClientWrapper(QObject):
    """Wraps connected QWebSockets clients in WebSocketTransport objects.

       This code is all that is required to connect incoming WebSockets to
       the WebChannel. Any kind of remote JavaScript client that supports
       WebSockets can thus receive messages and access the published objects.
    """
    clientConnected = Signal(WebSocketTransport)

    def __init__(self, server, parent=None):
        """Construct the client wrapper with the given parent. All clients
           connecting to the QWebSocketServer will be automatically wrapped
           in WebSocketTransport objects."""
        super(WebSocketClientWrapper, self).__init__(parent)
        self._server = server
        self._server.newConnection.connect(self.handleNewConnection)
        self._transports = []

    @Slot()
    def handleNewConnection(self):
        """Wrap an incoming WebSocket connection in a WebSocketTransport
           object."""
        socket = self._server.nextPendingConnection()
        transport = WebSocketTransport(socket)
        self._transports.append(transport)
        self.clientConnected.emit(transport)
