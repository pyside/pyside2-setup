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

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper import TimedQApplication
from PySide2.support import deprecated
from PySide2.support.signature import importhandler
from PySide2 import QtGui


class TestTimedApp(TimedQApplication):
    '''Simple test case for TimedQApplication'''

    def testFoo(self):
        #Simple test of TimedQApplication
        self.app.exec_()


def fix_for_QtGui(QtGui):
    QtGui.something = 42

class TestPatchingFramework(unittest.TestCase):
    """Simple test that verifies that deprecated.py works"""

    deprecated.fix_for_QtGui = fix_for_QtGui

    def test_patch_works(self):
        something = "something"
        self.assertFalse(hasattr(QtGui, something))
        importhandler.finish_import(QtGui)
        self.assertTrue(hasattr(QtGui, something))

if __name__ == '__main__':
    unittest.main()
