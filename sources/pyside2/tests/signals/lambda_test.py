#!/usr/bin/env python

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

'''Connecting lambda to signals'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QObject, SIGNAL, QProcess

from helper import UsesQCoreApplication


class Dummy(QObject):

    def __init__(self, *args):
        super(Dummy, self).__init__(*args)


class BasicCase(unittest.TestCase):

    def testSimplePythonSignalNoArgs(self):
        #Connecting a lambda to a simple python signal without arguments
        obj = Dummy()
        QObject.connect(obj, SIGNAL('foo()'),
                        lambda: setattr(obj, 'called', True))
        obj.emit(SIGNAL('foo()'))
        self.assertTrue(obj.called)

    def testSimplePythonSignal(self):
        #Connecting a lambda to a simple python signal witharguments
        obj = Dummy()
        arg = 42
        QObject.connect(obj, SIGNAL('foo(int)'),
                        lambda x: setattr(obj, 'arg', 42))
        obj.emit(SIGNAL('foo(int)'), arg)
        self.assertEqual(obj.arg, arg)


class QtSigLambda(UsesQCoreApplication):

    qapplication = True

    def testNoArgs(self):
        '''Connecting a lambda to a signal without arguments'''
        proc = QProcess()
        dummy = Dummy()
        QObject.connect(proc, SIGNAL('started()'),
                        lambda: setattr(dummy, 'called', True))
        proc.start(sys.executable, ['-c', '""'])
        proc.waitForFinished()
        self.assertTrue(dummy.called)

    def testWithArgs(self):
        '''Connecting a lambda to a signal with arguments'''
        proc = QProcess()
        dummy = Dummy()
        QObject.connect(proc, SIGNAL('finished(int)'),
                        lambda x: setattr(dummy, 'called', x))
        proc.start(sys.executable, ['-c', '""'])
        proc.waitForFinished()
        self.assertEqual(dummy.called, proc.exitCode())


if __name__ == '__main__':
    unittest.main()
