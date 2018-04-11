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

from PySide2.QtCore import *

import unittest

get_counter = 0
set_counter = 0

class Descriptor(object):
    def __get__(self, obj, owner):
        global get_counter

        if not obj:
            return self

        get_counter += 1
        return obj.var

    def __set__(self, obj, value):
        global set_counter

        set_counter += 1
        obj.var = value

class FooBar(QObject):
    test = Descriptor()
    var = 0

class SetAndGetTestCases(unittest.TestCase):
    def setUp(self):
        global get_counter
        global set_counter

        get_counter = 0
        set_counter = 0

    def testSetMethod(self):
        global get_counter
        global set_counter

        f = FooBar()

        f.test = 1
        self.assertEqual(get_counter, 0)
        self.assertEqual(set_counter, 1)

        get_counter = 0
        set_counter = 0

    def testGetMethod(self):
        global get_counter
        global set_counter

        f = FooBar()
        f.test = 1
        set_counter = 0

        ret = f.test
        self.assertEqual(get_counter, 1)
        self.assertEqual(set_counter, 0)

        get_counter = 0
        set_counter = 0

if __name__ == "__main__":
    unittest.main()
