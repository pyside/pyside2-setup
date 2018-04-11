#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

'''Test cases for SimpleFile class'''

import os
import unittest

from sample import SimpleFile

class SimpleFileTest(unittest.TestCase):
    '''Test cases for SimpleFile class.'''

    def setUp(self):
        filename = 'simplefile%d.txt' % os.getpid()
        self.existing_filename = os.path.join(os.path.curdir, filename)
        self.delete_file = False
        if not os.path.exists(self.existing_filename):
            f = open(self.existing_filename, 'w')
            for line in range(10):
                f.write('sbrubbles\n')
            f.close()
            self.delete_file = True

        self.non_existing_filename = os.path.join(os.path.curdir, 'inexistingfile.txt')
        i = 0
        while os.path.exists(self.non_existing_filename):
            i += 1
            filename = 'inexistingfile-%d.txt' % i
            self.non_existing_filename = os.path.join(os.path.curdir, filename)

    def tearDown(self):
        if self.delete_file:
            os.remove(self.existing_filename)

    def testExistingFile(self):
        '''Test SimpleFile class with existing file.'''
        f = SimpleFile(self.existing_filename)
        self.assertEqual(f.filename(), self.existing_filename)
        f.open()
        self.assertNotEqual(f.size(), 0)
        f.close()

    def testNonExistingFile(self):
        '''Test SimpleFile class with non-existing file.'''
        f = SimpleFile(self.non_existing_filename)
        self.assertEqual(f.filename(), self.non_existing_filename)
        self.assertRaises(IOError, f.open)
        self.assertEqual(f.size(), 0)

if __name__ == '__main__':
    unittest.main()

