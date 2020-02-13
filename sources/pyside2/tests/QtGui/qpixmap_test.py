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

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper import UsesQApplication
from PySide2.QtGui import *
from PySide2.QtCore import *

class QPixmapTest(UsesQApplication):
    def testQVariantConstructor(self):
        obj = QObject()
        pixmap = QPixmap()
        obj.setProperty('foo', pixmap)
        self.assertEqual(type(obj.property('foo')), QPixmap)

    def testQSizeConstructor(self):
        pixmap = QPixmap(QSize(10,20))
        self.assertTrue(pixmap.size().height(), 20)

    def testQStringConstructor(self):
        pixmap = QPixmap("Testing!")

    def testQPixmapLoadFromDataWithQFile(self):
        f = QFile(os.path.join(os.path.dirname(__file__), 'sample.png'))
        self.assertTrue(f.open(QIODevice.ReadOnly))
        data = f.read(f.size())
        f.close()
        pixmap = QPixmap()
        self.assertTrue(pixmap.loadFromData(data))

    def testQPixmapLoadFromDataWithPython(self):
        data = open(os.path.join(os.path.dirname(__file__),'sample.png'),'rb').read()
        pixmap = QPixmap()
        self.assertTrue(pixmap.loadFromData(data))


class QPixmapToImage(UsesQApplication):

    def testFilledImage(self):
        '''QPixmap.fill + toImage + image.pixel'''
        pixmap = QPixmap(100, 200)
        pixmap.fill(Qt.red) # Default Qt.white

        self.assertEqual(pixmap.height(), 200)
        self.assertEqual(pixmap.width(), 100)

        image = pixmap.toImage()

        self.assertEqual(image.height(), 200)
        self.assertEqual(image.width(), 100)

        pixel = image.pixel(10,10)
        self.assertEqual(pixel, QColor(Qt.red).rgba())


if __name__ == '__main__':
    unittest.main()

