#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

''' Test bug 662: http://bugs.openbossa.org/show_bug.cgi?id=662'''

import unittest
from PySide2.QtGui import QTextCharFormat
from PySide2.QtWidgets import QTextEdit, QApplication
import sys

class testQTextBlock(unittest.TestCase):
    def tesIterator(self):
        edit = QTextEdit()
        cursor = edit.textCursor()
        fmt = QTextCharFormat()
        frags = []
        for i in range(10):
            fmt.setFontPointSize(i+10)
            frags.append("block%d"%i)
            cursor.insertText(frags[i], fmt)

        doc = edit.document()
        block = doc.begin()

        index = 0
        for i in block:
            self.assertEqual(i.fragment().text(), frags[index])
            index += 1

if __name__ == '__main__':
    app = QApplication(sys.argv)
    unittest.main()
