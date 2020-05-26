############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the examples of Qt for Python.
##
## $QT_BEGIN_LICENSE:BSD$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## BSD License Usage
## Alternatively, you may use this file under the terms of the BSD license
## as follows:
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
############################################################################

//! [0]
from PySide2.QtCore import Qt, QFile, QFileInfo, QSettings, QTextStream
from PySide2.QtGui import QIcon
from PySide2.Widgets import (QAction, QApplication, QFileDialog, QMainWindow,
                             QPlainTextEdit, QFileDialog, QMessageBox, )
//! [0]

//! [1]
def __init__(self, parent=None):
    QMainWindow.__init__(self)
//! [1] //! [2]
    self.textEdit =  QPlainTextEdit()
    self.setCentralWidget(textEdit)

    self.createActions()
    self.createMenus()
    self.createToolBars()
    self.createStatusBar()

    self.readSettings()

    self.textEdit.document().contentsChanged.connect(self.documentWasModified)

    self.setCurrentFile("")
    self.setUnifiedTitleAndToolBarOnMac(True)

//! [2]

//! [3]
def closeEvent(self, event):
//! [3] //! [4]
    if maybeSave():
        writeSettings()
        event.accept()
    else:
        event.ignore()
//! [4]

//! [5]
def File(self):
//! [5] //! [6]
    if maybeSave():
        textEdit.clear()
        setCurrentFile("")
//! [6]

//! [7]
def open(self):
//! [7] //! [8]
    if maybeSave():
        fileName = QFileDialog.getOpenFileName(self)
        if not fileName.isEmpty():
            loadFile(fileName)
//! [8]

//! [9]
def save(self):
//! [9] //! [10]
    if curFile.isEmpty():
        return saveAs()
    else:
        return saveFile(curFile)
//! [10]

//! [11]
def saveAs(self):
//! [11] //! [12]
    fileName = QFileDialog.getSaveFileName(self)
    if fileName.isEmpty():
        return False

    return saveFile(fileName)
//! [12]

//! [13]
def about(self):
//! [13] //! [14]
   QMessageBox.about(self, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."))

//! [14]

//! [15]
def documentWasModified(self):
//! [15] //! [16]
    setWindowModified(textEdit.document().isModified())
//! [16]

//! [17]
def MainWindow.createActions(self):
//! [17] //! [18]
    Act = QAction(QIcon(":/images/new.png"), tr("&New"), self)
    Act.setShortcuts(QKeySequence.New)
    Act.setStatusTip(tr("Create a new file"))
    Act.triggered.connect(newFile)

//! [19]
    openAct = QAction(QIcon(":/images/open.png"), tr("&Open..."), self)
    openAct.setShortcuts(QKeySequence.Open)
    openAct.setStatusTip(tr("Open an existing file"))
    openAct.triggered.connect(open)
//! [18] //! [19]

    saveAct = QAction(QIcon(":/images/save.png"), tr("&Save"), self)
    saveAct.setShortcuts(QKeySequence.Save)
    saveAct.setStatusTip(tr("Save the document to disk"))
    saveAct.triggered.connect(save)

    saveAsAct = QAction(tr("Save &As..."), self)
    saveAsAct.setShortcuts(QKeySequence.SaveAs)
    saveAsAct.setStatusTip(tr("Save the document under a  name"))
    saveAsAct.triggered.connect(saveAs)

//! [20]
    exitAct = QAction(tr("E&xit"), self)
    exitAct.setShortcut(tr("Ctrl+Q"))
//! [20]
    exitAct.setStatusTip(tr("Exit the application"))
    exitAct.triggered.connect(close)

//! [21]
    cutAct = QAction(QIcon(":/images/cut.png"), tr("Cu&t"), self)
//! [21]
    cutAct.setShortcuts(QKeySequence.Cut)
    cutAct.setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"))
    cutAct.triggered.connect(cut)

    copyAct = QAction(QIcon(":/images/copy.png"), tr("&Copy"), self)
    copyAct.setShortcuts(QKeySequence.Copy)
    copyAct.setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"))
    copyAct.triggered.connect(copy)

    pasteAct = QAction(QIcon(":/images/paste.png"), tr("&Paste"), self)
    pasteAct.setShortcuts(QKeySequence.Paste)
    pasteAct.setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"))
    pasteAct.triggered.connect(textEdit.paste)

    aboutAct = QAction(tr("&About"), self)
    aboutAct.setStatusTip(tr("Show the application's About box"))
    aboutAct.triggered.connect(about)

//! [22]
    aboutQtAct =  QAction(tr("About &Qt"), self)
    aboutQtAct.setStatusTip(tr("Show the Qt library's About box"))
    aboutQtAct.triggered.connect(qApp.aboutQt)
//! [22]

//! [23]
    cutAct.setEnabled(False)
//! [23] //! [24]
    copyAct.setEnabled(False)
    textEdit.copyAvailable[bool].connect(cutAct.setEnabled)
    textEdit.copyAvailable[bool].connect(copyAct.setEnabled)
}
//! [24]

