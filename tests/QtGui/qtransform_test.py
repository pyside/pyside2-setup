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
from PySide2.QtCore import QPointF
from PySide2.QtGui import QTransform, QPolygonF, QPolygonF

class QTransformTest(unittest.TestCase):

    def testMap(self):
        transform = QTransform()
        values = (10.0, 20.0)
        tx, ty = transform.map(*values)
        self.assertTrue(isinstance(tx, float))
        self.assertTrue(isinstance(ty, float))
        self.assertEqual((tx, ty), values)

    def testquadToQuad(self):
        q1 = QPolygonF()
        q1.append(QPointF(10.0, 10.0))
        q1.append(QPointF(20.0, 10.0))
        q1.append(QPointF(10.0, -10.0))
        q1.append(QPointF(20.0, -10.0))

        q2 = QPolygonF()
        q2.append(QPointF(20.0, 20.0))
        q2.append(QPointF(30.0, 20.0))
        q2.append(QPointF(20.0, -20.0))
        q2.append(QPointF(30.0, -20.0))

        t1 = QTransform()
        r1 = QTransform.quadToQuad(q1, q2, t1)
        r2 = QTransform.quadToQuad(q1, q2)

        self.assertTrue(r1)
        self.assertTrue(r2)

        self.assertEqual(t1, r2)

    def testquadToSquare(self):
        q1 = QPolygonF()
        q1.append(QPointF(10.0, 10.0))
        q1.append(QPointF(20.0, 10.0))
        q1.append(QPointF(10.0, -10.0))
        q1.append(QPointF(20.0, -10.0))

        t1 = QTransform()
        r1 = QTransform.quadToSquare(q1, t1)
        r2 = QTransform.quadToSquare(q1)

        self.assertTrue(r1)
        self.assertTrue(r2)

        self.assertEqual(t1, r2)


    def testsquareToQuad(self):
        q1 = QPolygonF()
        q1.append(QPointF(10.0, 10.0))
        q1.append(QPointF(20.0, 10.0))
        q1.append(QPointF(10.0, -10.0))
        q1.append(QPointF(20.0, -10.0))

        t1 = QTransform()
        r1 = QTransform.squareToQuad(q1, t1)
        r2 = QTransform.squareToQuad(q1)

        self.assertTrue(r1)
        self.assertTrue(r2)

        self.assertEqual(t1, r2)


if __name__ == "__main__":
   unittest.main()

