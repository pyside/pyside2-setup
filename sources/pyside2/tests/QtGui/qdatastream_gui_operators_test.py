# -*- coding: utf-8 -*-

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
import sys

from PySide2.QtCore import QDataStream, QByteArray, QIODevice, Qt
from PySide2.QtGui import QPixmap, QColor

from helper import UsesQApplication

class QPixmapQDatastream(UsesQApplication):
    '''QDataStream <<>> QPixmap'''

    def setUp(self):
        super(QPixmapQDatastream, self).setUp()
        self.source_pixmap = QPixmap(100, 100)
        self.source_pixmap.fill(Qt.red)
        self.output_pixmap = QPixmap()
        self.buffer = QByteArray()
        self.read_stream = QDataStream(self.buffer, QIODevice.ReadOnly)
        self.write_stream = QDataStream(self.buffer, QIODevice.WriteOnly)

    def testStream(self):
        self.write_stream << self.source_pixmap

        self.read_stream >> self.output_pixmap

        image = self.output_pixmap.toImage()
        pixel = image.pixel(10,10)
        self.assertEqual(pixel, QColor(Qt.red).rgba())
        self.assertEqual(self.source_pixmap.toImage(), self.output_pixmap.toImage())


if __name__ == '__main__':
    unittest.main()
