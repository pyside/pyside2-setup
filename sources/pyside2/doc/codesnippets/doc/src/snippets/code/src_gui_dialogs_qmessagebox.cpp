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

//! [0]
ret = QMessageBox.warning(self, self.tr("My Application"),
                               self.tr("The document has been modified.\n" + \
                                  "Do you want to save your changes?"),
                               QMessageBox.Save | QMessageBox.Discard
                               | QMessageBox.Cancel,
                               QMessageBox.Save)
//! [0]


//! [1]
msgBox = QMessageBox()
msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
result = msgBox.exec_()

if result == QMessageBox.Yes:
    # yes was clicked
elif result == QMessageBox.No:
    # no was clicked
else:
    # should never be reached
//! [1]


//! [2]
msgBox = QMessageBox()
connectButton = msgBox.addButton(self.tr("Connect"), QMessageBox.ActionRole)
abortButton = msgBox.addButton(QMessageBox.Abort)

msgBox.exec_()

if msgBox.clickedButton() == connectButton:
    # connect
elif msgBox.clickedButton() == abortButton:
    # abort
}
//! [2]


//! [3]
messageBox = QMessageBox(self)
disconnectButton = messageBox.addButton(self.tr("Disconnect"),
                                        QMessageBox.ActionRole)
...
messageBox.exec_()
if messageBox.clickedButton() == disconnectButton:
    ...

//! [3]


//! [4]
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
# Not Supported by PySide
    QT_REQUIRE_VERSION(argc, argv, "4.0.2")

    QApplication app(argc, argv);
    ...
    return app.exec();
}
//! [4]

//! [5]
msgBox = QMessageBox()
msgBox.setText("The document has been modified.")
msgBox.exec_()
//! [5]

//! [6]
msgBox = QMessageBox()
msgBox.setText("The document has been modified.")
msgBox.setInformativeText("Do you want to save your changes?")
msgBox.setStandardButtons(QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
msgBox.setDefaultButton(QMessageBox.Save)
ret = msgBox.exec_()
//! [6]

//! [7]

if ret == QMessageBox.Save:
    # Save was clicked
elif ret == QMessageBox.Discard:
    # Don't save was clicked
elif ret == QMessageBox.Cancel:
    # cancel was clicked
else:
    # should never be reached

//! [7]

//! [9]
msgBox = QMessageBox(self)
msgBox.setText(tr("The document has been modified.\n" + \
                  "Do you want to save your changes?"))
msgBox.setStandardButtons(QMessageBox.Save | QMessageBox.Discard
                          | QMessageBox.Cancel)
msgBox.setDefaultButton(QMessageBox.Save)
//! [9]
