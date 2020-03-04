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

from functools import partial
import os
import random
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide2.QtCore import QObject, SIGNAL

try:
    from PySide2.QtWidgets import QPushButton, QSpinBox
    hasQtGui = True
except ImportError:
    hasQtGui = False

from helper.basicpyslotcase import BasicPySlotCase
from helper.usesqapplication import UsesQApplication

class MultipleSignalConnections(unittest.TestCase):
    '''Base class for multiple signal connection testing'''

    def run_many(self, sender, signal, emitter, receivers, args=None):
        """Utility method to connect a list of receivers to a signal.
        sender - QObject that will emit the signal
        signal - string with the signal signature
        emitter - the callable that will trigger the signal
        receivers - list of BasicPySlotCase instances
        args - tuple with the arguments to be sent.
        """

        if args is None:
            args = tuple()

        for rec in receivers:
            rec.setUp()
            QObject.connect(sender, SIGNAL(signal), rec.cb)
            rec.args = tuple(args)

        emitter(*args)

        for rec in receivers:
            self.assertTrue(rec.called)


if hasQtGui:
    class QtGuiMultipleSlots(UsesQApplication, MultipleSignalConnections):
        '''Multiple connections to QtGui signals'''

        def testButtonClick(self):
            """Multiple connections to QPushButton.clicked()"""
            sender = QPushButton('button')
            receivers = [BasicPySlotCase() for x in range(30)]
            self.run_many(sender, 'clicked()', sender.click, receivers)

        def testSpinBoxValueChanged(self):
            """Multiple connections to QSpinBox.valueChanged(int)"""
            sender = QSpinBox()
            #FIXME if number of receivers if higher than 50, segfaults
            receivers = [BasicPySlotCase() for x in range(10)]
            self.run_many(sender, 'valueChanged(int)', sender.setValue,
                          receivers, (1,))

if __name__ == '__main__':
    unittest.main()
