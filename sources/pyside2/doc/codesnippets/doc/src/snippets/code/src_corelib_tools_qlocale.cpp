/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of PySide2.
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
egyptian = QLocale(QLocale.Arabic, QLocale.Egypt)
s1 = egyptian.toString(1.571429E+07, 'e')
s2 = egyptian.toString(10)

(d, ok) = egyptian.toDouble(s1)
(i, ok) = egyptian.toInt(s2)
//! [0]


//! [1]
QLocale.setDefault(QLocale(QLocale.Hebrew, QLocale.Israel))
hebrew = QLocale() # Constructs a default QLocale
s1 = hebrew.toString(15714.3, 'e')

QLocale.setDefault(QLocale(QLocale.C))
c = QLocale()
(d, ok) = c.toDouble("1234,56")   # ok == false
(d, ok) = c.toDouble("1234.56")   # ok == true, d == 1234.56

QLocale.setDefault(QLocale(QLocale.German))
german = QLocale()
(d, ok) = german.toDouble("1234,56")   # ok == true, d == 1234.56
(d, ok) = german.toDouble("1234.56")   # ok == true, d == 1234.56

QLocale.setDefault(QLocale(QLocale.English, QLocale.UnitedStates))
english = QLocale()
string = '%s %s %10x' % (12345, english.toString(12345), 12345)
# string == "12345 12,345 3039"
//! [1]


//! [2]
korean = QLocale("ko")
swiss = QLocale("de_CH")
//! [2]


//! [3]
c = QLocale(QLocale.C)
(d, ok) = c.toDouble( "1234.56" )  # ok == true, d == 1234.56
(d, ok) = c.toDouble( "1,234.56" ) # ok == true, d == 1234.56
(d, ok) = c.toDouble( "1234,56" )  # ok == false

german = QLocale(QLocale.German)
(d, ok) = german.toDouble( "1234,56" )  # ok == true, d == 1234.56
(d, ok) = german.toDouble( "1.234,56" ) # ok == true, d == 1234.56
(d, ok) = german.toDouble( "1234.56" )  # ok == false

(d, ok) = german.toDouble( "1.234" )    # ok == true, d == 1234.0
//! [3]
