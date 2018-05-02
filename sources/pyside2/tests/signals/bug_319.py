#!/usr/bin/env python
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

import unittest
from PySide2 import QtCore
from helper import UsesQCoreApplication

class Listener(QtCore.QObject):
    def __init__(self):
        QtCore.QObject.__init__(self, None)
        self._phrase = []

    @QtCore.Slot(tuple)
    def listen(self, words):
        for w in words:
            self._phrase.append(w)

class Communicate(QtCore.QObject):
    # create a new signal on the fly and name it 'speak'
    speak = QtCore.Signal(tuple)

class SignaltoSignalTest(UsesQCoreApplication):
    def testBug(self):
        someone = Communicate()
        someone2 = Listener()
        # connect signal and slot
        someone.speak.connect(someone2.listen)
        # emit 'speak' signal
        talk = ("one","two","three")
        someone.speak.emit(talk)
        self.assertEqual(someone2._phrase, list(talk))

if __name__ == '__main__':
    unittest.main()

