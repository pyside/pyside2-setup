#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
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

'''Test cases for virtual methods in multiple inheritance scenarios'''

import unittest

from sample import VirtualMethods, ObjectType, Event


class ImplementsNone(ObjectType, VirtualMethods):
    '''Implements no virtual methods'''

    def __init__(self):
        ObjectType.__init__(self)
        VirtualMethods.__init__(self)


class ImplementsBoth(ObjectType, VirtualMethods):
    '''Implements ObjectType.event and VirtualMethods.sum1'''

    def __init__(self):
        ObjectType.__init__(self)
        VirtualMethods.__init__(self)
        self.event_processed = False

    def event(self, event):
        self.event_processed = True
        return True

    def sum1(self, arg0, arg1, arg2):
        return (arg0 + arg1 + arg2) * 2


class CppVirtualTest(unittest.TestCase):
    '''Virtual method defined in c++ called from C++'''

    def testCpp(self):
        '''C++ calling C++ virtual method in multiple inheritance scenario'''
        obj = ImplementsNone()
        self.assertTrue(ObjectType.processEvent([obj], Event(Event.BASIC_EVENT)))
        self.assertRaises(AttributeError, getattr, obj, 'event_processed')

        self.assertEqual(obj.callSum0(1, 2, 3), 6)


class PyVirtualTest(unittest.TestCase):
    '''Virtual method reimplemented in python called from C++'''

    def testEvent(self):
        '''C++ calling Python reimplementation of virtual in multiple inheritance'''
        obj = ImplementsBoth()
        self.assertTrue(ObjectType.processEvent([obj], Event(Event.BASIC_EVENT)))
        self.assertTrue(obj.event_processed)

        self.assertEqual(obj.callSum1(1, 2, 3), 12)


if __name__ == '__main__':
    unittest.main()
