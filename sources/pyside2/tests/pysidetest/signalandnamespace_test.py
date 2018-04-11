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
from testbinding import PySideCPP, TestObjectWithoutNamespace

class ModelViewTest(unittest.TestCase):

    def callback(self, o):
        self._called = o

    def testWithoutNamespace(self):
        self._called = None
        o = PySideCPP.TestObjectWithNamespace(None)
        o.emitSignal.connect(self.callback)
        o.emitSignal.emit(o)
        self.assertTrue(o == self._called)

        self._called = None
        o = PySideCPP.TestObjectWithNamespace(None)
        o.emitSignal.connect(self.callback)
        o.callSignal(o)
        self.assertTrue(o == self._called)

    def testWithNamespace(self):
        self._called = None
        o = PySideCPP.TestObjectWithNamespace(None)
        o.emitSignalWithNamespace.connect(self.callback)
        o.emitSignalWithNamespace.emit(o)
        self.assertTrue(o == self._called)

        self._called = None
        o = PySideCPP.TestObjectWithNamespace(None)
        o.emitSignalWithNamespace.connect(self.callback)
        o.callSignalWithNamespace(o)
        self.assertTrue(o == self._called)


    def testWithoutNamespace1(self):
        self._called = None
        o = TestObjectWithoutNamespace(None)
        o.emitSignal.connect(self.callback)
        o.emitSignal.emit(o)
        self.assertTrue(o == self._called)

        self._called = None
        o = TestObjectWithoutNamespace(None)
        o.emitSignal.connect(self.callback)
        o.callSignal(o)
        self.assertTrue(o == self._called)

    def testWithNamespace1(self):
        self._called = None
        o = TestObjectWithoutNamespace(None)
        o.emitSignalWithNamespace.connect(self.callback)
        o.emitSignalWithNamespace.emit(o)
        self.assertTrue(o == self._called)

        self._called = None
        o = TestObjectWithoutNamespace(None)
        o.emitSignalWithNamespace.connect(self.callback)
        o.callSignalWithNamespace(o)
        self.assertTrue(o == self._called)

    def testTypedfWithouNamespace(self):
        self._called = None
        o = PySideCPP.TestObjectWithNamespace(None)
        o.emitSignalWithTypedef.connect(self.callback)
        o.emitSignalWithTypedef.emit(10)
        self.assertEqual(10, self._called)

        self._called = None
        o = PySideCPP.TestObjectWithNamespace(None)
        o.emitSignalWithTypedef.connect(self.callback)
        o.callSignalWithTypedef(10)
        self.assertEqual(10, self._called)

    def testTypedefWithNamespace(self):
        self._called = None
        o = TestObjectWithoutNamespace(None)
        o.emitSignalWithTypedef.connect(self.callback)
        o.emitSignalWithTypedef.emit(10)
        self.assertEqual(10, self._called)

        self._called = None
        o = TestObjectWithoutNamespace(None)
        o.emitSignalWithTypedef.connect(self.callback)
        o.callSignalWithTypedef(10)
        self.assertEqual(10, self._called)

if __name__ == '__main__':
    unittest.main()

