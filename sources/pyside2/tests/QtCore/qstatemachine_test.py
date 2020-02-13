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

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QObject, QState, QFinalState, SIGNAL
from PySide2.QtCore import QTimer, QStateMachine
from PySide2.QtCore import QParallelAnimationGroup

from helper import UsesQCoreApplication


class StateMachineTest(unittest.TestCase):
    '''Check presence of State Machine classes'''

    def testBasic(self):
        '''State machine classes'''
        import PySide2.QtCore
        PySide2.QtCore.QSignalTransition
        PySide2.QtCore.QPropertyAnimation



class QStateMachineTest(UsesQCoreApplication):

    def cb(self, *args):
        self.assertEqual(self.machine.defaultAnimations(), [self.anim])

    def testBasic(self):
        self.machine = QStateMachine()
        s1 = QState()
        s2 = QState()
        s3 = QFinalState()

        QObject.connect(self.machine, SIGNAL("started()"), self.cb)

        self.anim = QParallelAnimationGroup()

        self.machine.addState(s1)
        self.machine.addState(s2)
        self.machine.addState(s3)
        self.machine.setInitialState(s1)
        self.machine.addDefaultAnimation(self.anim)
        self.machine.start()

        QTimer.singleShot(100, self.app.quit)
        self.app.exec_()


class QSetConverterTest(UsesQCoreApplication):
    '''Test converter of QSet toPython using QStateAnimation.configuration'''

    def testBasic(self):
        '''QStateMachine.configuration converting QSet to python set'''
        machine = QStateMachine()
        s1 = QState()
        machine.addState(s1)
        machine.setInitialState(s1)
        machine.start()

        QTimer.singleShot(100, self.app.quit)
        self.app.exec_()

        configuration = machine.configuration()

        self.assertTrue(isinstance(configuration, set))
        self.assertTrue(s1 in configuration)


if __name__ == '__main__':
    unittest.main()
