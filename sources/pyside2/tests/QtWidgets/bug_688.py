#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

''' Test bug 688: http://bugs.openbossa.org/show_bug.cgi?id=688'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from helper.usesqapplication import UsesQApplication
from PySide2.QtGui import QTextFrame, QTextCursor, QTextCharFormat, QFont, QTextFrameFormat
from PySide2.QtWidgets import QTextEdit

class BugTest(UsesQApplication):
    def testCase(self):
        editor = QTextEdit()
        cursor = QTextCursor(editor.textCursor())
        cursor.movePosition(QTextCursor.Start)

        mainFrame = cursor.currentFrame()

        plainCharFormat = QTextCharFormat()
        boldCharFormat = QTextCharFormat()
        boldCharFormat.setFontWeight(QFont.Bold);
        cursor.insertText("""
                          Text documents are represented by the
                          QTextDocument class, rather than by QString objects.
                          Each QTextDocument object contains information about
                          the document's internal representation, its structure,
                          and keeps track of modifications to provide undo/redo
                          facilities. This approach allows features such as the
                          layout management to be delegated to specialized
                          classes, but also provides a focus for the framework.""",
                          plainCharFormat)

        frameFormat = QTextFrameFormat()
        frameFormat.setMargin(32)
        frameFormat.setPadding(8)
        frameFormat.setBorder(4)
        cursor.insertFrame(frameFormat)

        cursor.insertText("""
                          Documents are either converted from external sources
                          or created from scratch using Qt. The creation process
                          can done by an editor widget, such as QTextEdit, or by
                          explicit calls to the Scribe API.""",
                          boldCharFormat)

        cursor = mainFrame.lastCursorPosition()
        cursor.insertText("""
                          There are two complementary ways to visualize the
                          contents of a document: as a linear buffer that is
                          used by editors to modify the contents, and as an
                          object hierarchy containing structural information
                          that is useful to layout engines. In the hierarchical
                          model, the objects generally correspond to visual
                          elements such as frames, tables, and lists. At a lower
                          level, these elements describe properties such as the
                          style of text used and its alignment. The linear
                          representation of the document is used for editing and
                          manipulation of the document's contents.""",
                          plainCharFormat)


        frame = cursor.currentFrame()

        items = []

        #test iterator
        for i in frame:
            items.append(i)

        #test __iadd__
        b = frame.begin()
        i = 0
        while not b.atEnd():
            self.assertEqual(b, items[i])
            self.assertTrue(b.parentFrame(), items[i].parentFrame())
            b.__iadd__(1)
            i += 1

        #test __isub__
        b = frame.end()
        i = 0
        while i > 0:
            self.assertEqual(b, items[i])
            self.assertTrue(b.parentFrame(), items[i].parentFrame())
            b.__isub__(1)
            i -= 1



if __name__ == '__main__':
    unittest.main()
