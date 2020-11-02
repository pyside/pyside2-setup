/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of Qt for Python.
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

def wrapInFunction():

//! [0]
builder = QProcess()
builder.setProcessChannelMode(QProcess.MergedChannels)
builder.start("make", ["-j2"])

import sys
if not builder.waitForFinished():
    sys.stderr.write("Make failed:" + builder.errorString())
else
    sys.stderr.write("Make output:" + builder.readAll())
//! [0]


//! [1]
more = QProcess()
more.start("more")
more.write("Text to display")
more.closeWriteChannel()
#QProcess will emit readyRead() once "more" starts printing
//! [1]


//! [2]
command1 | command2
//! [2]


//! [3]
process1 = QProcess()
process2 = QProcess()

process1.setStandardOutputProcess(process2)

process1.start("command1")
process2.start("command2")
//! [3]


//! [4]
class SandboxProcess(QProcess):
    def setupChildProcess(self)
        # Drop all privileges in the child process, and enter
        # a chroot jail.
        os.setgroups(0, 0)
        os.chroot("/etc/safe")
        os.chdir("/")
        os.setgid(safeGid)
        os.setuid(safeUid)
        os.umask(0)

//! [4]


//! [5]
process = QProcess()
process.start("del /s *.txt")
# same as process.start("del", ["/s", "*.txt"])
...
//! [5]


//! [6]
process = QProcess()
process.start("dir \"My Documents\"")
//! [6]


//! [7]
process = QProcess()
process.start("dir \"\"\"My Documents\"\"\"")
//! [7]


//! [8]
environment = QProcess.systemEnvironment()
# environment = [PATH=/usr/bin:/usr/local/bin",
#                "USER=greg", "HOME=/home/greg"]
//! [8]


