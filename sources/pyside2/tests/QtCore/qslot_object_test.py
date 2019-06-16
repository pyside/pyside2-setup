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

import unittest
from PySide2 import QtCore

global qApp

class objTest(QtCore.QObject):

    def __init__(self, parent=None):
        QtCore.QObject.__init__(self, parent)

        self.ok = False

    def slot(self):
        global qApp

        self.ok = True
        qApp.quit()



class slotTest(unittest.TestCase):
    def quit_app(self):
        global qApp

        qApp.quit()

    def testBasic(self):
        global qApp
        timer = QtCore.QTimer()
        timer.setInterval(100)

        my_obj = objTest()
        my_slot = QtCore.SLOT("slot()")
        QtCore.QObject.connect(timer, QtCore.SIGNAL("timeout()"), my_obj, my_slot)
        timer.start(100)

        QtCore.QTimer.singleShot(1000, self.quit_app)
        qApp.exec_()

        self.assertTrue(my_obj.ok)


if __name__ == '__main__':
    global qApp
    qApp = QtCore.QCoreApplication([])
    unittest.main()
