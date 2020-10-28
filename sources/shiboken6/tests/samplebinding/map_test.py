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

'''Test cases for std::map container conversions'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from sample import MapUser

class ExtendedMapUser(MapUser):
    def __init__(self):
        MapUser.__init__(self)
        self.create_map_called = False

    def createMap(self):
        self.create_map_called = True
        return {'two' : (complex(2.2, 2.2), 2),
                'three' : (complex(3.3, 3.3), 3),
                'five' : (complex(5.5, 5.5), 5),
                'seven' : (complex(7.7, 7.7), 7)}

class MapConversionTest(unittest.TestCase):
    '''Test case for std::map container conversions'''

    def testReimplementedVirtualMethodCall(self):
        '''Test if a Python override of a virtual method is correctly called from C++.'''
        mu = ExtendedMapUser()
        map_ = mu.callCreateMap()
        self.assertTrue(mu.create_map_called)
        self.assertEqual(type(map_), dict)
        for key, value in map_.items():
            self.assertEqual(type(key), str)
            self.assertEqual(type(value[0]), complex)
            self.assertEqual(type(value[1]), int)

    def testConversionInBothDirections(self):
        '''Test converting a map from Python to C++ and back again.'''
        mu = MapUser()
        map_ = {'odds' : [2, 4, 6], 'evens' : [3, 5, 7], 'primes' : [3, 4, 6]}
        mu.setMap(map_)
        result = mu.getMap()
        self.assertEqual(result, map_)

    def testConversionMapIntKeyValueTypeValue(self):
        '''C++ signature: MapUser::passMapIntValueType(const std::map<int, const ByteArray>&)'''
        mu = MapUser()
        map_ = {0 : 'string'}
        result = mu.passMapIntValueType(map_)
        self.assertEqual(map_, result)

if __name__ == '__main__':
    unittest.main()
