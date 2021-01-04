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

from PySide6.QtGui import *

class QMdiSubWindow(QMainWindow):
    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)

        mdiArea =  QMdiArea()
        mdiArea.setHorizontalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        mdiArea.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        setCentralWidget(mdiArea)
        mdiArea.subWindowActivated[QMdiSubWindow].connect(updateMenus)
        windowMapper =  QSignalMapper(self)
        windowMapper.mapped[QWidget].connect(setActiveSubWindow)

        self.createActions()
        self.createMenus()
        self.createToolBars()
        self.createStatusBar()
        self.updateMenus()
        self.readSettings()
        self.setWindowTitle(tr("MDI"))
        self.setUnifiedTitleAndToolBarOnMac(True)


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
        if not fileName.isEmpty():
            existing = self.findMdiChild(fileName)
            if existing:
                mdiArea.setActiveSubWindow(existing)
                return

            child = createMdiChild()
            if child.loadFile(fileName):
                statusBar().showMessage(tr("File loaded"), 2000)
                child.show()
            else:
                child.close()

    def save(self):
        if self.activeMdiChild() and self.activeMdiChild().save():
            self.statusBar().showMessage(tr("File saved"), 2000)

    def saveAs(self):
        if self.activeMdiChild() and self.activeMdiChild().saveAs():
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
                                  "document interface applications using Qt."))

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

        hasSelection = (self.activeMdiChild() and
                        self.activeMdiChild().textCursor().hasSelection())
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
        separatorAct.setVisible(not windows.isEmpty())

        for i in range(0, windows.size()):
            child = windows.at(i).widget()

            text = ""
            child_file = child.userFriendlyCurrentFile()
            if i < 9:
                text = f"&{i + 1} {child_file}"
            else:
                text = f"{i + 1} {child_file}"

            action  = windowMenu.addAction(text)
            action.setCheckable(True)
            action.setChecked(child == activeMdiChild())
            action.triggered.connect(windowMapper.map)
            windowMapper.setMapping(action, windows.at(i))

            createMdiChild = MdiChild()

            child =  MdiChild()
            mdiArea.addSubWindow(child)

            child.copyAvailable[bool].connect(cutAct.setEnabled)
            child.copyAvailable[bool].connect(copyAct.setEnabled)

            return child


    def createActions(self):

        Act = QAction(QIcon(":/images/new.png"), tr("&New"), self)
        Act.setShortcuts(QKeySequence.New)
        Act.setStatusTip(tr("Create a new file"))
        Act.triggered.connect(self.newFile)

        openAct = QAction(QIcon(":/images/open.png"), tr("&Open..."), self)
        openAct.setShortcuts(QKeySequence.Open)
        openAct.setStatusTip(tr("Open an existing file"))
        openAct.triggered.connect(self.open)

        saveAct =  QAction(QIcon(":/images/save.png"), tr("&Save"), self)
        saveAct.setShortcuts(QKeySequence.Save)
        saveAct.setStatusTip(tr("Save the document to disk"))
        saveAct.triggered.connect(self.save)

        saveAsAct =  QAction(tr("Save &As..."), self)
        saveAsAct.setShortcuts(QKeySequence.SaveAs)
        saveAsAct.setStatusTip(tr("Save the document under a  name"))
        saveAsAct.triggered.connect(self.saveAs)

//! [0]
        exitAct = QAction(tr("E&xit"), self)
        exitAct.setShortcut(tr("Ctrl+Q"))
        exitAct.setStatusTip(tr("Exit the application"))
        exitAct.triggered.connect(qApp.closeAllWindows)
