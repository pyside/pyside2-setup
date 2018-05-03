#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

''' PYSIDE-315: https://bugreports.qt.io/browse/PYSIDE-315
    Test that creating a signal in the wrong order triggers a warning. '''

from __future__ import print_function

import unittest
import PySide2.QtCore as QtCore
import sys
import warnings


class Whatever(QtCore.QObject):
    echoSignal = QtCore.Signal(str)

    def __init__(self):
        super(Whatever, self).__init__()
        self.echoSignal.connect(self.mySlot)

    def mySlot(self, v):
        pass

class WarningTest(unittest.TestCase):
    def testSignalSlotWarning(self):
        # we create an object. This gives no warning.
        obj = Whatever()
        # then we insert a signal after slots have been created.
        setattr(Whatever, "foo", QtCore.Signal())
        with warnings.catch_warnings(record=True) as w:
            # Cause all warnings to always be triggered.
            warnings.simplefilter("always")
            # Trigger a warning.
            obj.foo.connect(obj.mySlot)
            # Verify some things
            assert issubclass(w[-1].category, RuntimeWarning)
            assert "*** Sort Warning ***" in str(w[-1].message)
            # note that this warning cannot be turned into an error (too hard)


if __name__ == "__main__":
    unittest.main()
