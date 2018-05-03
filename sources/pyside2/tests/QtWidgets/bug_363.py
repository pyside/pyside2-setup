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

''' Test bug 363: http://bugs.openbossa.org/show_bug.cgi?id=363'''

import sys
import unittest

from helper import UsesQApplication
from PySide2 import QtCore, QtWidgets

# Check for desktop object lifetime
class BugTest(UsesQApplication):
    def mySlot(self):
        pass

    # test if it is possible to connect with a desktop object after storing that on an auxiliar variable
    def testCase1(self):
        desktop = QtWidgets.QApplication.desktop()
        desktop.resized[int].connect(self.mySlot)
        self.assertTrue(True)

    # test if it is possible to connect with a desktop object without storing that on an auxiliar variable
    def testCase2(self):
        QtWidgets.QApplication.desktop().resized[int].connect(self.mySlot)
        self.assertTrue(True)

if __name__ == '__main__':
    unittest.main()
