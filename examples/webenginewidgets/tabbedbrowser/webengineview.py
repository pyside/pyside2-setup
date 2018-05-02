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

import sys
from PySide2.QtWebEngineWidgets import QWebEnginePage, QWebEngineView

from PySide2 import QtCore

_webActions = [QWebEnginePage.Back, QWebEnginePage.Forward,
               QWebEnginePage.Reload,
               QWebEnginePage.Undo, QWebEnginePage.Redo,
               QWebEnginePage.Cut, QWebEnginePage.Copy,
               QWebEnginePage.Paste, QWebEnginePage.SelectAll]

class WebEngineView(QWebEngineView):

    enabledChanged = QtCore.Signal(QWebEnginePage.WebAction, bool)

    @staticmethod
    def webActions():
        return _webActions

    @staticmethod
    def minimumZoomFactor():
        return 0.25

    @staticmethod
    def maximumZoomFactor():
        return 5

    def __init__(self, tabFactoryFunc, windowFactoryFunc):
        super(WebEngineView, self).__init__()
        self._tabFactoryFunc = tabFactoryFunc
        self._windowFactoryFunc = windowFactoryFunc
        page = self.page()
        self._actions = {}
        for webAction in WebEngineView.webActions():
            action = page.action(webAction)
            action.changed.connect(self._enabledChanged)
            self._actions[action] = webAction

    def isWebActionEnabled(self, webAction):
        return self.page().action(webAction).isEnabled()

    def createWindow(self, windowType):
        if windowType == QWebEnginePage.WebBrowserTab or windowType == QWebEnginePage.WebBrowserBackgroundTab:
            return self._tabFactoryFunc()
        return self._windowFactoryFunc()

    def _enabledChanged(self):
        action = self.sender()
        webAction = self._actions[action]
        self.enabledChanged.emit(webAction, action.isEnabled())