//! [0]

        cutAct =  QAction(QIcon(":/images/cut.png"), tr("Cu&t"), self)
        cutAct.setShortcuts(QKeySequence.Cut)
        cutAct.setStatusTip(tr("Cut the current selection's contents to the "
                                "clipboard"))
        cutAct.triggered.connect(self.cut)

        copyAct =  QAction(QIcon(":/images/copy.png"), tr("&Copy"), self)
        copyAct.setShortcuts(QKeySequence.Copy)
        copyAct.setStatusTip(tr("Copy the current selection's contents to the "
                                 "clipboard"))
        copyAct.triggered.connect(self.copy)

        pasteAct =  QAction(QIcon(":/images/paste.png"), tr("&Paste"), self)
        pasteAct.setShortcuts(QKeySequence.Paste)
        pasteAct.setStatusTip(tr("Paste the clipboard's contents into the current "
                                  "selection"))
        pasteAct.triggered.connect(self.paste)

        closeAct =  QAction(tr("Cl&ose"), self)
        closeAct.setShortcut(tr("Ctrl+F4"))
        closeAct.setStatusTip(tr("Close the active window"))
        closeAct.triggered.connect(mdiArea.closeActiveSubWindow)

        closeAllAct =  QAction(tr("Close &All"), self)
        closeAllAct.setStatusTip(tr("Close all the windows"))
        closeAllAct.triggered.connect(mdiArea.closeAllSubWindows)

        tileAct =  QAction(tr("&Tile"), self)
        tileAct.setStatusTip(tr("Tile the windows"))
        tileAct.triggered.connect(mdiArea.tileSubWindows)

        cascadeAct =  QAction(tr("&Cascade"), self)
        cascadeAct.setStatusTip(tr("Cascade the windows"))
        cascadeAct.triggered.connect(mdiArea.cascadeSubWindows)

        nextAct =  QAction(tr("Ne&xt"), self)
        nextAct.setShortcuts(QKeySequence.NextChild)
        nextAct.setStatusTip(tr("Move the focus to the next window"))
        nextAct.triggered.connect(mdiArea.activateNextSubWindow)

        previousAct =  QAction(tr("Pre&vious"), self)
        previousAct.setShortcuts(QKeySequence.PreviousChild)
        previousAct.setStatusTip(tr("Move the focus to the previous "
                                     "window"))
        previousAct.triggered.connect(mdiArea.activatePreviousSubWindow)

        separatorAct =  QAction(self)
        separatorAct.setSeparator(True)

        aboutAct =  QAction(tr("&About"), self)
        aboutAct.setStatusTip(tr("Show the application's About box"))
        aboutAct.triggered.connect(self.about)

        aboutQtAct =  QAction(tr("About &Qt"), self)
        aboutQtAct.setStatusTip(tr("Show the Qt library's About box"))
        aboutQtAct.triggered.connect(qApp.aboutQt)


    def createMenus(self):

        fileMenu = menuBar().addMenu(tr("&File"))
        fileMenu.addAction(Act)
        fileMenu.addAction(openAct)
        fileMenu.addAction(saveAct)
        fileMenu.addAction(saveAsAct)
        fileMenu.addSeparator()
        action = fileMenu.addAction(tr("Switch layout direction"))
        action.triggered.connect(self.switchLayoutDirection)
        fileMenu.addAction(exitAct)

        editMenu = menuBar().addMenu(tr("&Edit"))
        editMenu.addAction(cutAct)
        editMenu.addAction(copyAct)
        editMenu.addAction(pasteAct)

        windowMenu = menuBar().addMenu(tr("&Window"))
        updateWindowMenu()
        windowMenu.aboutToShow.connect(self.updateWindowMenu)

        menuBar().addSeparator()

        helpMenu = menuBar().addMenu(tr("&Help"))
        helpMenu.addAction(aboutAct)
        helpMenu.addAction(aboutQtAct)


    def createToolBars(self):
        fileToolBar = addToolBar(tr("File"))
        fileToolBar.addAction(Act)
        fileToolBar.addAction(openAct)
        fileToolBar.addAction(saveAct)

        editToolBar = addToolBar(tr("Edit"))
        editToolBar.addAction(cutAct)
        editToolBar.addAction(copyAct)
        editToolBar.addAction(pasteAct)


    def createStatusBar(self):
        statusBar().showMessage(tr("Ready"))


    def readSettings(self):
        settings = QSettings("Trolltech", "MDI Example")
        QPoint pos = settings.value("pos", QPoint(200, 200)").toPoint()
        QSize size = settings.value("size", QSize(400, 400)").toSize()
        move(pos)
        resize(size)

    def writeSettings(self):
        QSettings settings("Trolltech", "MDI Example")
        settings.setValue("pos", pos()")
        settings.setValue("size", size()")


        activeMdiChild = MdiChild()
        activeSubWindow = mdiArea.activeSubWindow()
        if activeSubWindow:
            return activeSubWindow.widget()
        return 0


    def findMdiChild(self, fileName):

        canonicalFilePath = QFileInfo(fileName).canonicalFilePath()

        for window in mdiArea.subWindowList():
            mdiChild = window.widget()
            if mdiChild.currentFile() == canonicalFilePath:
                return window
        return 0


    def switchLayoutDirection(self)
        if layoutDirection() == Qt.LeftToRight:
            qApp.setLayoutDirection(Qt.RightToLeft)
        else:
            qApp.setLayoutDirection(Qt.LeftToRight)


    def setActiveSubWindow(self, window):
        if not window:
            return
        mdiArea.setActiveSubWindow(window)
