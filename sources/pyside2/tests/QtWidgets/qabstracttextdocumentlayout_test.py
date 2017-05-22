#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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

import unittest
import py3kcompat as py3k

from PySide2.QtCore import QSizeF, QTimer
from PySide2.QtGui import QTextFormat, QTextCharFormat, QPyTextObject
from PySide2.QtWidgets import QTextEdit
from helper import UsesQApplication

class Foo(QPyTextObject):
    called = False

    def intrinsicSize(self, doc, posInDocument, format):
        Foo.called = True
        return QSizeF(10, 10)

    def drawObject(self, painter, rect, doc, posInDocument, format):
        pass

class QAbstractTextDocumentLayoutTest(UsesQApplication):

    objectType = QTextFormat.UserObject + 1

    def foo(self):
        fmt = QTextCharFormat()
        fmt.setObjectType(QAbstractTextDocumentLayoutTest.objectType)

        cursor = self.textEdit.textCursor()
        cursor.insertText(py3k.unichr(0xfffc), fmt)
        self.textEdit.setTextCursor(cursor)
        self.textEdit.close()

    def testIt(self):

        self.textEdit = QTextEdit()
        self.textEdit.show()

        interface = Foo()
        self.textEdit.document().documentLayout().registerHandler(QAbstractTextDocumentLayoutTest.objectType, interface)

        QTimer.singleShot(0, self.foo)
        self.app.exec_()

        self.assertTrue(Foo.called)

if __name__ == "__main__":
    unittest.main()

