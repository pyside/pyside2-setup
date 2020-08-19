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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from helper.helper import adjust_filename

from PySide2.QtCore import Property, QObject, QTimer, QUrl
from PySide2.QtGui import QGuiApplication, QPen, QColor, QPainter
from PySide2.QtQml import qmlRegisterType, qmlRegisterUncreatableType, ListProperty
from PySide2.QtQuick import QQuickView, QQuickItem, QQuickPaintedItem

noCreationReason = 'Cannot create an item of type: Uncreatable (expected)';

class Uncreatable(QObject):
    def __init__(self, parent = None):
        QObject.__init__(self, parent)
        self._name = 'uncreatable'

    def getName(self):
        return self._name

    def setName(self, value):
        self._name = value

    name = Property(str, getName, setName)

class PieSlice (QQuickPaintedItem):
    def __init__(self, parent = None):
        QQuickPaintedItem.__init__(self, parent)
        self._color = QColor()
        self._fromAngle = 0
        self._angleSpan = 0

    def getColor(self):
        return self._color

    def setColor(self, value):
        self._color = value

    def getFromAngle(self):
        return self._angle

    def setFromAngle(self, value):
        self._fromAngle = value

    def getAngleSpan(self):
        return self._angleSpan

    def setAngleSpan(self, value):
        self._angleSpan = value

    color = Property(QColor, getColor, setColor)
    fromAngle = Property(int, getFromAngle, setFromAngle)
    angleSpan = Property(int, getAngleSpan, setAngleSpan)

    def paint(self, painter):
        global paintCalled
        pen = QPen(self._color, 2)
        painter.setPen(pen);
        painter.setRenderHints(QPainter.Antialiasing, True);
        painter.drawPie(self.boundingRect(), self._fromAngle * 16, self._angleSpan * 16);
        paintCalled = True

class PieChart (QQuickItem):
    def __init__(self, parent = None):
        QQuickItem.__init__(self, parent)
        self._name = ''
        self._slices = []

    def getName(self):
        return self._name

    def setName(self, value):
        self._name = value

    name = Property(str, getName, setName)

    def appendSlice(self, _slice):
        global appendCalled
        _slice.setParentItem(self)
        self._slices.append(_slice)
        appendCalled = True

    slices = ListProperty(PieSlice, append=appendSlice)

appendCalled = False
paintCalled = False

class TestQmlSupport(unittest.TestCase):

    def testIt(self):
        app = QGuiApplication([])

        qmlRegisterType(PieChart, 'Charts', 1, 0, 'PieChart')
        self.assertTrue(qmlRegisterType(PieSlice, "Charts", 1, 0, "PieSlice") > 0);
        self.assertTrue(qmlRegisterUncreatableType(Uncreatable, 'Charts', 1, 0,
                                                   'Uncreatable', noCreationReason) > 0);

        view = QQuickView()
        view.setSource(QUrl.fromLocalFile(adjust_filename('registertype.qml', __file__)))
        view.show()
        QTimer.singleShot(250, view.close)
        app.exec_()
        self.assertTrue(appendCalled)
        self.assertTrue(paintCalled)

        # Check that the uncreatable item produces the correct error
        view.setSource(QUrl.fromLocalFile(adjust_filename('registeruncreatable.qml', __file__)))
        self.assertEqual(view.status(), QQuickView.Error)
        errorFound = False
        for e in view.errors():
            if noCreationReason in e.toString():
                errorFound = True
                break
        self.assertTrue(errorFound)


if __name__ == '__main__':
    unittest.main()
