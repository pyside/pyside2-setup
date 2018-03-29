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

import sys
from PySide2 import QtCore
from PySide2.QtCore import QDir, QFileInfo, QStandardPaths, Qt, QUrl
from PySide2.QtGui import QDesktopServices
from PySide2.QtWidgets import (QAction, QLabel, QMenu, QProgressBar,
    QStyleFactory, QWidget)
from PySide2.QtWebEngineWidgets import QWebEngineDownloadItem

# A QProgressBar with context menu for displaying downloads in a QStatusBar.
class DownloadWidget(QProgressBar):

    finished = QtCore.Signal()
    removeRequested = QtCore.Signal()

    def __init__(self, downloadItem):
        super(DownloadWidget, self).__init__()
        self._downloadItem = downloadItem
        downloadItem.finished.connect(self._finished)
        downloadItem.downloadProgress.connect(self._downloadProgress)
        downloadItem.stateChanged.connect(self._updateToolTip())
        path = downloadItem.path()
        self.setMaximumWidth(300)
        # Shorten 'PySide2-5.11.0a1-5.11.0-cp36-cp36m-linux_x86_64.whl'...
        description = QFileInfo(path).fileName()
        descriptionLength = len(description)
        if descriptionLength > 30:
            description = '{}...{}'.format(description[0:10], description[descriptionLength - 10:])
        self.setFormat('{} %p%'.format(description))
        self.setOrientation(Qt.Horizontal)
        self.setMinimum(0)
        self.setValue(0)
        self.setMaximum(100)
        self._updateToolTip()
        # Force progress bar text to be shown on macoS by using 'fusion' style
        if sys.platform == 'darwin':
            self.setStyle(QStyleFactory.create('fusion'))

    @staticmethod
    def openFile(file):
        QDesktopServices.openUrl(QUrl.fromLocalFile(file))

    @staticmethod
    def openDownloadDirectory():
        path = QStandardPaths.writableLocation(QStandardPaths.DownloadLocation)
        DownloadWidget.openFile(path)

    def state(self):
        return self._downloadItem.state()

    def _updateToolTip(self):
        path = self._downloadItem.path()
        toolTip = "{}\n{}".format(self._downloadItem.url().toString(),
            QDir.toNativeSeparators(path))
        totalBytes = self._downloadItem.totalBytes()
        if totalBytes > 0:
            toolTip += "\n{}K".format(totalBytes / 1024)
        state = self.state()
        if state == QWebEngineDownloadItem.DownloadRequested:
            toolTip += "\n(requested)"
        elif state == QWebEngineDownloadItem.DownloadInProgress:
            toolTip += "\n(downloading)"
        elif state == QWebEngineDownloadItem.DownloadCompleted:
            toolTip += "\n(completed)"
        elif state == QWebEngineDownloadItem.DownloadCancelled:
            toolTip += "\n(cancelled)"
        else:
            toolTip += "\n(interrupted)"
        self.setToolTip(toolTip)

    def _downloadProgress(self, bytesReceived, bytesTotal):
        self.setValue(int(100 * bytesReceived / bytesTotal))

    def _finished(self):
        self._updateToolTip()
        self.finished.emit()

    def _launch(self):
        DownloadWidget.openFile(self._downloadItem.path())

    def mouseDoubleClickEvent(self, event):
        if self.state() == QWebEngineDownloadItem.DownloadCompleted:
            self._launch()

    def contextMenuEvent(self, event):
        state = self.state()
        contextMenu = QMenu()
        launchAction = contextMenu.addAction("Launch")
        launchAction.setEnabled(state == QWebEngineDownloadItem.DownloadCompleted)
        showInFolderAction = contextMenu.addAction("Show in Folder")
        showInFolderAction.setEnabled(state == QWebEngineDownloadItem.DownloadCompleted)
        cancelAction = contextMenu.addAction("Cancel")
        cancelAction.setEnabled(state == QWebEngineDownloadItem.DownloadInProgress)
        removeAction = contextMenu.addAction("Remove")
        removeAction.setEnabled(state != QWebEngineDownloadItem.DownloadInProgress)

        chosenAction = contextMenu.exec_(event.globalPos())
        if chosenAction == launchAction:
            self._launch()
        elif chosenAction == showInFolderAction:
            DownloadWidget.openFile(QFileInfo(self._downloadItem.path()).absolutePath())
        elif chosenAction == cancelAction:
            self._downloadItem.cancel()
        elif chosenAction == removeAction:
            self.removeRequested.emit()
