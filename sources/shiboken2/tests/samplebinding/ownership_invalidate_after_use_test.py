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

'''Ownership tests for cases of invalidation of Python wrapper after use.'''

import sys
import unittest

from sample import ObjectType, ObjectTypeDerived, Event


class ExtObjectType(ObjectType):
    def __init__(self):
        ObjectType.__init__(self)
        self.type_of_last_event = None
        self.last_event = None
    def event(self, event):
        self.last_event = event
        self.type_of_last_event = event.eventType()
        return True

class MyObjectType (ObjectType):
    def __init__(self):
        super(MyObjectType, self).__init__()
        self.fail = False

    def event(self, ev):
        self.callInvalidateEvent(ev)
        try:
            ev.eventType()
        except:
            self.fail = True
            raise
        return True

    def invalidateEvent(self, ev):
        pass

class ExtObjectTypeDerived(ObjectTypeDerived):
    def __init__(self):
        ObjectTypeDerived.__init__(self)
        self.type_of_last_event = None
        self.last_event = None
    def event(self, event):
        self.last_event = event
        self.type_of_last_event = event.eventType()
        return True

class OwnershipInvalidateAfterUseTest(unittest.TestCase):
    '''Ownership tests for cases of invalidation of Python wrapper after use.'''

    def testInvalidateAfterUse(self):
        '''In ObjectType.event(Event*) the wrapper object created for Event must me marked as invalid after the method is called.'''
        eot = ExtObjectType()
        eot.causeEvent(Event.SOME_EVENT)
        self.assertEqual(eot.type_of_last_event, Event.SOME_EVENT)
        self.assertRaises(RuntimeError, eot.last_event.eventType)

    def testObjectInvalidatedAfterUseAsParameter(self):
        '''Tries to use wrapper invalidated after use as a parameter to another method.'''
        eot = ExtObjectType()
        ot = ObjectType()
        eot.causeEvent(Event.ANY_EVENT)
        self.assertEqual(eot.type_of_last_event, Event.ANY_EVENT)
        self.assertRaises(RuntimeError, ot.event, eot.last_event)

    def testit(self):
        obj = MyObjectType()
        obj.causeEvent(Event.BASIC_EVENT)
        self.assertFalse(obj.fail)

    def testInvalidateAfterUseInDerived(self):
        '''Invalidate was failing in a derived C++ class that also inherited
        other base classes'''
        eot = ExtObjectTypeDerived()
        eot.causeEvent(Event.SOME_EVENT)
        self.assertEqual(eot.type_of_last_event, Event.SOME_EVENT)
        self.assertRaises(RuntimeError, eot.last_event.eventType)

if __name__ == '__main__':
    unittest.main()

