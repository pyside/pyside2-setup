#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: http://www.qt.io/licensing/
##
## This file is part of the PySide examples of the Qt Toolkit.
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

from functools import partial
import sys

from bookmarkwidget import BookmarkWidget
from webengineview import WebEngineView
from PySide2 import QtCore
from PySide2.QtCore import QPoint, Qt, QUrl
from PySide2.QtWidgets import (QAction, QMenu, QTabBar, QTabWidget)
from PySide2.QtWebEngineWidgets import (QWebEngineDownloadItem,
    QWebEnginePage, QWebEngineProfile)

class BrowserTabWidget(QTabWidget):

    urlChanged = QtCore.Signal(QUrl)
    enabledChanged = QtCore.Signal(QWebEnginePage.WebAction, bool)
    downloadRequested = QtCore.Signal(QWebEngineDownloadItem)

    def __init__(self, windowFactoryFunction):
        super(BrowserTabWidget, self).__init__()
        self.setTabsClosable(True)
        self._windowFactoryFunction = windowFactoryFunction
        self._webengineviews = []
        self.currentChanged.connect(self._currentChanged)
        self.tabCloseRequested.connect(self.handleTabCloseRequest)
        self._actionsEnabled = {}
        for webAction in WebEngineView.webActions():
            self._actionsEnabled[webAction] = False

        tabBar = self.tabBar()
        tabBar.setSelectionBehaviorOnRemove(QTabBar.SelectPreviousTab)
        tabBar.setContextMenuPolicy(Qt.CustomContextMenu)
        tabBar.customContextMenuRequested.connect(self._handleTabContextMenu)

    def addBrowserTab(self):
        factoryFunc = partial(BrowserTabWidget.addBrowserTab, self)
        webEngineView = WebEngineView(factoryFunc, self._windowFactoryFunction)
        index = self.count()
        self._webengineviews.append(webEngineView)
        title = 'Tab {}'.format(index + 1)
        self.addTab(webEngineView, title)
        page = webEngineView.page()
        page.titleChanged.connect(self._titleChanged)
        page.iconChanged.connect(self._iconChanged)
        page.profile().downloadRequested.connect(self._downloadRequested)
        webEngineView.urlChanged.connect(self._urlChanged)
        webEngineView.enabledChanged.connect(self._enabledChanged)
        self.setCurrentIndex(index)
        return webEngineView

    def load(self, url):
        index = self.currentIndex()
        if index >= 0 and url.isValid():
            self._webengineviews[index].setUrl(url)

    def find(self, needle, flags):
        index = self.currentIndex()
        if index >= 0:
            self._webengineviews[index].page().findText(needle, flags)

    def url(self):
        index = self.currentIndex()
        return self._webengineviews[index].url() if index >= 0 else QUrl()

    def _urlChanged(self, url):
        index = self.currentIndex()
        if index >= 0 and self._webengineviews[index] == self.sender():
                self.urlChanged.emit(url)

    def _titleChanged(self, title):
        index = self._indexOfPage(self.sender())
        if (index >= 0):
            self.setTabText(index, BookmarkWidget.shortTitle(title))

    def _iconChanged(self, icon):
        index = self._indexOfPage(self.sender())
        if (index >= 0):
            self.setTabIcon(index, icon)

    def _enabledChanged(self, webAction, enabled):
        index = self.currentIndex()
        if index >= 0 and self._webengineviews[index] == self.sender():
            self._checkEmitEnabledChanged(webAction, enabled)

    def _checkEmitEnabledChanged(self, webAction, enabled):
        if enabled != self._actionsEnabled[webAction]:
            self._actionsEnabled[webAction] = enabled
            self.enabledChanged.emit(webAction, enabled)

    def _currentChanged(self, index):
        self._updateActions(index)
        self.urlChanged.emit(self.url())

    def _updateActions(self, index):
        if index >= 0 and index < len(self._webengineviews):
            view = self._webengineviews[index]
            for webAction in WebEngineView.webActions():
                enabled = view.isWebActionEnabled(webAction)
                self._checkEmitEnabledChanged(webAction, enabled)

    def back(self):
        self._triggerAction(QWebEnginePage.Back)

    def forward(self):
        self._triggerAction(QWebEnginePage.Forward)

    def reload(self):
        self._triggerAction(QWebEnginePage.Reload)

    def undo(self):
        self._triggerAction(QWebEnginePage.Undo)

    def redo(self):
        self._triggerAction(QWebEnginePage.Redo)

    def cut(self):
        self._triggerAction(QWebEnginePage.Cut)

    def copy(self):
        self._triggerAction(QWebEnginePage.Copy)

    def paste(self):
        self._triggerAction(QWebEnginePage.Paste)

    def selectAll(self):
        self._triggerAction(QWebEnginePage.SelectAll)

    def zoomFactor(self):
        return self._webengineviews[0].zoomFactor() if self._webengineviews else 1.0

    def setZoomFactor(self, z):
        for w in self._webengineviews:
            w.setZoomFactor(z)

    def _handleTabContextMenu(self, point):
        index = self.tabBar().tabAt(point)
        if index < 0:
            return
        tabCount = len(self._webengineviews)
        contextMenu = QMenu()
        duplicateTabAction = contextMenu.addAction("Duplicate Tab")
        closeOtherTabsAction = contextMenu.addAction("Close Other Tabs")
        closeOtherTabsAction.setEnabled(tabCount > 1)
        closeTabsToTheRightAction = contextMenu.addAction("Close Tabs to the Right")
        closeTabsToTheRightAction.setEnabled(index < tabCount - 1)
        closeTabAction = contextMenu.addAction("&Close Tab")
        chosenAction = contextMenu.exec_(self.tabBar().mapToGlobal(point))
        if chosenAction == duplicateTabAction:
            currentUrl = self.url()
            self.addBrowserTab().load(currentUrl)
        elif chosenAction == closeOtherTabsAction:
            for t in range(tabCount - 1, -1, -1):
                if t != index:
                     self.handleTabCloseRequest(t)
        elif chosenAction == closeTabsToTheRightAction:
            for t in range(tabCount - 1, index, -1):
                self.handleTabCloseRequest(t)
        elif chosenAction == closeTabAction:
            self.handleTabCloseRequest(index)

    def handleTabCloseRequest(self, index):
        if (index >= 0 and self.count() > 1):
            self._webengineviews.remove(self._webengineviews[index])
            self.removeTab(index)

    def closeCurrentTab(self):
        self.handleTabCloseRequest(self.currentIndex())

    def _triggerAction(self, action):
        index = self.currentIndex()
        if index >= 0:
            self._webengineviews[index].page().triggerAction(action)

    def _indexOfPage(self, webPage):
        for p in range(0, len(self._webengineviews)):
            if (self._webengineviews[p].page() == webPage):
                return p
        return -1

    def _downloadRequested(self, item):
        self.downloadRequested.emit(item)
