#!/usr/bin/env python

#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: http://www.qt.io/licensing/
##
## This file is part of the Qt for Python examples of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:BSD$
## You may use this file under the terms of the BSD license as follows:
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
#############################################################################

"""PySide2 WebEngineWidgets Example"""

import sys
from bookmarkwidget import BookmarkWidget
from browsertabwidget import BrowserTabWidget
from downloadwidget import DownloadWidget
from findtoolbar import FindToolBar
from webengineview import QWebEnginePage, WebEngineView
from PySide2 import QtCore
from PySide2.QtCore import Qt, QUrl
from PySide2.QtGui import QCloseEvent, QKeySequence, QIcon
from PySide2.QtWidgets import (qApp, QAction, QApplication, QDesktopWidget,
    QDockWidget, QLabel, QLineEdit, QMainWindow, QMenu, QMenuBar, QPushButton,
    QStatusBar, QToolBar)
from PySide2.QtWebEngineWidgets import (QWebEngineDownloadItem, QWebEnginePage,
    QWebEngineView)

mainWindows = []

def createMainWindow():
    mainWin = MainWindow()
    mainWindows.append(mainWin)
    availableGeometry = app.desktop().availableGeometry(mainWin)
    mainWin.resize(availableGeometry.width() * 2 / 3, availableGeometry.height() * 2 / 3)
    mainWin.show()
    return mainWin

def createMainWindowWithBrowser():
    mainWin = createMainWindow()
    return mainWin.addBrowserTab()

