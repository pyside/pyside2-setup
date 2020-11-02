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
def main():
    useGUI = not '-no-gui' in sys.argv
    app = QApplication(sys.argv) if useGUI else QCoreApplication(sys.argv)
    ...
    return app.exec_()
//! [0]


//! [1]
QApplication.setStyle(QWindowsStyle())
//! [1]


//! [2]
def main():
    QApplication.setColorSpec(QApplication.ManyColor)
    QApplication app(sys.argv)
    ...
    return app.exec_()

//! [2]


//! [3]
class MyWidget (QWidget):
    # ...
    def sizeHint(self):
        return QSize(80, 25).expandedTo(QApplication.globalStrut())
//! [3]


//! [4]
def showAllHiddenTopLevelWidgets():
    for widget in QApplication.topLevelWidgets():
        if widget.isHidden():
            widget.show()
//! [4]


//! [5]
def updateAllWidgets():
    for widget in QApplication.allWidgets()
        widget.update()
//! [5]


//! [6]
if __name__ == '__main__':
    QApplication.setDesktopSettingsAware(False)
    app = QApplication(sys.argv)
    # ...
    return app.exec_()
//! [6]


//! [7]
if (startPos - currentPos).manhattanLength() >= QApplication.startDragDistance():
    startTheDrag()
//! [7]


//! [8]
class MyApplication (QApplication):
# ...
    def commitData(QSessionManager& manager)
        if manager.allowsInteraction():
            ret = QMessageBox.warning(
                    mainWindow,
                    QObject.tr("My Application"),
                    QObject.tr("Save changes to document?"),
                    QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)

            if ret == QMessageBox.Save:
                manager.release()
                if not saveDocument():
                    manager.cancel()
            elif ret == QMessageBox.Discard:
                pass
            else:
                manager.cancel()
        else:
            # we did not get permission to interact, then
            # do something reasonable instead
            pass
//! [8]


//! [9]
appname -session id
//! [9]


//! [10]
for command in mySession.restartCommand():
    do_something(command)
//! [10]


//! [11]
for command in mySession.discardCommand():
    do_something(command)
//! [11]


//! [12]
widget = qApp.widgetAt(x, y)
if widget:
    widget = widget.window()
//! [12]


//! [13]
widget = qApp.widgetAt(point)
if widget:
    widget = widget.window()
//! [13]