//! [25] //! [26]
def createMenus(self):
//! [25] //! [27]
    fileMenu = menuBar().addMenu(tr("&File"))
    fileMenu.addAction(Act)
//! [28]
    fileMenu.addAction(openAct)
//! [28]
    fileMenu.addAction(saveAct)
//! [26]
    fileMenu.addAction(saveAsAct)
    fileMenu.addSeparator()
    fileMenu.addAction(exitAct)

    editMenu = menuBar().addMenu(tr("&Edit"))
    editMenu.addAction(cutAct)
    editMenu.addAction(copyAct)
    editMenu.addAction(pasteAct)

    menuBar().addSeparator()

    helpMenu = menuBar().addMenu(tr("&Help"))
    helpMenu.addAction(aboutAct)
    helpMenu.addAction(aboutQtAct)

//! [27]

//! [29] //! [30]
def createToolBars(self):
    fileToolBar = addToolBar(tr("File"))
    fileToolBar.addAction(Act)
//! [29] //! [31]
    fileToolBar.addAction(openAct)
//! [31]
    fileToolBar.addAction(saveAct)

    editToolBar = addToolBar(tr("Edit"))
    editToolBar.addAction(cutAct)
    editToolBar.addAction(copyAct)
    editToolBar.addAction(pasteAct)
//! [30]

//! [32]
def createStatusBar(self):
//! [32] //! [33]
    statusBar().showMessage(tr("Ready"))

//! [33]

//! [34] //! [35]
def readSettings(self):
//! [34] //! [36]
    settings("Trolltech", "Application Example")
    pos = settings.value("pos", QPoint(200, 200)).toPoint()
    size = settings.value("size", QSize(400, 400)).toSize()
    resize(size)
    move(pos)

//! [35] //! [36]

//! [37] //! [38]
def writeSettings(self):
//! [37] //! [39]
    settings = QSettings("Trolltech", "Application Example")
    settings.setValue("pos", pos())
    settings.setValue("size", size())

//! [38] //! [39]

//! [40]
def maybeSave(self):
//! [40] //! [41]
    if textEdit.document()->isModified():
        ret = QMessageBox.warning(self, tr("Application"),
                                  tr("The document has been modified.\n"
                                     "Do you want to save your changes?"),
                                 QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
        if ret == QMessageBox.Save:
            return save()
        elif ret == QMessageBox.Cancel:
            return False
    return True
//! [41]

//! [42]
def loadFile(self, fileName):
//! [42] //! [43]
    file = QFile(fileName)
    if !file.open(QFile.ReadOnly | QFile.Text):
        QMessageBox.warning(self, tr("Application"), tr("Cannot read file "
            "{}:\n{}.".format(fileName, file.errorString())))
        return

    in = QTextStream(file)
    QApplication.setOverrideCursor(Qt::WaitCursor)
    textEdit.setPlainText(in.readAll())
    QApplication.restoreOverrideCursor()

    self.setCurrentFile(fileName)
    self.statusBar().showMessage(tr("File loaded"), 2000)

//! [43]

//! [44]
def saveFile(self, fileName):
//! [44] //! [45]
    file = QFile(fileName)
    if !file.open(QFile.WriteOnly | QFile::Text):
        QMessageBox.warning(self, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()))
        return False

    out = QTextStream(file)
    QApplication.setOverrideCursor(Qt.WaitCursor)
    out << textEdit.toPlainText()
    QApplication.restoreOverrideCursor()

    setCurrentFile(fileName)
    statusBar().showMessage(tr("File saved"), 2000)
    return True

//! [45]

//! [46]
def setCurrentFile(fileName):
//! [46] //! [47]
    curFile = fileName
    textEdit.document().setModified(False)
    setWindowModified(False)

    if curFile.isEmpty():
        shownName = "untitled.txt"
    else:
        shownName = strippedName(curFile)

    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Application")))

//! [47]

//! [48]
def strippedName(self, fullFileName):
//! [48] //! [49]
    return QFileInfo(fullFileName).fileName()
//! [49]
