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
import pickle
from PySide2.QtGui import QColor

class TestQColor (unittest.TestCase):
    def reduceColor(self, c):
        p = pickle.dumps(c)
        c2 = pickle.loads(p)
        self.assertEqual(c.spec(), c2.spec())
        self.assertEqual(c, c2)

    def testReduceEmpty(self):
        self.reduceColor(QColor())

    def testReduceString(self):
        self.reduceColor(QColor('gray'))

    def testReduceRGB(self):
        self.reduceColor(QColor.fromRgbF(0.1, 0.2, 0.3, 0.4))

    def testReduceCMYK(self):
        self.reduceColor(QColor.fromCmykF(0.1, 0.2, 0.3, 0.4, 0.5))

    def testReduceHsl(self):
        self.reduceColor(QColor.fromHslF(0.1, 0.2, 0.3, 0.4))

    def testReduceHsv(self):
        self.reduceColor(QColor.fromHsvF(0.1, 0.2, 0.3, 0.4))

if __name__ == "__main__":
    unittest.main()
