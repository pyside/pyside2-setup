#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

from PySide6.QtCore import QObject
from PySide6.QtQml import ListProperty

class InheritsQObject(QObject):
    pass

def dummyFunc():
    pass

class TestListProperty(unittest.TestCase):
    def testIt(self):

        # Verify that type checking works properly
        type_check_error = False

        try:
            ListProperty(QObject)
            ListProperty(InheritsQObject)
        except:
            type_check_error = True

        self.assertFalse(type_check_error)

        try:
            ListProperty(int)
        except TypeError:
            type_check_error = True

        self.assertTrue(type_check_error)

        # Verify that method validation works properly
        method_check_error = False

        try:
            ListProperty(QObject, append=None, at=None, count=None, replace=None, clear=None, removeLast=None)  # Explicitly setting None
            ListProperty(QObject, append=dummyFunc)
            ListProperty(QObject, count=dummyFunc, at=dummyFunc)
        except:
            method_check_error = True

        self.assertFalse(method_check_error)

        try:
            ListPropery(QObject, append=QObject())
        except:
            method_check_error = True

        self.assertTrue(method_check_error)

if __name__ == '__main__':
    unittest.main()
