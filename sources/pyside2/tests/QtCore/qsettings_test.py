#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

'''Test cases for QDate'''

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from helper.helper import adjust_filename
import py3kcompat as py3k
from PySide2.QtCore import QSettings

class TestQSettings(unittest.TestCase):
    def testConversions(self):
        file_path = adjust_filename('qsettings_test.ini', __file__)
        settings = QSettings(file_path, QSettings.IniFormat)

        r = settings.value('var1')
        self.assertEqual(type(r), list)

        r = settings.value('var2')
        if py3k.IS_PY3K:
            self.assertEqual(type(r), str)
        else:
            self.assertEqual(type(r), unicode)

        r = settings.value('var2', type=list)
        self.assertEqual(type(r), list)


    def testDefaultValueConversion(self):
        settings = QSettings('foo.ini', QSettings.IniFormat)
        settings.setValue('zero_value', 0)
        settings.setValue('empty_list', [])
        settings.setValue('bool1', False)
        settings.setValue('bool2', True)
        del settings

        # Loading values already set
        settings = QSettings('foo.ini', QSettings.IniFormat)

        # Getting value that doesn't exist
        r = settings.value("variable")
        self.assertEqual(type(r), type(None))

        # Handling zero value
        r = settings.value('zero_value')
        if py3k.IS_PY3K:
            self.assertEqual(type(r), int)
        else:
            self.assertEqual(type(r), long)

        r = settings.value('zero_value', type=int)
        self.assertEqual(type(r), int)

        # Empty list
        r = settings.value('empty_list')
        self.assertTrue(len(r) == 0)
        self.assertEqual(type(r), list)

        r = settings.value('empty_list', type=list)
        self.assertTrue(len(r) == 0)
        self.assertEqual(type(r), list)

        # Booleans
        r = settings.value('bool1')
        self.assertEqual(type(r), bool)

        r = settings.value('bool2')
        self.assertEqual(type(r), bool)

        r = settings.value('bool1', type=bool)
        self.assertEqual(type(r), bool)

        r = settings.value('bool2', type=int)
        self.assertEqual(type(r), int)

        r = settings.value('bool2', type=bool)
        self.assertEqual(type(r), bool)

        # Not set variable, but with default value
        r = settings.value('lala', 22, type=bytes)
        self.assertEqual(type(r), bytes)

        r = settings.value('lala', 22, type=int)
        self.assertEqual(type(r), int)

        r = settings.value('lala', 22, type=float)
        self.assertEqual(type(r), float)

if __name__ == '__main__':
    unittest.main()
