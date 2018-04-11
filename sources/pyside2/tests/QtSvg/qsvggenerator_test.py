#!/usr/bin/python

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

import unittest
from sys import getrefcount
from PySide2.QtCore import QBuffer
from PySide2.QtSvg import QSvgGenerator

class QSvgGeneratorTest(unittest.TestCase):

    def testRefCountOfTOutputDevice(self):
        generator = QSvgGenerator()
        iodevice1 = QBuffer()
        refcount1 = getrefcount(iodevice1)

        generator.setOutputDevice(iodevice1)

        self.assertEqual(generator.outputDevice(), iodevice1)
        self.assertEqual(getrefcount(generator.outputDevice()), refcount1 + 1)

        iodevice2 = QBuffer()
        refcount2 = getrefcount(iodevice2)

        generator.setOutputDevice(iodevice2)

        self.assertEqual(generator.outputDevice(), iodevice2)
        self.assertEqual(getrefcount(generator.outputDevice()), refcount2 + 1)
        self.assertEqual(getrefcount(iodevice1), refcount1)

        del generator

        self.assertEqual(getrefcount(iodevice2), refcount2)

if __name__ == '__main__':
    unittest.main()

