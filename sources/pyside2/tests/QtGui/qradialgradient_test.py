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

from PySide2.QtGui import QRadialGradient
from PySide2.QtCore import QPointF

class QRadialGradientConstructor(unittest.TestCase):
    def _compare(self, qptf, tpl):
        self.assertEqual((qptf.x(), qptf.y()), tpl)

    def _assertValues(self, grad):
        self._compare(grad.center(), (1.0, 2.0))
        self._compare(grad.focalPoint(), (3.0, 4.0))
        self.assertEqual(grad.radius(), 5.0)

    def testAllInt(self):
        grad = QRadialGradient(1, 2, 5, 3, 4)
        self._assertValues(grad)

    def testQPointF(self):
        grad = QRadialGradient(QPointF(1, 2), 5, QPointF(3, 4))
        self._assertValues(grad)

    def testSetQPointF(self):
        grad = QRadialGradient()
        grad.setCenter(QPointF(1, 2))
        self._compare(grad.center(), (1.0, 2.0))

if __name__ == '__main__':
    unittest.main()
