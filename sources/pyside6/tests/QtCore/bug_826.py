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

from PySide6.QtCore import QEvent, Qt
import PySide6


TEST_EVENT_TYPE = QEvent.Type(QEvent.registerEventType())

class TestEvent(QEvent):
    TestEventType = QEvent.Type(QEvent.registerEventType())

    def __init__(self, rand=0):
        super(TestEvent, self).__init__(TestEvent.TestEventType)
        self._rand = rand

    def getRand(self):
        return self._rand


class TestEnums(unittest.TestCase):
    def testUserTypesValues(self):
        self.assertTrue(QEvent.User <= TestEvent.TestEventType <= QEvent.MaxUser)
        self.assertTrue(QEvent.User <= TEST_EVENT_TYPE <= QEvent.MaxUser)

    def testUserTypesRepr(self):
        self.assertEqual(eval(repr(TestEvent.TestEventType)), TestEvent.TestEventType)
        self.assertEqual(eval(repr(TEST_EVENT_TYPE)), TEST_EVENT_TYPE)

if __name__ == '__main__':
    unittest.main()
