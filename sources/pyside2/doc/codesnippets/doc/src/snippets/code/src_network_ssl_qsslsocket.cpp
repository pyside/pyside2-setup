/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt for Python project.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
socket = QSslSocket(self)
QObject.connect(socket, SIGNAL("encrypted()"), self, SLOT("ready()"))

socket.connectToHostEncrypted("imap.example.com", 993)
//! [0]


//! [1]
def incomingConnection(socketDescriptor):
    serverSocket = QSslSocket()
    if serverSocket.setSocketDescriptor(socketDescriptor):
        QObject.connect(serverSocket, SIGNAL("encrypted()"), self, SLOT("ready()"))
        serverSocket.startServerEncryption()
//! [1]


//! [2]
socket = QSslSocket()
socket.connectToHostEncrypted("http.example.com", 443)
if not socket.waitForEncrypted():
    print socket.errorString()
    return false

socket.write("GET / HTTP/1.0\r\n\r\n")
while socket.waitForReadyRead():
    print socket.readAll().data()
//! [2]


//! [3]
socket = QSslSocket()
QObject.connect(socket, SIGNAL("encrypted()"), receiver, SLOT("socketEncrypted()"))

socket.connectToHostEncrypted("imap", 993)
socket.write("1 CAPABILITY\r\n")
//! [3]


//! [4]
socket = QSslSocket()
socket.setCiphers("DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA")
//! [4]


//! [5]
socket.connectToHostEncrypted("imap", 993)
if socket.waitForEncrypted(1000):
    print "Encrypted!"
//! [5]
