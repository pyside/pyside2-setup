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

import json, os, warnings

from PySide2 import QtCore
from PySide2.QtCore import (QDir, QFileInfo, QModelIndex, QStandardPaths, Qt,
    QUrl)
from PySide2.QtGui import QIcon, QPixmap, QStandardItem, QStandardItemModel
from PySide2.QtWidgets import (QAction, QDockWidget, QMenu, QMessageBox,
    QToolBar, QTreeView, QWidget)

_urlRole = Qt.UserRole + 1

# Default bookmarks as an array of arrays which is the form
# used to read from/write to a .json bookmarks file
_defaultBookMarks = [
    ['Tool Bar'],
    ['http://qt.io', 'Qt', ':/qt-project.org/qmessagebox/images/qtlogo-64.png'],
    ['https://download.qt.io/snapshots/ci/pyside/', 'Downloads'],
    ['https://doc-snapshots.qt.io/qtforpython/', 'Documentation'],
    ['https://bugreports.qt.io/projects/PYSIDE/', 'Bug Reports'],
    ['https://www.python.org/', 'Python', None],
    ['https://wiki.qt.io/PySide2', 'Qt for Python', None],
    ['Other Bookmarks']
]

def _configDir():
    return '{}/QtForPythonBrowser'.format(
        QStandardPaths.writableLocation(QStandardPaths.ConfigLocation))

_bookmarkFile = 'bookmarks.json'

def _createFolderItem(title):
    result = QStandardItem(title)
    result.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
    return result

def _createItem(url, title, icon):
    result = QStandardItem(title)
    result.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
    result.setData(url, _urlRole)
    if icon is not None:
        result.setIcon(icon)
    return result

# Create the model from an array of arrays
def _createModel(parent, serializedBookmarks):
    result = QStandardItemModel(0, 1, parent)
    lastFolderItem = None
    for entry in serializedBookmarks:
        if len(entry) == 1:
            lastFolderItem = _createFolderItem(entry[0])
            result.appendRow(lastFolderItem)
        else:
            url = QUrl.fromUserInput(entry[0])
            title = entry[1]
            icon = QIcon(entry[2]) if len(entry) > 2 and entry[2] else None
            lastFolderItem.appendRow(_createItem(url, title, icon))
    return result

# Serialize model into an array of arrays, writing out the icons
# into .png files under directory in the process
def _serializeModel(model, directory):
    result = []
    folderCount = model.rowCount()
    for f in range(0, folderCount):
        folderItem = model.item(f)
        result.append([folderItem.text()])
        itemCount = folderItem.rowCount()
        for i in range(0, itemCount):
            item = folderItem.child(i)
            entry = [item.data(_urlRole).toString(), item.text()]
            icon = item.icon()
            if not icon.isNull():
                iconSizes = icon.availableSizes()
                largestSize = iconSizes[len(iconSizes) - 1]
                iconFileName = '{}/icon{:02}_{:02}_{}.png'.format(directory,
                               f, i, largestSize.width())
                icon.pixmap(largestSize).save(iconFileName, 'PNG')
                entry.append(iconFileName)
            result.append(entry)
    return result

