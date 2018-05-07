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

'''Test cases for QQuickView'''

import unittest

from helper import adjust_filename, TimedQApplication

from PySide2.QtCore import QUrl, QObject, Property, Slot
from PySide2.QtQuick import QQuickView

class MyObject(QObject):
    def __init__(self, text, parent=None):
        QObject.__init__(self, parent)
        self._text = text

    def getText(self):
        return self._text


    @Slot(str)
    def qmlText(self, text):
        self._qmlText = text

    title = Property(str, getText)


class TestQQuickView(TimedQApplication):

    def testQQuickViewList(self):
        view = QQuickView()

        dataList = ["Item 1", "Item 2", "Item 3", "Item 4"]

        ctxt = view.rootContext()
        ctxt.setContextProperty("myModel", dataList)

        url = QUrl.fromLocalFile(adjust_filename('view.qml', __file__))
        view.setSource(url)
        view.show()

        self.assertEqual(view.status(), QQuickView.Ready)


    def testModelExport(self):
        view = QQuickView()
        dataList = [MyObject("Item 1"), MyObject("Item 2"), MyObject("Item 3"), MyObject("Item 4")]

        ctxt = view.rootContext()
        ctxt.setContextProperty("myModel", dataList)

        url = QUrl.fromLocalFile(adjust_filename('viewmodel.qml', __file__))
        view.setSource(url)
        view.show()

        self.assertEqual(view.status(), QQuickView.Ready)


if __name__ == '__main__':
    unittest.main()
