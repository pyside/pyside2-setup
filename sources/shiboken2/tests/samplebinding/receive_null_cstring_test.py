#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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

'''Test case for a function that could receive a NULL pointer in a '[const] char*' parameter.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import countCharacters

class ReceiveNullCStringTest(unittest.TestCase):
    '''Test case for a function that could receive a NULL pointer in a '[const] char*' parameter.'''

    def testBasic(self):
        '''The test function should be working for the basic cases.'''
        a = ''
        b = 'abc'
        self.assertEqual(countCharacters(a), len(a))
        self.assertEqual(countCharacters(b), len(b))

    def testReceiveNull(self):
        '''The test function returns '-1' when receives a None value instead of a string.'''
        self.assertEqual(countCharacters(None), -1)

if __name__ == '__main__':
    unittest.main()

