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

void wrapInFunction()
{

//! [0]
file_ = QFile("file.dat")
file_.open(QIODevice.WriteOnly)
# we will serialize the data into the file
out = QDataStream(file_)
# serialize a string
out.writeQString("the answer is")
# serialize an integer
out.writeInt32(42)
//! [0]


//! [1]
file_ = QFile("file.dat")
file_.open(QIODevice.ReadOnly)
# read the data serialized from the file
i = QDataStream(file_)
string = ''
a = 0
# extract "the answer is" and 42
string = i.readQString()
a = i.readInt32()
//! [1]


//! [2]
stream.setVersion(QDataStream.Qt_4_0)
//! [2]


//! [3]
file_ = QFile("file.xxx")
file_.open(QIODevice.WriteOnly)
out = QDataStream(file_)

# Write a header with a "magic number" and a version
out.writeInt32(0xA0B0C0D0)
out.writeInt32(123)

out.setVersion(QDataStream.Qt_4_0)

// Write the data
out << lots_of_interesting_data
//! [3]


//! [4]
file_ = QFile("file.xxx")
file_.open(QIODevice.ReadOnly)
i = QDataStream(file_)

// Read and check the header
magic = i.readInt32()
if magic != 0xA0B0C0D0:
    return XXX_BAD_FILE_FORMAT

// Read the version
version = i.readInt32()
if version < 100:
    return XXX_BAD_FILE_TOO_OLD
if version > 123:
    return XXX_BAD_FILE_TOO_NEW

if version <= 110:
    in_.setVersion(QDataStream.Qt_3_2)
else:
    in_.setVersion(QDataStream.Qt_4_0)

// Read the data
in_ >> lots_of_interesting_data
if version >= 120:
    in_ >> data_new_in_XXX_version_1_2
in_ >> other_interesting_data
//! [4]


//! [5]
out = QDataStream(file_)
out.setVersion(QDataStream.Qt_4_0)
//! [5]

}
