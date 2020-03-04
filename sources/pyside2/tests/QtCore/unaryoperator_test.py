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

'''Tests the presence of unary operator __neg__ on the QPoint class'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import QPoint

class NegUnaryOperatorTest(unittest.TestCase):
    '''Tests the presence of unary operator __neg__ on the QPoint class'''

    def setUp(self):
        #Acquire resources
        self.x, self.y = 10, 20
        self.neg_x, self.neg_y = -self.x, -self.y
        self.qpoint = QPoint(self.x, self.y)

    def tearDown(self):
        #Release resources
        del self.qpoint
        del self.x
        del self.y
        del self.neg_x
        del self.neg_y

    def testNegUnaryOperator(self):
        #Test __neg__ unary operator on QPoint class
        __neg__method_exists = True
        try:
            neg_qpoint = -self.qpoint
        except:
            __neg__method_exists = False

        self.assertTrue(__neg__method_exists)
        self.assertEqual(self.qpoint, -neg_qpoint)

if __name__ == '__main__':
    unittest.main()

