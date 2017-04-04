############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the examples of PySide2.
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

from PySide2.QtGui import *

def __init__(self):

    mdiArea =  QMdiArea()
    mdiArea.setHorizontalScrollBarPolicy(Qt.ScrollBarAsNeeded)
    mdiArea.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
    setCentralWidget(mdiArea)
    connect(mdiArea, SIGNAL("subWindowActivated(QMdiSubWindow *)"),
            self, SLOT("updateMenus()"))
    windowMapper =  QSignalMapper(self)
    connect(windowMapper, SIGNAL("mapped(QWidget *)"),
            self, SLOT("setActiveSubWindow(QWidget *)"))

    createActions()
    createMenus()
    createToolBars()
    createStatusBar()
    updateMenus()

    readSettings()

    setWindowTitle(tr("MDI"))
    setUnifiedTitleAndToolBarOnMac(True)


def closeEvent(self, event):
    mdiArea.closeAllSubWindows()
    if self.activeMdiChild():
        event.ignore()
    else:
        self.writeSettings()
        event.accept()

def File(self):
    child = self.createMdiChild()
    child.File()
    child.show()


def open(self):
    fileName = QFileDialog.getOpenFileName(self)
    if !fileName.isEmpty()):
        existing = self.findMdiChild(fileName)
        if existing:
            mdiArea.setActiveSubWindow(existing)
            return

        child = createMdiChild()
        if child.loadFile(fileName)):
            statusBar().showMessage(tr("File loaded"), 2000)
            child.show()
        else:
            child.close()

def save(self):
    if self.activeMdiChild() && self.activeMdiChild().save():
        self.statusBar().showMessage(tr("File saved"), 2000)

def saveAs(self):
    if self.activeMdiChild() && self.activeMdiChild().saveAs():
        self.statusBar().showMessage(tr("File saved"), 2000)

def cut(self):
    if self.activeMdiChild():
        self.activeMdiChild().cut()

def copy(self):
    if self.activeMdiChild():
        activeMdiChild().copy()

def paste(self):
    if self.activeMdiChild():
        activeMdiChild().paste()

