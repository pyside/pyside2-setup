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

from PySide2.QtWebChannel import QWebChannelAbstractTransport
from PySide2.QtCore import QByteArray, QJsonDocument, Slot


class WebSocketTransport(QWebChannelAbstractTransport):
    """QWebChannelAbstractSocket implementation using a QWebSocket internally.

        The transport delegates all messages received over the QWebSocket over
        its textMessageReceived signal. Analogously, all calls to
        sendTextMessage will be sent over the QWebSocket to the remote client.
    """

    def __init__(self, socket):
        """Construct the transport object and wrap the given socket.
           The socket is also set as the parent of the transport object."""
        super(WebSocketTransport, self).__init__(socket)
        self._socket = socket
        self._socket.textMessageReceived.connect(self.textMessageReceived)
        self._socket.disconnected.connect(self._disconnected)

    def __del__(self):
        """Destroys the WebSocketTransport."""
        self._socket.deleteLater()

    def _disconnected(self):
        self.deleteLater()

    def sendMessage(self, message):
        """Serialize the JSON message and send it as a text message via the
           WebSocket to the client."""
        doc = QJsonDocument(message)
        json_message = str(doc.toJson(QJsonDocument.Compact), "utf-8")
        self._socket.sendTextMessage(json_message)

    @Slot(str)
    def textMessageReceived(self, message_data_in):
        """Deserialize the stringified JSON messageData and emit
           messageReceived."""
        message_data = QByteArray(bytes(message_data_in, encoding='utf8'))
        message = QJsonDocument.fromJson(message_data)
        if message.isNull():
            print("Failed to parse text message as JSON object:", message_data)
            return
        if not message.isObject():
            print("Received JSON message that is not an object: ", message_data)
            return
        self.messageReceived.emit(message.object(), self)
