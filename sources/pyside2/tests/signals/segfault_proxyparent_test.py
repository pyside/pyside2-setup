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

from PySide2.QtCore import QObject, SIGNAL

# Description of the problem
# After creating an PyObject that inherits from QObject, connecting it,
# deleting it and later creating another Python QObject-based object, this
# new object will point to the same memory position as the first one.

# Somehow the underlying QObject also points to the same position.

# In PyQt4, the connection works fine with the same memory behavior,
# so it looks like specific to SIP.

class Dummy(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)

class Joe(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)

class SegfaultCase(unittest.TestCase):
    """Test case for the segfault happening when parent() is called inside
    ProxyObject"""

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

    def testSegfault(self):
        """Regression: Segfault for qobjects in the same memory position."""
        obj = Dummy()
        QObject.connect(obj, SIGNAL('bar(int)'), self.callback)
        self.args = (33,)
        obj.emit(SIGNAL('bar(int)'), self.args[0])
        self.assertTrue(self.called)
        del obj

        obj = Joe()
        QObject.connect(obj, SIGNAL('bar(int)'), self.callback)
        self.args = (33,)
        obj.emit(SIGNAL('bar(int)'), self.args[0])
        self.assertTrue(self.called)


if __name__ == '__main__':
    unittest.main()

