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

//![newstuff]
    fi = QFileInfo("c:/temp/foo") => fi.absoluteFilePath() => "C:/temp/foo"
//![newstuff]

//! [0]
info1 = QFileInfo("/home/bob/bin/untabify")
info1.isSymLink()           # returns true
info1.absoluteFilePath()    # returns "/home/bob/bin/untabify"
info1.size()                # returns 56201
info1.symLinkTarget()       # returns "/opt/pretty++/bin/untabify"

info2 = QFileInfo(info1.symLinkTarget())
info1.isSymLink()           # returns False
info1.absoluteFilePath()    # returns "/opt/pretty++/bin/untabify"
info1.size()                # returns 56201

//! [0]


//! [1]
info1 = QFileInfo("C:\\Documents and Settings\\Bob\\untabify.lnk")
info1.isSymLink()           # returns True
info1.absoluteFilePath()    # returns "C:/Documents and Settings/Bob/untabify.lnk"
info1.size()                # returns 743
info1.symLinkTarget()       # returns "C:/Pretty++/untabify"

info2 = QFileInfo(info1.symLinkTarget())
info1.isSymLink()           # returns False
info1.absoluteFilePath()    # returns "C:/Pretty++/untabify"
info1.size()                # returns 63942
//! [1]


//! [2]
absolute = "/local/bin"
relative = "local/bin"
absFile = QFileInfo(absolute)
relFile = QFileInfo(relative)

QDir.setCurrent(QDir.rootPath())
# absFile and relFile now point to the same file

QDir.setCurrent("/tmp")
# absFile now points to "/local/bin",
# while relFile points to "/tmp/local/bin"
//! [2]


//! [3]
fi = QFileInfo("/tmp/archive.tar.gz")
name = fi.fileName()                    # name = "archive.tar.gz"
//! [3]


//! [4]
fi = QFileInfo("/Applications/Safari.app")
bundle = fi.bundleName()                # name = "Safari"
//! [4]


//! [5]
fi = QFileInfo("/tmp/archive.tar.gz")
base = fi.baseName()                    # base = "archive"
//! [5]


//! [6]
fi = QFileInfo("/tmp/archive.tar.gz")
base = fi.completeBaseName()            # base = "archive.tar"
//! [6]


//! [7]
fi = QFileInfo("/tmp/archive.tar.gz")
ext = fi.completeSuffix()               # ext = "tar.gz"
//! [7]


//! [8]
fi = QFileInfo("/tmp/archive.tar.gz")
ext = fi.suffix();                      # ext = "gz"
//! [8]


//! [9]
info = QFileInfo(fileName)
if info.isSymLink():
    fileName = info.symLinkTarget()
//! [9]


//! [10]
fi = QFileInfo("/tmp/archive.tar.gz")
if fi.permission(QFileDevice.WriteUser | QFileDevice.ReadGroup):
    print("I can change the file; my group can read the file")
if fi.permission(QFileDevice.WriteGroup | QFileDevice.WriteOther):
    print("The group or others can change the file")
//! [10]
