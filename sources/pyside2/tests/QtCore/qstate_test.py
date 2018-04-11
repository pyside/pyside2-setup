#!/usr/bin/python

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
from PySide2.QtCore import *


class QStateTest(unittest.TestCase):
    def testBasic(self):
        app = QCoreApplication([])

        o = QObject()
        o.setProperty("text", "INdT")

        machine = QStateMachine()
        s1 = QState()
        s1.assignProperty(o, "text", "Rocks");

        s2 = QFinalState()
        t = s1.addTransition(o, SIGNAL("change()"), s2);
        self.assertTrue(isinstance(t, QSignalTransition))

        machine.addState(s1)
        machine.addState(s2)
        machine.setInitialState(s1)
        machine.start()

        o.emit(SIGNAL("change()"))

        QTimer.singleShot(100, app.quit)
        app.exec_()

        txt = o.property("text")
        self.assertTrue(txt, "Rocks")

if __name__ == '__main__':
    unittest.main()
