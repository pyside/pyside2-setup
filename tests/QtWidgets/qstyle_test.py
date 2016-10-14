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
from helper import UsesQApplication

from PySide2.QtWidgets import QWidget, QLabel, QFontComboBox, QStyleFactory

class SetStyleTest(UsesQApplication):
    '''Tests setting the same QStyle for all objects in a UI hierarchy.'''

    def testSetStyle(self):
        '''All this test have to do is not break with some invalid Python wrapper.'''

        def setStyleHelper(widget, style):
            widget.setStyle(style)
            widget.setPalette(style.standardPalette())
            for child in widget.children():
                if isinstance(child, QWidget):
                    setStyleHelper(child, style)

        container = QWidget()
        # QFontComboBox is used because it has an QLineEdit created in C++ inside it,
        # and if the QWidget.setStyle(style) steals the ownership of the style
        # for the C++ originated widget everything will break.
        fontComboBox = QFontComboBox(container)
        label = QLabel(container)
        label.setText('Label')
        style = QStyleFactory.create(QStyleFactory.keys()[0])
        setStyleHelper(container, style)

if __name__ == '__main__':
    unittest.main()

