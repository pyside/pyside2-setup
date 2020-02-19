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

'''Connecting lambda to gui signals'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QObject, SIGNAL

try:
    from PySide2.QtWidgets import QSpinBox, QPushButton
    hasQtGui = True
except ImportError:
    hasQtGui = False

from helper.usesqapplication import UsesQApplication

if hasQtGui:
    class Control:
        def __init__(self):
            self.arg = False

    class QtGuiSigLambda(UsesQApplication):

        def testButton(self):
            #Connecting a lambda to a QPushButton.clicked()
            obj = QPushButton('label')
            ctr = Control()
            func = lambda: setattr(ctr, 'arg', True)
            QObject.connect(obj, SIGNAL('clicked()'), func)
            obj.click()
            self.assertTrue(ctr.arg)
            QObject.disconnect(obj, SIGNAL('clicked()'), func)


        def testSpinButton(self):
            #Connecting a lambda to a QPushButton.clicked()
            obj = QSpinBox()
            ctr = Control()
            arg = 444
            func = lambda x: setattr(ctr, 'arg', 444)
            QObject.connect(obj, SIGNAL('valueChanged(int)'), func)
            obj.setValue(444)
            self.assertEqual(ctr.arg, arg)
            QObject.disconnect(obj, SIGNAL('valueChanged(int)'), func)

if __name__ == '__main__':
    unittest.main()
