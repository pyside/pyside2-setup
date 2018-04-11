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

'''Helper classes and functions'''

import os
import unittest

from random import randint

from PySide2.QtCore import QCoreApplication, QTimer

try:
    from PySide2.QtGui import QGuiApplication
except ImportError:
    has_gui = False
else:
    has_gui = True

try:
    from PySide2.QtWidgets import QApplication
except ImportError:
    has_widgets = False
else:
    has_widgets = True

def adjust_filename(filename, orig_mod_filename):
    dirpath = os.path.dirname(os.path.abspath(orig_mod_filename))
    return os.path.join(dirpath, filename)

class NoQtGuiError(Exception):
    def __init__(self):
        Exception.__init__(self, 'No QtGui found')

class BasicPySlotCase(object):
    '''Base class that tests python slots and signal emissions.

    Python slots are defined as any callable passed to QObject.connect().
    '''
    def setUp(self):
        self.called = False

    def tearDown(self):
        try:
            del self.args
        except:
            pass

    def cb(self, *args):
        '''Simple callback with arbitrary arguments.

        The test function must setup the 'args' attribute with a sequence
        containing the arguments expected to be received by this slot.
        Currently only a single connection is supported.
        '''
        if tuple(self.args) == args:
            self.called = True
        else:
            raise ValueError('Invalid arguments for callback')

if has_gui:
    class UsesQGuiApplication(unittest.TestCase):
        '''Helper class to provide QGuiApplication instances'''

        def setUp(self):
            '''Creates the QGuiApplication instance'''

            # Simple way of making instance a singleton
            super(UsesQGuiApplication, self).setUp()
            self.app = QGuiApplication.instance() or QGuiApplication([])

        def tearDown(self):
            '''Deletes the reference owned by self'''
            del self.app
            super(UsesQGuiApplication, self).tearDown()

if has_widgets:
    class UsesQApplication(unittest.TestCase):
        '''Helper class to provide QApplication instances'''

        qapplication = True

        def setUp(self):
            '''Creates the QApplication instance'''

            # Simple way of making instance a singleton
            super(UsesQApplication, self).setUp()
            self.app = QApplication.instance() or QApplication([])

        def tearDown(self):
            '''Deletes the reference owned by self'''
            del self.app
            super(UsesQApplication, self).tearDown()


    class TimedQApplication(unittest.TestCase):
        '''Helper class with timed QApplication exec loop'''

        def setUp(self, timeout=100):
            '''Setups this Application.

            timeout - timeout in milisseconds'''
            self.app = QApplication.instance() or QApplication([])
            QTimer.singleShot(timeout, self.app.quit)

        def tearDown(self):
            '''Delete resources'''
            del self.app
else:
    class UsesQApplication(unittest.TestCase):
        def setUp(self):
            raise NoQtGuiError()
    class TimedQapplication(unittest.TestCase):
        def setUp(self):
            raise NoQtGuiError()


_core_instance = None

class UsesQCoreApplication(unittest.TestCase):
    '''Helper class for test cases that require an QCoreApplication
    Just connect or call self.exit_app_cb. When called, will ask
    self.app to exit.
    '''

    def setUp(self):
        '''Set up resources'''

        global _core_instance
        if _core_instance is None:
            _core_instance = QCoreApplication([])

        self.app = _core_instance

    def tearDown(self):
        '''Release resources'''
        del self.app

    def exit_app_cb(self):
        '''Quits the application'''
        self.app.exit(0)


def random_string(size=5):
    '''Generate random string with the given size'''
    return ''.join(map(chr, [randint(33, 126) for x in range(size)]))
