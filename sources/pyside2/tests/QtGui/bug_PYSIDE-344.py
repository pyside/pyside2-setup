#!/usr/bin/python

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

'''Test cases for PYSIDE-344, imul/idiv are used instead of mul/div, modifying the argument passed in'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import *
from PySide2.QtGui import *

def testList():
    return [QPoint(10, 10), QPointF(1, 1), QSize(10, 10), QSizeF(1, 1),
            QMargins(10, 10, 10, 10),
            QTransform(), QMatrix4x4(),
            QVector2D(1, 1), QVector3D(1, 1, 1), QVector4D(1, 1, 1, 1),
            QQuaternion(1, 1, 1, 1)]

class TestMulDiv(unittest.TestCase):

    def testMultiplication(self):
        fails = ''
        for a in testList():
            mul = (a * 2)
        if a == mul:
            fails += ' ' + type(a).__name__
        self.assertEqual(fails, '')

    def testDivision(self):
        fails = ''
        for a in testList():
            div = (a * 2)
            if a == div:
                fails += ' ' + type(a).__name__
        self.assertEqual(fails, '')

if __name__ == '__main__':
    unittest.main()