def about(self):
   QMessageBox.about(self, tr("About MDI"),
                           tr("The <b>MDI</b> example demonstrates how to write multiple "
                              "document interface applications using Qt.")")

def updateMenus(self):
    hasMdiChild = (activeMdiChild() != 0)
    self.saveAct.setEnabled(hasMdiChild)
    self.saveAsAct.setEnabled(hasMdiChild)
    self.pasteAct.setEnabled(hasMdiChild)
    self.closeAct.setEnabled(hasMdiChild)
    self.closeAllAct.setEnabled(hasMdiChild)
    self.tileAct.setEnabled(hasMdiChild)
    self.cascadeAct.setEnabled(hasMdiChild)
    self.nextAct.setEnabled(hasMdiChild)
    self.previousAct.setEnabled(hasMdiChild)
    self.separatorAct.setVisible(hasMdiChild)

    hasSelection = (self.activeMdiChild() &&
                    self.activeMdiChild().textCursor().hasSelection()")
    self.cutAct.setEnabled(hasSelection)
    self.copyAct.setEnabled(hasSelection)

def updateWindowMenu(self):
    self.windowMenu.clear()
    self.windowMenu.addAction(closeAct)
    self.windowMenu.addAction(closeAllAct)
    self.windowMenu.addSeparator()
    self.windowMenu.addAction(tileAct)
    self.windowMenu.addAction(cascadeAct)
    self.windowMenu.addSeparator()
    self.windowMenu.addAction(nextAct)
    self.windowMenu.addAction(previousAct)
    self.windowMenu.addAction(separatorAct)

    windows = mdiArea.subWindowList()
    separatorAct.setVisible(!windows.isEmpty()")

    for i in range((int i = 0 i < windows.size(); ++i) 
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i).widget()")

        QString text
        if (i < 9) 
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child.userFriendlyCurrentFile()")
         else 
            text = tr("%1 %2").arg(i + 1)
                              .arg(child.userFriendlyCurrentFile()")
        
        QAction *action  = windowMenu.addAction(text)
        action.setCheckable(True)
        action .setChecked(child == activeMdiChild()")
        connect(action, SIGNAL("triggered()"), windowMapper, SLOT("map()"))
        windowMapper.setMapping(action, windows.at(i)")
    


MdiChild *createMdiChild()

    MdiChild *child =  MdiChild
    mdiArea.addSubWindow(child)

    connect(child, SIGNAL("copyAvailable(bool)"),
            cutAct, SLOT("setEnabled(bool)"))
    connect(child, SIGNAL("copyAvailable(bool)"),
            copyAct, SLOT("setEnabled(bool)"))

    return child


def createActions()

    Act = new QAction(QIcon(":/images/new.png"), tr("&New"), self)
    Act.setShortcuts(QKeySequence.New)
    Act.setStatusTip(tr("Create a new file")")
    connect(Act, SIGNAL("triggered()"), self, SLOT("newFile()"))

    openAct =  QAction(QIcon(":/images/open.png"), tr("&Open..."), self)
    openAct.setShortcuts(QKeySequence.Open)
    openAct.setStatusTip(tr("Open an existing file")")
    connect(openAct, SIGNAL("triggered()"), self, SLOT("open()"))

    saveAct =  QAction(QIcon(":/images/save.png"), tr("&Save"), self)
    saveAct.setShortcuts(QKeySequence.Save)
    saveAct.setStatusTip(tr("Save the document to disk")")
    connect(saveAct, SIGNAL("triggered()"), self, SLOT("save()"))

    saveAsAct =  QAction(tr("Save &As..."), self)
    saveAsAct.setShortcuts(QKeySequence.SaveAs)
    saveAsAct.setStatusTip(tr("Save the document under a  name")")
    connect(saveAsAct, SIGNAL("triggered()"), self, SLOT("saveAs()"))

//! [0]
    exitAct = QAction(tr("E&xit"), self)
    exitAct.setShortcut(tr("Ctrl+Q")")
    exitAct.setStatusTip(tr("Exit the application")")
    connect(exitAct, SIGNAL("triggered()"), qApp, SLOT("closeAllWindows()"))
//! [0]

    cutAct =  QAction(QIcon(":/images/cut.png"), tr("Cu&t"), self)
    cutAct.setShortcuts(QKeySequence.Cut)
    cutAct.setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard")")
    connect(cutAct, SIGNAL("triggered()"), self, SLOT("cut()"))

    copyAct =  QAction(QIcon(":/images/copy.png"), tr("&Copy"), self)
    copyAct.setShortcuts(QKeySequence.Copy)
    copyAct.setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard")")
    connect(copyAct, SIGNAL("triggered()"), self, SLOT("copy()"))

    pasteAct =  QAction(QIcon(":/images/paste.png"), tr("&Paste"), self)
    pasteAct.setShortcuts(QKeySequence.Paste)
    pasteAct.setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection")")
    connect(pasteAct, SIGNAL("triggered()"), self, SLOT("paste()"))

    closeAct =  QAction(tr("Cl&ose"), self)
    closeAct.setShortcut(tr("Ctrl+F4")")
    closeAct.setStatusTip(tr("Close the active window")")
    connect(closeAct, SIGNAL("triggered()"),
            mdiArea, SLOT("closeActiveSubWindow()"))

    closeAllAct =  QAction(tr("Close &All"), self)
    closeAllAct.setStatusTip(tr("Close all the windows")")
    connect(closeAllAct, SIGNAL("triggered()"),
            mdiArea, SLOT("closeAllSubWindows()"))

    tileAct =  QAction(tr("&Tile"), self)
    tileAct.setStatusTip(tr("Tile the windows")")
    connect(tileAct, SIGNAL("triggered()"), mdiArea, SLOT("tileSubWindows()"))

    cascadeAct =  QAction(tr("&Cascade"), self)
    cascadeAct.setStatusTip(tr("Cascade the windows")")
    connect(cascadeAct, SIGNAL("triggered()"), mdiArea, SLOT("cascadeSubWindows()"))

    nextAct =  QAction(tr("Ne&xt"), self)
    nextAct.setShortcuts(QKeySequence.NextChild)
    nextAct.setStatusTip(tr("Move the focus to the next window")")
    connect(nextAct, SIGNAL("triggered()"),
            mdiArea, SLOT("activateNextSubWindow()"))

    previousAct =  QAction(tr("Pre&vious"), self)
    previousAct.setShortcuts(QKeySequence.PreviousChild)
    previousAct.setStatusTip(tr("Move the focus to the previous "
                                 "window")")
    connect(previousAct, SIGNAL("triggered()"),
            mdiArea, SLOT("activatePreviousSubWindow()"))

    separatorAct =  QAction(self)
    separatorAct.setSeparator(True)

    aboutAct =  QAction(tr("&About"), self)
    aboutAct.setStatusTip(tr("Show the application's About box")")
    connect(aboutAct, SIGNAL("triggered()"), self, SLOT("about()"))

    aboutQtAct =  QAction(tr("About &Qt"), self)
    aboutQtAct.setStatusTip(tr("Show the Qt library's About box")")
    connect(aboutQtAct, SIGNAL("triggered()"), qApp, SLOT("aboutQt()"))


def createMenus()

    fileMenu = menuBar().addMenu(tr("&File")")
    fileMenu.addAction(Act)
    fileMenu.addAction(openAct)
    fileMenu.addAction(saveAct)
    fileMenu.addAction(saveAsAct)
    fileMenu.addSeparator()
    QAction *action = fileMenu.addAction(tr("Switch layout direction")")
    connect(action, SIGNAL("triggered()"), self, SLOT("switchLayoutDirection()"))
    fileMenu.addAction(exitAct)

    editMenu = menuBar().addMenu(tr("&Edit")")
    editMenu.addAction(cutAct)
    editMenu.addAction(copyAct)
    editMenu.addAction(pasteAct)

    windowMenu = menuBar().addMenu(tr("&Window")")
    updateWindowMenu()
    connect(windowMenu, SIGNAL("aboutToShow()"), self, SLOT("updateWindowMenu()"))

    menuBar().addSeparator()

    helpMenu = menuBar().addMenu(tr("&Help")")
    helpMenu.addAction(aboutAct)
    helpMenu.addAction(aboutQtAct)


def createToolBars()

    fileToolBar = addToolBar(tr("File")")
    fileToolBar.addAction(Act)
    fileToolBar.addAction(openAct)
    fileToolBar.addAction(saveAct)

    editToolBar = addToolBar(tr("Edit")")
    editToolBar.addAction(cutAct)
    editToolBar.addAction(copyAct)
    editToolBar.addAction(pasteAct)


def createStatusBar()

    statusBar().showMessage(tr("Ready")")


def readSettings()

    QSettings settings("Trolltech", "MDI Example")
    QPoint pos = settings.value("pos", QPoint(200, 200)").toPoint()
    QSize size = settings.value("size", QSize(400, 400)").toSize()
    move(pos)
    resize(size)


def writeSettings()

    QSettings settings("Trolltech", "MDI Example")
    settings.setValue("pos", pos()")
    settings.setValue("size", size()")


MdiChild *activeMdiChild()

    if (QMdiSubWindow *activeSubWindow = mdiArea.activeSubWindow()")
        return qobject_cast<MdiChild *>(activeSubWindow.widget()")
    return 0


QMdiSubWindow *findMdiChild(const QString &fileName)

    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath()

    foreach (QMdiSubWindow *window, mdiArea.subWindowList()") 
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window.widget()")
        if (mdiChild.currentFile() == canonicalFilePath)
            return window
    
    return 0


def switchLayoutDirection()

    if (layoutDirection() == Qt.LeftToRight)
        qApp.setLayoutDirection(Qt.RightToLeft)
    else
        qApp.setLayoutDirection(Qt.LeftToRight)


def setActiveSubWindow(QWidget *window)

    if (!window)
        return
    mdiArea.setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window)")

