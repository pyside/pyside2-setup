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
from PySide2.QtCore import *
from PySide2.QtGui import *

class QPolygonFNotIterableTest(unittest.TestCase):
    """Test if a QPolygonF is iterable"""
    def testIt(self):
        p = QPolygonF(4)
        self.assertEqual(len(p), 4)

        for i in range(0, 4):
            p[i] = QPointF(float(i), float(i))

        i = 0
        for point in p:
            self.assertEqual(int(point.x()), i)
            self.assertEqual(int(point.y()), i)
            i += 1;

    def testPolygonShiftOperators(self):
        p = QPolygon()
        self.assertEqual(len(p), 0)
        p << QPoint(10, 20) << QPoint(20, 30) << [QPoint(20, 30), QPoint(40, 50)]
        self.assertEqual(len(p), 4)

if __name__ == '__main__':
    unittest.main()
