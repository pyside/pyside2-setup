#!/usr/bin/python
# -*- coding: utf-8 -*-

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

'''Tests conversions of QString to and from QKeySequence.'''

import unittest
import py3kcompat as py3k
from helper import UsesQApplication

from PySide2.QtGui import QKeySequence
from PySide2.QtWidgets import QAction

class QStringQKeySequenceTest(UsesQApplication):
    '''Tests conversions of QString to and from QKeySequence.'''

    def testQStringFromQKeySequence(self):
        '''Creates a QString from a QKeySequence.'''
        keyseq = 'Ctrl+A'
        a = QKeySequence(keyseq)
        self.assertEqual(a, keyseq)

    def testPythonStringAsQKeySequence(self):
        '''Passes a Python string to an argument expecting a QKeySequence.'''
        keyseq = py3k.unicode_('Ctrl+A')
        action = QAction(None)
        action.setShortcut(keyseq)
        shortcut = action.shortcut()
        self.assertTrue(isinstance(shortcut, QKeySequence))
        self.assertEqual(shortcut.toString(), keyseq)

if __name__ == '__main__':
    unittest.main()

