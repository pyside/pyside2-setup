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

import unittest
from PySide2.QtWidgets import (QApplication, QComboBox, QGraphicsScene,
    QGraphicsRectItem)

from helper import UsesQApplication

class MyDiagram(QGraphicsScene):
    pass

class MyItem(QGraphicsRectItem):
    def itemChange(self, change, value):
        return value;

class Sequence(object):
    # Having the __getitem__ method on a class transform the Python
    # type to a PySequence.
    # Before the patch: aa75437f9119d997dd290471ac3e2cc88ca88bf1
    # "Fix QVariant conversions when using PySequences"
    # one could not use an object from this class, because internally
    # we were requiring that the PySequence was finite.
    def __getitem__(self, key):
        raise IndexError()

class QGraphicsSceneOnQVariantTest(UsesQApplication):
    """Test storage ot QGraphicsScene into QVariants"""
    def setUp(self):
        super(QGraphicsSceneOnQVariantTest, self).setUp()
        self.s = MyDiagram()
        self.i = MyItem()
        self.combo = QComboBox()

    def tearDown(self):
        del self.s
        del self.i
        del self.combo
        super(QGraphicsSceneOnQVariantTest, self).tearDown()

    def testIt(self):
        self.s.addItem(self.i)
        self.assertEqual(len(self.s.items()), 1)

    def testSequence(self):
        # PYSIDE-641
        self.combo.addItem("test", userData=Sequence())
        self.assertTrue(isinstance(self.combo.itemData(0), Sequence))

if __name__ == '__main__':
    unittest.main()
