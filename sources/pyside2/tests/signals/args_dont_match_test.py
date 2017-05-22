#!/usr/bin/env python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
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
from PySide2.QtCore import *

class ArgsDontMatch(unittest.TestCase):

    def callback(self, arg1):
        self.ok = True

    def testConnectSignalToSlotWithLessArgs(self):
        self.ok = False
        obj1 = QObject()
        QObject.connect(obj1, SIGNAL('the_signal(int, int, int)'), self.callback)
        obj1.emit(SIGNAL('the_signal(int, int, int)'), 1, 2, 3)

        self.assertTrue(self.ok)



if __name__ == '__main__':
    unittest.main()
