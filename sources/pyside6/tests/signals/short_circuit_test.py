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

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

from PySide6.QtCore import QObject, SIGNAL, SLOT

class Dummy(QObject):
    """Dummy class used in this test."""
    def __init__(self, parent=None):
        QObject.__init__(self, parent)

class ShortCircuitSignals(unittest.TestCase):
    def setUp(self):
        self.called = False

    def tearDown(self):
        try:
            del self.args
        except:
            pass

    def callback(self, *args):
        if tuple(self.args) == args:
            self.called = True

    def testNoArgs(self):
        """Short circuit signal without arguments"""
        obj1 = Dummy()
        QObject.connect(obj1, SIGNAL('foo()'), self.callback)
        self.args = tuple()
        obj1.emit(SIGNAL('foo()'), *self.args)
        self.assertTrue(self.called)

    def testWithArgs(self):
        """Short circuit signal with integer arguments"""
        obj1 = Dummy()

        QObject.connect(obj1, SIGNAL('foo(int)'), self.callback)
        self.args = (42,)
        obj1.emit(SIGNAL('foo(int)'), *self.args)

        self.assertTrue(self.called)

    def testMultipleArgs(self):
        """Short circuit signal with multiple arguments"""
        obj1 = Dummy()

        QObject.connect(obj1, SIGNAL('foo(int,int,QString)'), self.callback)
        self.args = (42,33,'char')
        obj1.emit(SIGNAL('foo(int,int,QString)'), *self.args)

        self.assertTrue(self.called)

    def testComplexArgs(self):
        """Short circuit signal with complex arguments"""
        obj1 = Dummy()

        QObject.connect(obj1, SIGNAL('foo(int,QObject*)'), self.callback)
        self.args = (42, obj1)
        obj1.emit(SIGNAL('foo(int,QObject*)'), *self.args)

        self.assertTrue(self.called)

if __name__ == '__main__':
    unittest.main()