# Bookmarks as a tree view to be used in a dock widget with
# functionality to persist and populate tool bars and menus.
class BookmarkWidget(QTreeView):

    openBookmark = QtCore.Signal(QUrl)
    openBookmarkInNewTab = QtCore.Signal(QUrl)
    changed = QtCore.Signal()

    def __init__(self):
        super(BookmarkWidget, self).__init__()
        self.setRootIsDecorated(False)
        self.setUniformRowHeights(True)
        self.setHeaderHidden(True)
        self._model = _createModel(self, self._readBookmarks())
        self.setModel(self._model)
        self.expandAll()
        self.activated.connect(self._activated)
        self._model.rowsInserted.connect(self._changed)
        self._model.rowsRemoved.connect(self._changed)
        self._model.dataChanged.connect(self._changed)
        self._modified = False

    def _changed(self):
        self._modified = True
        self.changed.emit()

    def _activated(self, index):
        item = self._model.itemFromIndex(index)
        self.openBookmark.emit(item.data(_urlRole))

    def _actionActivated(self, index):
        action = self.sender()
        self.openBookmark.emit(action.data())

    def _toolBarItem(self):
        return self._model.item(0, 0)

    def _otherItem(self):
        return self._model.item(1, 0)

    def addBookmark(self, url, title, icon):
        self._otherItem().appendRow(_createItem(url, title, icon))

    def addToolBarBookmark(self, url, title, icon):
        self._toolBarItem().appendRow(_createItem(url, title, icon))

    # Synchronize the bookmarks under parentItem to a targetObject
    # like QMenu/QToolBar, which has a list of actions. Update
    # the existing actions, append new ones if needed or hide
    # superfluous ones
    def _populateActions(self, parentItem, targetObject, firstAction):
        existingActions = targetObject.actions()
        existingActionCount = len(existingActions)
        a = firstAction
        rowCount = parentItem.rowCount()
        for r in range(0, rowCount):
            item = parentItem.child(r)
            title = item.text()
            icon = item.icon()
            url = item.data(_urlRole)
            if a < existingActionCount:
                action = existingActions[a]
                if (title != action.toolTip()):
                    action.setText(BookmarkWidget.shortTitle(title))
                    action.setIcon(icon)
                    action.setToolTip(title)
                    action.setData(url)
                    action.setVisible(True)
            else:
                action = targetObject.addAction(icon, BookmarkWidget.shortTitle(title))
                action.setToolTip(title)
                action.setData(url)
                action.triggered.connect(self._actionActivated)
            a = a + 1
        while a < existingActionCount:
            existingActions[a].setVisible(False)
            a = a + 1

    def populateToolBar(self, toolBar):
        self._populateActions(self._toolBarItem(), toolBar, 0)

    def populateOther(self, menu, firstAction):
        self._populateActions(self._otherItem(), menu, firstAction)

    def _currentItem(self):
        index = self.currentIndex()
        if index.isValid():
            item = self._model.itemFromIndex(index)
            if item.parent(): # Exclude top level items
                return item
        return None

    def contextMenuEvent(self, event):
        contextMenu = QMenu()
        openInNewTabAction = contextMenu.addAction("Open in New Tab")
        removeAction = contextMenu.addAction("Remove...")
        currentItem = self._currentItem()
        openInNewTabAction.setEnabled(currentItem is not None)
        removeAction.setEnabled(currentItem is not None)
        chosenAction = contextMenu.exec_(event.globalPos())
        if chosenAction == openInNewTabAction:
            self.openBookmarkInNewTab.emit(currentItem.data(_urlRole))
        elif chosenAction == removeAction:
            self._removeItem(currentItem)

    def _removeItem(self, item):
        button = QMessageBox.question(self, "Remove",
            "Would you like to remove \"{}\"?".format(item.text()),
            QMessageBox.Yes | QMessageBox.No)
        if button == QMessageBox.Yes:
            item.parent().removeRow(item.row())

    def writeBookmarks(self):
        if not self._modified:
            return
        dirPath = _configDir()
        nativeDirPath = QDir.toNativeSeparators(dirPath)
        dir = QFileInfo(dirPath)
        if not dir.isDir():
            print('Creating {}...'.format(nativeDirPath))
            if not QDir(dir.absolutePath()).mkpath(dir.fileName()):
                warnings.warn('Cannot create {}.'.format(nativeDirPath),
                              RuntimeWarning)
                return
        serializedModel = _serializeModel(self._model, dirPath)
        bookmarkFileName = os.path.join(nativeDirPath, _bookmarkFile)
        print('Writing {}...'.format(bookmarkFileName))
        with open(bookmarkFileName, 'w') as bookmarkFile:
            json.dump(serializedModel, bookmarkFile, indent = 4)

    def _readBookmarks(self):
        bookmarkFileName = os.path.join(QDir.toNativeSeparators(_configDir()),
                                        _bookmarkFile)
        if os.path.exists(bookmarkFileName):
            print('Reading {}...'.format(bookmarkFileName))
            return json.load(open(bookmarkFileName))
        return _defaultBookMarks

    # Return a short title for a bookmark action,
    # "Qt | Cross Platform.." -> "Qt"
    @staticmethod
    def shortTitle(t):
        i = t.find(' | ')
        if i == -1:
            i = t.find(' - ')
        return t[0:i] if i != -1 else t
