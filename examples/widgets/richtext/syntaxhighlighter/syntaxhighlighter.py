
############################################################################
##
## Copyright (C) 2013 Riverbank Computing Limited.
## Copyright (C) 2016 The Qt Company Ltd.
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

"""PySide2 port of the widgets/richtext/syntaxhighlighter example from Qt v5.x"""

import sys
import re
from PySide2.QtCore import (QFile, Qt, QTextStream)
from PySide2.QtGui import (QColor, QFont, QKeySequence, QSyntaxHighlighter,
    QTextCharFormat)
from PySide2.QtWidgets import (qApp, QApplication, QFileDialog, QMainWindow,
    QPlainTextEdit)

import syntaxhighlighter_rc


class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)

        self.highlighter = Highlighter()

        self.setupFileMenu()
        self.setupEditor()

        self.setCentralWidget(self.editor)
        self.setWindowTitle(self.tr("Syntax Highlighter"))

    def newFile(self):
        self.editor.clear()

    def openFile(self, path = ""):
        fileName = path

        if not fileName:
            fileName, _ = QFileDialog.getOpenFileName(self, self.tr("Open File"), "",
                                                      "qmake Files (*.pro *.prf *.pri)")

        if fileName:
            inFile = QFile(fileName)
            if inFile.open(QFile.ReadOnly | QFile.Text):
                stream = QTextStream(inFile)
                self.editor.setPlainText(stream.readAll())

    def setupEditor(self):
        variableFormat = QTextCharFormat()
        variableFormat.setFontWeight(QFont.Bold)
        variableFormat.setForeground(Qt.blue)
        self.highlighter.addMapping("\\b[A-Z_]+\\b", variableFormat)

        singleLineCommentFormat = QTextCharFormat()
        singleLineCommentFormat.setBackground(QColor("#77ff77"))
        self.highlighter.addMapping("#[^\n]*", singleLineCommentFormat)

        quotationFormat = QTextCharFormat()
        quotationFormat.setBackground(Qt.cyan)
        quotationFormat.setForeground(Qt.blue)
        self.highlighter.addMapping("\".*\"", quotationFormat)

        functionFormat = QTextCharFormat()
        functionFormat.setFontItalic(True)
        functionFormat.setForeground(Qt.blue)
        self.highlighter.addMapping("\\b[a-z0-9_]+\\(.*\\)", functionFormat)

        font = QFont()
        font.setFamily("Courier")
        font.setFixedPitch(True)
        font.setPointSize(10)

        self.editor = QPlainTextEdit()
        self.editor.setFont(font)
        self.highlighter.setDocument(self.editor.document())

    def setupFileMenu(self):
        fileMenu = self.menuBar().addMenu(self.tr("&File"))

        newFileAct = fileMenu.addAction(self.tr("&New..."))
        newFileAct.setShortcut(QKeySequence(QKeySequence.New))
        newFileAct.triggered.connect(self.newFile)

        openFileAct = fileMenu.addAction(self.tr("&Open..."))
        openFileAct.setShortcut(QKeySequence(QKeySequence.Open))
        openFileAct.triggered.connect(self.openFile)

        quitAct = fileMenu.addAction(self.tr("E&xit"))
        quitAct.setShortcut(QKeySequence(QKeySequence.Quit))
        quitAct.triggered.connect(self.close)

        helpMenu = self.menuBar().addMenu("&Help")
        helpMenu.addAction("About &Qt", qApp.aboutQt)


class Highlighter(QSyntaxHighlighter):
    def __init__(self, parent=None):
        QSyntaxHighlighter.__init__(self, parent)

        self.mappings = {}

    def addMapping(self, pattern, format):
        self.mappings[pattern] = format

    def highlightBlock(self, text):
        for pattern in self.mappings:
            for m in re.finditer(pattern,text):
                s,e = m.span()
                self.setFormat(s, e - s, self.mappings[pattern])

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.resize(640, 512)
    window.show()
    window.openFile(":/examples/example")
    sys.exit(app.exec_())
