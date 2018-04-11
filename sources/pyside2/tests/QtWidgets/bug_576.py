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

""" Unittest for bug #576 """
""" http://bugs.openbossa.org/show_bug.cgi?id=576 """

import sys
import unittest

from PySide2 import QtCore, QtWidgets

class Bug576(unittest.TestCase):
    def onButtonDestroyed(self, button):
        self._destroyed = True
        self.assertTrue(isinstance(button, QtWidgets.QPushButton))

    def testWidgetParent(self):
        self._destroyed = False
        app = QtWidgets.QApplication(sys.argv)
        w = QtWidgets.QWidget()

        b = QtWidgets.QPushButton("test")
        b.destroyed[QtCore.QObject].connect(self.onButtonDestroyed)
        self.assertEqual(sys.getrefcount(b), 2)
        b.setParent(w)
        self.assertEqual(sys.getrefcount(b), 3)
        b.parent()
        self.assertEqual(sys.getrefcount(b), 3)
        b.setParent(None)
        self.assertEqual(sys.getrefcount(b), 2)
        del b
        self.assertTrue(self._destroyed)


if __name__ == '__main__':
    unittest.main()

