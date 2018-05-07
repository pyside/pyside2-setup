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

from PySide2.QtCore import QObject, SIGNAL, Qt

class Dummy(QObject):
    """Dummy class used in this test."""
    def __init__(self, parent=None):
        QObject.__init__(self, parent)

class TestConnectionTypeSupport(unittest.TestCase):
    def callback(self, *args):
        if tuple(self.args) == args:
            self.called = True

    def testNoArgs(self):
        """Connect signal using a Qt.ConnectionType as argument"""
        obj1 = Dummy()

        QObject.connect(obj1, SIGNAL('foo()'), self.callback, Qt.DirectConnection)
        self.args = tuple()
        obj1.emit(SIGNAL('foo()'), *self.args)

        self.assertTrue(self.called)


if __name__ == '__main__':
    unittest.main()
