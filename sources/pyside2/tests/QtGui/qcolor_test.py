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
import colorsys
import PySide2

from PySide2.QtCore import Qt
from PySide2.QtGui import QColor


class QColorGetTest(unittest.TestCase):

    def setUp(self):
        self.color = QColor(20, 40, 60, 80)

    def testGetRgb(self):
        self.assertEqual(self.color.getRgb(), (20, 40, 60, 80))

    def testGetHslF(self):
        hls = colorsys.rgb_to_hls(20.0/255, 40.0/255, 60.0/255)
        hsla = hls[0], hls[2], hls[1], self.color.alphaF()
        for x, y in zip(self.color.getHslF(), hsla): # Due to rounding problems
            self.assertTrue(x - y < 1/100000.0)

    def testGetHsv(self):
        hsv = colorsys.rgb_to_hsv(20.0/255, 40.0/255, 60.0/255)
        hsva = int(hsv[0]*360.0), int(hsv[1]*255), int(hsv[2]*256), self.color.alpha()
        self.assertEqual(self.color.getHsv(), hsva)

    def testGetCmyk(self): # not supported by colorsys
        self.assertEqual(self.color.getCmyk(), (170, 85, 0, 195, 80))

    def testGetCmykF(self): # not supported by colorsys
        for x, y in zip(self.color.getCmykF(), (170/255.0, 85/255.0, 0, 195/255.0, 80/255.0)):
            self.assertTrue(x - y < 1/10000.0)


class QColorQRgbConstructor(unittest.TestCase):
    '''QColor(QRgb) constructor'''
    # Affected by bug #170 - QColor(QVariant) coming before QColor(uint)
    # in overload sorting

    def testBasic(self):
        '''QColor(QRgb)'''
        color = QColor(255, 0, 0)
        #QRgb format #AARRGGBB
        rgb = 0x00FF0000
        self.assertEqual(QColor(rgb), color)


class QColorEqualGlobalColor(unittest.TestCase):

    def testEqualGlobalColor(self):
        '''QColor == Qt::GlobalColor'''
        self.assertEqual(QColor(255, 0, 0), Qt.red)


class QColorCopy(unittest.TestCase):

    def testDeepCopy(self):
        '''QColor deepcopy'''

        from copy import deepcopy

        original = QColor(0, 0, 255)
        copy = deepcopy([original])[0]

        self.assertTrue(original is not copy)
        self.assertEqual(original, copy)
        del original
        self.assertEqual(copy, QColor(0, 0, 255))

    def testEmptyCopy(self):
        from copy import deepcopy

        original = QColor()
        copy = deepcopy([original])[0]
        self.assertTrue(original is not copy)
        self.assertEqual(original, copy)
        del original
        self.assertEqual(copy, QColor())


class QColorRepr(unittest.TestCase):
    def testReprFunction(self):
        c = QColor(100, 120, 200)
        c2 = eval(c.__repr__())
        self.assertEqual(c, c2)

    def testStrFunction(self):
        c = QColor('red')
        c2 = eval(c.__str__())
        self.assertEqual(c, c2)

if __name__ == '__main__':
    unittest.main()
