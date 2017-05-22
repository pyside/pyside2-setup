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

from helper import UsesQApplication

from PySide2.QtWidgets import QDoubleSpinBox, QGraphicsBlurEffect

class TestSignalConnection(UsesQApplication):
    def testFloatSignal(self):
        foo1 = QDoubleSpinBox()
        foo2 = QDoubleSpinBox()
        foo1.valueChanged[float].connect(foo2.setValue)
        foo2.valueChanged[float].connect(foo1.setValue)
        foo1.setValue(0.42)
        self.assertEqual(foo1.value(), foo2.value())

    def testQRealSignal(self):
        foo1 = QDoubleSpinBox()
        effect = QGraphicsBlurEffect()
        effect.blurRadiusChanged['qreal'].connect(foo1.setValue) # check if qreal is a valid type
        effect.setBlurRadius(0.42)
        self.assertAlmostEqual(foo1.value(), effect.blurRadius())

if __name__ == '__main__':
    unittest.main()
