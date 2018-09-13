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

from PySide2.QtCore import QPoint
from PySide2.QtGui import QMatrix, QMatrix2x2, QMatrix4x4


def qpointTimesQMatrix(point, matrix):
    '''As seen in "QPoint QMatrix::map(const QPoint &p) const" C++ implementation.'''
    return QPoint(matrix.m11() * point.x() + matrix.m21() * point.y() + matrix.dx(),
                  matrix.m12() * point.x() + matrix.m22() * point.y() + matrix.dy())

class QMatrixTest(unittest.TestCase):

    def testMatrix(self):
        matrix = QMatrix(11, 12, 21, 22, 100, 200)
        point = QPoint(3, 3)
        self.assertEqual(point * matrix, qpointTimesQMatrix(point, matrix))

    def testMatrixWithWrongType(self):
        matrix = QMatrix(11, 12, 21, 22, 100, 200)
        point = QPoint(3, 3)
        self.assertRaises(TypeError, matrix.__mul__, point)

    def testMatrix2x2(self):
        matrix = QMatrix2x2([1.0, 2.0, 3.0, 4.0])

        expectedTransposed = QMatrix2x2([1.0, 3.0, 2.0, 4.0])
        self.assertEqual(matrix.transposed(), expectedTransposed)

        expectedMultiplied = QMatrix2x2([2.0, 4.0, 6.0, 8.0])
        matrix *= 2.0
        self.assertEqual(matrix, expectedMultiplied)

        matrix.setToIdentity()
        self.assertTrue(matrix.isIdentity())

    def testMatrix4x4(self):
        self.assertRaises(TypeError, QMatrix4x4, [0.0, 1.0, 2.0, 3.0])
        self.assertRaises(TypeError, QMatrix4x4, [0.0, 1.0, 2.0, 'I',
                                                  4.0, 5.0, 6.0, 7.0,
                                                  8.0, 9.0, 'N', 11.0,
                                                  12.0, 'd', 14.0, 'T'])

        my_data = [0.0, 1.0, 2.0, 3.0,
                   4.0, 5.0, 6.0, 7.0,
                   8.0, 9.0, 10.0, 11.0,
                   12.0, 13.0, 14.0, 15.0]
        my_datac = [0.0, 4.0, 8.0, 12.0,
                    1.0, 5.0, 9.0, 13.0,
                    2.0, 6.0, 10.0, 14.0,
                    3.0, 7.0, 11.0, 15.0]

        m = QMatrix4x4(my_data)
        d = m.data()
        self.assertTrue(my_datac, d)

        d = m.copyDataTo()
        self.assertTrue(my_data == list(d))

    def testMatrixMapping(self):
        m = QMatrix(1.0, 2.0, 1.0, 3.0, 100.0, 200.0)
        res = m.map(5, 5)
        self.assertAlmostEqual(res[0], 5 * 1.0 + 5 * 1.0 + 100.0)
        self.assertAlmostEqual(res[1], 5 * 2.0 + 5 * 3.0 + 200.0)
        res = m.map(5.0, 5.0)
        self.assertAlmostEqual(res[0], 5.0 * 1.0 + 5.0 * 1.0 + 100.0)
        self.assertAlmostEqual(res[1], 5.0 * 2.0 + 5.0 * 3.0 + 200.0)

if __name__ == '__main__':
    unittest.main()