class MainWindow(QMainWindow):

    def __init__(self):
        super(MainWindow, self).__init__()

        self.setWindowTitle('PySide2 Tabbed Browser Example')

        self._tabWidget = BrowserTabWidget(createMainWindowWithBrowser)
        self._tabWidget.enabledChanged.connect(self._enabledChanged)
        self._tabWidget.downloadRequested.connect(self._downloadRequested)
        self.setCentralWidget(self._tabWidget)
        self.connect(self._tabWidget, QtCore.SIGNAL("urlChanged(QUrl)"),
                     self.urlChanged)

        self._bookmarkDock = QDockWidget()
        self._bookmarkDock.setWindowTitle('Bookmarks')
        self._bookmarkWidget = BookmarkWidget()
        self._bookmarkWidget.openBookmark.connect(self.loadUrl)
        self._bookmarkWidget.openBookmarkInNewTab.connect(self.loadUrlInNewTab)
        self._bookmarkDock.setWidget(self._bookmarkWidget)
        self.addDockWidget(Qt.LeftDockWidgetArea, self._bookmarkDock)

        self._findToolBar = None

        self._actions = {}
        self._createMenu()

        self._toolBar = QToolBar()
        self.addToolBar(self._toolBar)
        for action in self._actions.values():
            if not action.icon().isNull():
                self._toolBar.addAction(action)

        self._addressLineEdit = QLineEdit()
        self._addressLineEdit.setClearButtonEnabled(True)
        self._addressLineEdit.returnPressed.connect(self.load)
        self._toolBar.addWidget(self._addressLineEdit)
        self._zoomLabel = QLabel()
        self.statusBar().addPermanentWidget(self._zoomLabel)
        self._updateZoomLabel()

        self._bookmarksToolBar = QToolBar()
        self.addToolBar(Qt.TopToolBarArea, self._bookmarksToolBar)
        self.insertToolBarBreak(self._bookmarksToolBar)
        self._bookmarkWidget.changed.connect(self._updateBookmarks)
        self._updateBookmarks()

    def _updateBookmarks(self):
        self._bookmarkWidget.populateToolBar(self._bookmarksToolBar)
        self._bookmarkWidget.populateOther(self._bookmarkMenu, 3)

    def _createMenu(self):
        fileMenu = self.menuBar().addMenu("&File")
        exitAction = QAction(QIcon.fromTheme("application-exit"), "E&xit",
                             self, shortcut = "Ctrl+Q", triggered=qApp.quit)
        fileMenu.addAction(exitAction)

        navigationMenu = self.menuBar().addMenu("&Navigation")

        styleIcons = ':/qt-project.org/styles/commonstyle/images/'
        backAction = QAction(QIcon.fromTheme("go-previous",
                                             QIcon(styleIcons + 'left-32.png')),
                             "Back", self,
                             shortcut = QKeySequence(QKeySequence.Back),
                             triggered = self._tabWidget.back)
        self._actions[QWebEnginePage.Back] = backAction
        backAction.setEnabled(False)
        navigationMenu.addAction(backAction)
        forwardAction = QAction(QIcon.fromTheme("go-next",
                                                QIcon(styleIcons + 'right-32.png')),
                                "Forward", self,
                                shortcut = QKeySequence(QKeySequence.Forward),
                                triggered = self._tabWidget.forward)
        forwardAction.setEnabled(False)
        self._actions[QWebEnginePage.Forward] = forwardAction

        navigationMenu.addAction(forwardAction)
        reloadAction = QAction(QIcon(styleIcons + 'refresh-32.png'),
                               "Reload", self,
                               shortcut = QKeySequence(QKeySequence.Refresh),
                               triggered = self._tabWidget.reload)
        self._actions[QWebEnginePage.Reload] = reloadAction
        reloadAction.setEnabled(False)
        navigationMenu.addAction(reloadAction)

        navigationMenu.addSeparator()

        newTabAction = QAction("New Tab", self,
                             shortcut = 'Ctrl+T',
                             triggered = self.addBrowserTab)
        navigationMenu.addAction(newTabAction)

        closeTabAction = QAction("Close Current Tab", self,
                                 shortcut = "Ctrl+W",
                                 triggered = self._closeCurrentTab)
        navigationMenu.addAction(closeTabAction)

        editMenu = self.menuBar().addMenu("&Edit")

        findAction = QAction("Find", self,
                             shortcut = QKeySequence(QKeySequence.Find),
                             triggered = self._showFind)
        editMenu.addAction(findAction)

        editMenu.addSeparator()
        undoAction = QAction("Undo", self,
                             shortcut = QKeySequence(QKeySequence.Undo),
                             triggered = self._tabWidget.undo)
        self._actions[QWebEnginePage.Undo] = undoAction
        undoAction.setEnabled(False)
        editMenu.addAction(undoAction)

        redoAction = QAction("Redo", self,
                             shortcut = QKeySequence(QKeySequence.Redo),
                             triggered = self._tabWidget.redo)
        self._actions[QWebEnginePage.Redo] = redoAction
        redoAction.setEnabled(False)
        editMenu.addAction(redoAction)

        editMenu.addSeparator()

        cutAction = QAction("Cut", self,
                            shortcut = QKeySequence(QKeySequence.Cut),
                            triggered = self._tabWidget.cut)
        self._actions[QWebEnginePage.Cut] = cutAction
        cutAction.setEnabled(False)
        editMenu.addAction(cutAction)

        copyAction = QAction("Copy", self,
                             shortcut = QKeySequence(QKeySequence.Copy),
                             triggered = self._tabWidget.copy)
        self._actions[QWebEnginePage.Copy] = copyAction
        copyAction.setEnabled(False)
        editMenu.addAction(copyAction)

        pasteAction = QAction("Paste", self,
                             shortcut = QKeySequence(QKeySequence.Paste),
                             triggered = self._tabWidget.paste)
        self._actions[QWebEnginePage.Paste] = pasteAction
        pasteAction.setEnabled(False)
        editMenu.addAction(pasteAction)

        editMenu.addSeparator()

        selectAllAction = QAction("Select All", self,
                                  shortcut = QKeySequence(QKeySequence.SelectAll),
                                  triggered = self._tabWidget.selectAll)
        self._actions[QWebEnginePage.SelectAll] = selectAllAction
        selectAllAction.setEnabled(False)
        editMenu.addAction(selectAllAction)

        self._bookmarkMenu = self.menuBar().addMenu("&Bookmarks")
        addBookmarkAction = QAction("&Add Bookmark", self,
                                    triggered = self._addBookmark)
        self._bookmarkMenu.addAction(addBookmarkAction)
        addToolBarBookmarkAction = QAction("&Add Bookmark to Tool Bar", self,
                                           triggered = self._addToolBarBookmark)
        self._bookmarkMenu.addAction(addToolBarBookmarkAction)
        self._bookmarkMenu.addSeparator()

        toolsMenu = self.menuBar().addMenu("&Tools")
        downloadAction = QAction("Open Downloads", self,
                                 triggered = DownloadWidget.openDownloadDirectory)
        toolsMenu.addAction(downloadAction)

        windowMenu = self.menuBar().addMenu("&Window")

        windowMenu.addAction(self._bookmarkDock.toggleViewAction())

        windowMenu.addSeparator()

        zoomInAction = QAction(QIcon.fromTheme("zoom-in"),
                               "Zoom In", self,
                               shortcut = QKeySequence(QKeySequence.ZoomIn),
                               triggered = self._zoomIn)
        windowMenu.addAction(zoomInAction)
        zoomOutAction = QAction(QIcon.fromTheme("zoom-out"),
                                "Zoom Out", self,
                                shortcut = QKeySequence(QKeySequence.ZoomOut),
                                triggered = self._zoomOut)
        windowMenu.addAction(zoomOutAction)

        resetZoomAction = QAction(QIcon.fromTheme("zoom-original"),
                                  "Reset Zoom", self,
                                  shortcut = "Ctrl+0",
                                  triggered = self._resetZoom)
        windowMenu.addAction(resetZoomAction)

        aboutMenu = self.menuBar().addMenu("&About")
        aboutAction = QAction("About Qt", self,
                              shortcut = QKeySequence(QKeySequence.HelpContents),
                              triggered=qApp.aboutQt)
        aboutMenu.addAction(aboutAction)

    def addBrowserTab(self):
        return self._tabWidget.addBrowserTab()

    def _closeCurrentTab(self):
        if self._tabWidget.count() > 1:
            self._tabWidget.closeCurrentTab()
        else:
            self.close()

    def closeEvent(self, event):
        mainWindows.remove(self)
        event.accept()

    def load(self):
        urlString = self._addressLineEdit.text().strip()
        if urlString:
            self.loadUrlString(urlString)

    def loadUrlString(self, urlS):
        url = QUrl.fromUserInput(urlS)
        if (url.isValid()):
            self.loadUrl(url)

    def loadUrl(self, url):
        self._tabWidget.load(url)

    def loadUrlInNewTab(self, url):
        self.addBrowserTab().load(url)

    def urlChanged(self, url):
        self._addressLineEdit.setText(url.toString())

    def _enabledChanged(self, webAction, enabled):
        action = self._actions[webAction]
        if action:
            action.setEnabled(enabled)

    def _addBookmark(self):
        index = self._tabWidget.currentIndex()
        if index >= 0:
            url = self._tabWidget.url()
            title = self._tabWidget.tabText(index)
            icon = self._tabWidget.tabIcon(index)
            self._bookmarkWidget.addBookmark(url, title, icon)

    def _addToolBarBookmark(self):
        index = self._tabWidget.currentIndex()
        if index >= 0:
            url = self._tabWidget.url()
            title = self._tabWidget.tabText(index)
            icon = self._tabWidget.tabIcon(index)
            self._bookmarkWidget.addToolBarBookmark(url, title, icon)

    def _zoomIn(self):
        newZoom = self._tabWidget.zoomFactor() * 1.5
        if (newZoom <= WebEngineView.maximumZoomFactor()):
            self._tabWidget.setZoomFactor(newZoom)
            self._updateZoomLabel()

    def _zoomOut(self):
        newZoom = self._tabWidget.zoomFactor() / 1.5
        if (newZoom >= WebEngineView.minimumZoomFactor()):
            self._tabWidget.setZoomFactor(newZoom)
            self._updateZoomLabel()

    def _resetZoom(self):
        self._tabWidget.setZoomFactor(1)
        self._updateZoomLabel()

    def _updateZoomLabel(self):
        percent = int(self._tabWidget.zoomFactor() * 100)
        self._zoomLabel.setText("{}%".format(percent))

    def _downloadRequested(self, item):
        # Remove old downloads before opening a new one
        for oldDownload in self.statusBar().children():
            if type(oldDownload).__name__ == 'DownloadWidget' and \
                oldDownload.state() != QWebEngineDownloadItem.DownloadInProgress:
                self.statusBar().removeWidget(oldDownload)
                del oldDownload

        item.accept()
        downloadWidget = DownloadWidget(item)
        downloadWidget.removeRequested.connect(self._removeDownloadRequested,
                                               Qt.QueuedConnection)
        self.statusBar().addWidget(downloadWidget)

    def _removeDownloadRequested(self):
            downloadWidget = self.sender()
            self.statusBar().removeWidget(downloadWidget)
            del downloadWidget

    def _showFind(self):
        if self._findToolBar is None:
            self._findToolBar = FindToolBar()
            self._findToolBar.find.connect(self._tabWidget.find)
            self.addToolBar(Qt.BottomToolBarArea, self._findToolBar)
        else:
            self._findToolBar.show()
        self._findToolBar.focusFind()

    def writeBookmarks(self):
        self._bookmarkWidget.writeBookmarks()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    mainWin = createMainWindow()
    initialUrls = sys.argv[1:]
    if not initialUrls:
        initialUrls.append('http://qt.io')
    for url in initialUrls:
        mainWin.loadUrlInNewTab(QUrl.fromUserInput(url))
    exitCode = app.exec_()
    mainWin.writeBookmarks()
    sys.exit(exitCode)
