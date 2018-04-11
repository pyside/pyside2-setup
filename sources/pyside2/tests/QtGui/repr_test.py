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

import PySide2
from PySide2.QtCore import QPoint
from PySide2.QtGui import QMatrix
from PySide2.QtGui import QMatrix2x2, QMatrix2x3, QMatrix2x4
from PySide2.QtGui import QMatrix3x2, QMatrix3x3, QMatrix3x4
from PySide2.QtGui import QMatrix4x2, QMatrix4x3, QMatrix4x4
from PySide2.QtGui import QVector2D, QVector3D, QVector4D
from PySide2.QtGui import QColor, QTransform, QKeySequence, QQuaternion
from PySide2.QtGui import QPolygon

class ReprCopyHelper:
    def testCopy(self):
        copy = eval(self.original.__repr__())
        self.assertTrue(copy is not self.original)
        self.assertEqual(copy, self.original)

class QTransformReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QTransform(1, 2, 3, 4, 5, 6, 7, 8)

class QKeySequenceReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QKeySequence("Ctrl+P")

class QQuaternionReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QQuaternion(1, 2, 3, 4)

class QVector2DReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QVector2D(1, 2)

class QVector3DReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QVector3D(1, 2, 3)

class QVector4DReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QVector4D(1, 2, 3, 4)

class QMatrixReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix(1, 2, 3, 4, 5, 6)


# Avoid these tests until get gcc fixed
# Related bug: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=43247
"""
class QMatrix2x2ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix2x2([1, 2, 3, 4])

class QMatrix2x3ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix2x3([1, 2, 3, 4, 5, 6])

class QMatrix2x4ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix2x4([1, 2, 3, 4, 5, 6, 7, 8])

class QMatrix3x2ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix3x2([1, 2, 3, 4, 5, 6])

class QMatrix3x3ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix3x3([1, 2, 3, 4, 5, 6, 7, 8, 9])

class QMatrix3x4ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix3x4([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12])

class QMatrix4x2ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix4x2([1, 2, 3, 4, 5, 6, 7, 8])

class QMatrix4x3ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix4x3([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12])

class QMatrix4x4ReprCopy(ReprCopyHelper, unittest.TestCase):
    def setUp(self):
        self.original = QMatrix4x4([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16])
"""

if __name__ == '__main__':
    unittest.main()
