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

'''Test cases for StrList class that inherits from std::list<Str>.'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import Str, StrList

class StrListTest(unittest.TestCase):
    '''Test cases for StrList class that inherits from std::list<Str>.'''

    def testStrListCtor_NoParams(self):
        '''StrList constructor receives no parameter.'''
        sl = StrList()
        self.assertEqual(len(sl), 0)
        self.assertEqual(sl.constructorUsed(), StrList.NoParamsCtor)

    def testStrListCtor_Str(self):
        '''StrList constructor receives a Str object.'''
        s = Str('Foo')
        sl = StrList(s)
        self.assertEqual(len(sl), 1)
        self.assertEqual(sl[0], s)
        self.assertEqual(sl.constructorUsed(), StrList.StrCtor)

    def testStrListCtor_PythonString(self):
        '''StrList constructor receives a Python string.'''
        s = 'Foo'
        sl = StrList(s)
        self.assertEqual(len(sl), 1)
        self.assertEqual(sl[0], s)
        self.assertEqual(sl.constructorUsed(), StrList.StrCtor)

    def testStrListCtor_StrList(self):
        '''StrList constructor receives a StrList object.'''
        sl1 = StrList(Str('Foo'))
        sl2 = StrList(sl1)
        #self.assertEqual(len(sl1), len(sl2))
        #self.assertEqual(sl1, sl2)
        self.assertEqual(sl2.constructorUsed(), StrList.CopyCtor)

    def testStrListCtor_ListOfStrs(self):
        '''StrList constructor receives a Python list of Str objects.'''
        strs = [Str('Foo'), Str('Bar')]
        sl = StrList(strs)
        self.assertEqual(len(sl), len(strs))
        self.assertEqual(sl, strs)
        self.assertEqual(sl.constructorUsed(), StrList.ListOfStrCtor)

    def testStrListCtor_MixedListOfStrsAndPythonStrings(self):
        '''StrList constructor receives a Python list of mixed Str objects and Python strings.'''
        strs = [Str('Foo'), 'Bar']
        sl = StrList(strs)
        self.assertEqual(len(sl), len(strs))
        self.assertEqual(sl, strs)
        self.assertEqual(sl.constructorUsed(), StrList.ListOfStrCtor)

    def testCompareStrListWithTupleOfStrs(self):
        '''Compares StrList with a Python tuple of Str objects.'''
        sl = StrList()
        sl.append(Str('Foo'))
        sl.append(Str('Bar'))
        self.assertEqual(len(sl), 2)
        self.assertEqual(sl, (Str('Foo'), Str('Bar')))

    def testCompareStrListWithTupleOfPythonStrings(self):
        '''Compares StrList with a Python tuple of Python strings.'''
        sl = StrList()
        sl.append(Str('Foo'))
        sl.append(Str('Bar'))
        self.assertEqual(len(sl), 2)
        self.assertEqual(sl, ('Foo', 'Bar'))

    def testCompareStrListWithTupleOfStrAndPythonString(self):
        '''Compares StrList with a Python tuple of mixed Str objects and Python strings.'''
        sl = StrList()
        sl.append(Str('Foo'))
        sl.append(Str('Bar'))
        self.assertEqual(len(sl), 2)
        self.assertEqual(sl, (Str('Foo'), 'Bar'))

if __name__ == '__main__':
    unittest.main()
