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

'''Test cases for QImage'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtGui import QImage, qRgb

from helper import UsesQApplication

class SetPixelFloat(UsesQApplication):
    '''Test case for calling setPixel with float as argument'''

    def setUp(self):
        #Acquire resources
        super(SetPixelFloat, self).setUp()
        self.color = qRgb(255, 0, 0)
        self.image = QImage(200, 200, QImage.Format_RGB32)

    def tearDown(self):
        #Release resources
        del self.color
        del self.image
        super(SetPixelFloat, self).tearDown()

    def testFloat(self):
        #QImage.setPixel(float, float, color) - Implicit conversion
        self.image.setPixel(3.14, 4.2, self.color)
        self.assertEqual(self.image.pixel(3.14, 4.2), self.color)


if __name__ == '__main__':
    unittest.main()
