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

void wrapInFunction()
{

//! [0]
class MyClass:

    Q_CLASSINFO("author", "Sabrina Schweinsteiger")
    Q_CLASSINFO("url", "http://doc.moosesoft.co.uk/1.0/")

    ...
//! [0]


//! [1]
normType = QMetaObject.normalizedType(" int    const  *")
# normType is now "const int*"
//! [1]


//! [2]
QMetaObject.invokeMethod(pushButton, "animateClick",
                         Qt.QueuedConnection)
//! [2]


//! [3]
QMetaObject.invokeMethod: Unable to handle unregistered datatype 'MyType'
//! [3]


//! [4]
retVal = QString()
QMetaObject.invokeMethod(obj, "compute", Qt::DirectConnection,
                         Q_RETURN_ARG(QString, retVal),
                         Q_ARG(QString, "sqrt"),
                         Q_ARG(int, 42),
                         Q_ARG(double, 9.7));
//! [4]


//! [5]
class MyClass:
    Q_CLASSINFO("author", "Sabrina Schweinsteiger")
    Q_CLASSINFO("url", "http://doc.moosesoft.co.uk/1.0/")
//! [5]


//! [propertyCount]
metaObject = obj.metaObject()
properties = [metaObject.property(i).name() for i in range(metaObject.propertyOffset(), metaObject.propertyCount())]
//! [propertyCount]


//! [methodCount]
metaObject = obj.metaObject()
methods = [metaObject.method(i).signature() for i in range(metaObject.methodOffset(), metaObject.methodCount())]
//! [methodCount]

//! [6]
methodIndex = pushButton.metaObject().indexOfMethod("animateClick()")
method = metaObject.method(methodIndex)
method.invoke(pushButton, Qt.QueuedConnection)
//! [6]

//! [7]
QMetaMethod.invoke: Unable to handle unregistered datatype 'MyType'
//! [7]

//! [8]
retVal = QString()
normalizedSignature = QMetaObject.normalizedSignature("compute(QString, int, double)")
methodIndex = obj.metaObject().indexOfMethod(normalizedSignature)
method = metaObject.method(methodIndex)
method.invoke(obj,
              Qt.DirectConnection,
              Q_RETURN_ARG(QString, retVal),
              Q_ARG(QString, "sqrt"),
              Q_ARG(int, 42),
              Q_ARG(double, 9.7));
//! [8]

}
