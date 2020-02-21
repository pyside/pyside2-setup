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

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2 import QtCore
from helper.usesqcoreapplication import UsesQCoreApplication

class DerivedDate(QtCore.QDate):
    def __init__(self,y,m,d):
        super(DerivedDate,self).__init__(y,m,d)

class Emitter(QtCore.QObject):
    dateSignal1 = QtCore.Signal(QtCore.QDate)
    dateSignal2 = QtCore.Signal(DerivedDate)
    tupleSignal = QtCore.Signal(tuple)

class SignaltoSignalTest(UsesQCoreApplication):
    def myCb(self, dt):
        self._dt = dt

    def testBug(self):
        e = Emitter()
        d = DerivedDate(2010,8,24)
        self._dt = None
        e.dateSignal1.connect(self.myCb)
        e.dateSignal1.emit(d)
        self.assertEqual(self._dt, d)

        self._dt = None
        e.dateSignal2.connect(self.myCb)
        e.dateSignal2.emit(d)
        self.assertEqual(self._dt, d)

        myTuple = (5, 6, 7)
        self._dt = None
        e.tupleSignal.connect(self.myCb)
        e.tupleSignal.emit(myTuple)
        self.assertEqual(myTuple, self._dt)

if __name__ == '__main__':
    unittest.main()

