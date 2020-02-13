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

''' Test bug 512: http://bugs.openbossa.org/show_bug.cgi?id=512'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper import UsesQApplication
from PySide2.QtCore import *
from PySide2.QtWidgets import *

class BugTest(UsesQApplication):
    def testCase(self):
        w = QWidget(None)
        lbl = QLabel("Hello", w);
        g = QGridLayout()
        g.addWidget(lbl, 0, 0)
        w.setLayout(g)
        w.show()

        t = g.getItemPosition(0)
        self.assertEqual(type(t), tuple)
        self.assertEqual(t, (0,0,1,1))

if __name__ == '__main__':
    unittest.main()
