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

''' Test bug 324: http://bugs.openbossa.org/show_bug.cgi?id=324'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import *
from PySide6.QtWidgets import *

class QBug( QObject ):
    def __init__(self, parent = None):
        QObject.__init__(self, parent)

    def check(self):
        self.done.emit("abc")

    done = Signal(str)

class Bug324(unittest.TestCase):

    def on_done(self, val):
        self.value = val

    def testBug(self):
        app = QApplication([])
        bug = QBug()
        self.value = ''
        bug.done.connect(self.on_done)
        bug.check()
        self.assertEqual(self.value, 'abc')

if __name__ == '__main__':
    unittest.main()
