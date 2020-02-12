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
import tempfile
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from init_paths import init_test_paths
init_test_paths(False)

import py3kcompat as py3k

from PySide2.QtCore import QDir, QFile, QIODevice, QSaveFile, QTemporaryDir

class GetCharTest(unittest.TestCase):
    '''Test case for QIODevice.getChar in QFile'''

    def setUp(self):
        '''Acquire resources'''
        handle, self.filename = tempfile.mkstemp()
        os.write(handle, py3k.b('a'))
        os.close(handle)

    def tearDown(self):
        '''release resources'''
        os.remove(self.filename)

    def testBasic(self):
        '''QFile.getChar'''
        obj = QFile(self.filename)
        obj.open(QIODevice.ReadOnly)
        try:
            self.assertEqual(obj.getChar(), (True, 'a'))
            self.assertFalse(obj.getChar()[0])
        finally:
            obj.close()

    def testBug721(self):
        obj = QFile(self.filename)
        obj.open(QIODevice.ReadOnly)
        try:
            memory = obj.map(0, 1)
            self.assertEqual(len(memory), 1)
            if sys.version_info[0] >= 3:
                self.assertEqual(memory[0], ord('a'))
            else:
                self.assertEqual(memory[0], py3k.b('a'))
            # now memory points to wild bytes... :-)
            # uncommenting this must cause a segfault.
            # self.assertEqual(memory[0], 'a')
        finally:
            obj.unmap(memory)
            obj.close()

    def testQSaveFile(self):
        dir = QTemporaryDir(QDir.tempPath() + "/XXXXXX.dir")
        self.assertTrue(dir.isValid())
        saveFile = QSaveFile(dir.path() + "/test.dat")
        self.assertTrue(saveFile.open(QIODevice.WriteOnly))
        saveFile.write(py3k.b("Test"))
        self.assertTrue(saveFile.commit())
        self.assertTrue(os.path.exists(QDir.toNativeSeparators(saveFile.fileName())))

if __name__ == '__main__':
    unittest.main()
