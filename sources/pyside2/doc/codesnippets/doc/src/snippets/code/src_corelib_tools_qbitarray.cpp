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
ba = QBitArray(200)
//! [0]


//! [1]
ba = QBitArray()
ba.resize(3)
ba[0] = True
ba[1] = False
ba[2] = True
//! [1]


//! [2]
ba = QBitArray(3)
ba.setBit(0, True)
ba.setBit(1, False)
ba.setBit(2, True)
//! [2]


//! [3]
x = QBitArray(5)
x.setBit(3, True)
# x: [ 0, 0, 0, 1, 0 ]

y = QBitArray(5)
y.setBit(4, True)
# y: [ 0, 0, 0, 0, 1 ]

x |= y
# x: [ 0, 0, 0, 1, 1 ]
//! [3]


//! [4]
QBitArray().isNull()           # returns True
QBitArray().isEmpty()          # returns True

QBitArray(0).isNull()          # returns False
QBitArray(0).isEmpty()         # returns True

QBitArray(3).isNull()          # returns False
QBitArray(3).isEmpty()         # returns False
//! [4]


//! [5]
QBitArray().isNull()           # returns True
QBitArray(0).isNull()          # returns False
QBitArray(3).isNull()          # returns False
//! [5]


//! [6]
ba = QBitArray(8)
ba.fill(True)
# ba: [ 1, 1, 1, 1, 1, 1, 1, 1 ]

ba.fill(False, 2)
# ba: [ 0, 0 ]
//! [6]


//! [7]
a = QBitArray(3)
a[0] = False
a[1] = True
a[2] = a[0] ^ a[1]
//! [7]


//! [8]
a = QBitArray(3)
b = QBitArray(2)
a[0] = 1
a[1] = 0
a[2] = 1
# a: [ 1, 0, 1 ]

b[0] = 1
b[1] = 0
# b: [ 1, 1 ]

a &= b
# a: [ 1, 0, 0 ]
//! [8]


//! [9]
a = QBitArray(3)
b = QBitArray(2)
a[0] = 1
a[1] = 0
a[2] = 1
# a: [ 1, 0, 1 ]

b[0] = 1
b[1] = 0
# b: [ 1, 1 ]

a |= b
# a: [ 1, 1, 1 ]
//! [9]


//! [10]
a = QBitArray(3)
b = QBitArray(2)
a[0] = 1
a[1] = 0
a[2] = 1
# a: [ 1, 0, 1 ]

b[0] = 1
b[1] = 0
# b: [ 1, 1 ]

a ^= b
# a: [ 0, 1, 1 ]
//! [10]


//! [11]
a = QBitArray(3)
b = QBitArray()
a[0] = 1
a[1] = 0
a[2] = 1
# a: [ 1, 0, 1 ]

b = ~a
# b: [ 0, 1, 0 ]
//! [11]


//! [12]
a = QBitArray(3)
b = QBitArray(2)
c = QBitArray()
a[0] = 1
a[1] = 0
a[2] = 1
# a: [ 1, 0, 1 ]

b[0] = 1
b[1] = 0
# b: [ 1, 1 ]

c = a & b
# c: [ 1, 0, 0 ]
//! [12]


//! [13]
a = QBitArray(3)
QBitArray b(2)
QBitArray c
a[0] = 1
a[1] = 0
a[2] = 1
# a: [ 1, 0, 1 ]

b[0] = 1
b[1] = 0
# b: [ 1, 1 ]

c = a | b
# c: [ 1, 1, 1 ]
//! [13]


//! [14]
a = QBitArray(3)
b = QBitArray(2)
c = QBitArray()
a[0] = 1
a[1] = 0
a[2] = 1
# a: [ 1, 0, 1 ]

b[0] = 1
b[1] = 0
# b: [ 1, 1 ]

c = a ^ b
# c: [ 0, 1, 1 ]
//! [14]
