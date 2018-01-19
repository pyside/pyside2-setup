#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

import os

from PySide2.QtCore import QIODevice, QTemporaryFile

class FileChild1(QTemporaryFile):
    pass

class FileChild2(QTemporaryFile):
    def readData(self, maxlen):
        return super(FileChild2, self).readData(maxlen)
    def readLineData(self, maxlen):
        return super(FileChild2, self).readLineData(maxlen)

class readDataTest(unittest.TestCase):
    '''Test case for readData and readLineData'''

    def setUp(self):
        '''Acquire resources'''
        self.filename1 = FileChild1()
        self.assertTrue(self.filename1.open())
        self.filename1.write('Test text for testing')

        self.filename2 = FileChild2()
        self.assertTrue(self.filename2.open())
        self.filename2.write('Test text for testing')

    def tearDown(self):
        '''release resources'''
        pass

    def testBasic(self):
        '''QTemporaryFile.read'''
        self.filename1.seek(0)
        s1 = self.filename1.read(50)
        self.assertEqual(s1, 'Test text for testing')


    def testBug40(self):
        self.filename2.seek(0)
        s2 = self.filename2.read(50)
        self.assertEqual(s2, 'Test text for testing')

        self.filename2.seek(0)
        s2 = self.filename2.readLine(22)
        self.assertEqual(s2, 'Test text for testing')

        self.filename1.seek(0)
        s1 = self.filename1.read(50)
        self.assertEqual(s1, s2)

if __name__ == '__main__':
    unittest.main()
