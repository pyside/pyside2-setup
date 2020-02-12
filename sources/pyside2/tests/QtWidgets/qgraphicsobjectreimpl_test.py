#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

''' Test cases related to QGraphicsItem and subclasses'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtWidgets import QGraphicsObject, QGraphicsWidget
from PySide2.QtCore import QRectF

from helper.usesqapplication import UsesQApplication

class GObjA(QGraphicsObject):
    def paint(self, *args):
        pass

    def boundingRect(self):
        return QRectF()

    def itemChange(self, *args):
        return QGraphicsObject.itemChange(self, *args)

class GObjB(QGraphicsObject):
    def paint(self, *args):
        pass

    def boundingRect(self):
        return QRectF()

class QGraphicsObjectReimpl(UsesQApplication):
    '''Test case for reimplementing QGraphicsObject'''

    def testReimplementationTypes(self):
        w = QGraphicsWidget()

        # PYSIDE-86:
        #   This case failed because GObjA was reimplementing
        #   the method itemChange() from QGraphicsItem,
        #   and then the QVariant was not associated with
        #   a QGraphicsItem but a QObjectItem because the base
        #   class was a QObject.
        gobjA = GObjA()
        gobjA.setParentItem(w)
        self.assertIs(type(w), type(gobjA.parentItem()))

        gobjB = GObjB()
        gobjB.setParentItem(w)
        self.assertIs(type(w), type(gobjB.parentItem()))

if __name__ == '__main__':
    unittest.main()
