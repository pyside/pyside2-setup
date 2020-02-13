#!/usr/bin/python

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

'''Test case for duck punching new implementations of C++ virtual methods into object instances.'''

import os
import sys
import types
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtCore import QObject
from helper import UsesQCoreApplication

def MethodType(func, instance, instanceType):
    if sys.version_info[0] == 3:
        return types.MethodType(func, instance)
    else:
        return types.MethodType(func, instance, instanceType)

class Duck(QObject):
    def __init__(self):
        QObject.__init__(self)
    def childEvent(self, event):
        QObject.childEvent(self, event)

class TestDuckPunchingOnQObjectInstance(UsesQCoreApplication):
    '''Test case for duck punching new implementations of C++ virtual methods into object instances.'''

    def setUp(self):
        #Acquire resources
        self.duck_childEvent_called = False
        UsesQCoreApplication.setUp(self)

    def tearDown(self):
        #Release resources
        del self.duck_childEvent_called
        UsesQCoreApplication.tearDown(self)


    def testChildEventMonkeyPatch(self):
        #Test if the new childEvent injected on QObject instance is called from C++
        parent = QObject()
        def childEvent(obj, event):
            self.duck_childEvent_called = True
        parent.childEvent = MethodType(childEvent, parent, QObject)
        child = QObject()
        child.setParent(parent)
        self.assertTrue(self.duck_childEvent_called)
        # This is done to decrease the refcount of the vm object
        # allowing the object wrapper to be deleted before the
        # BindingManager. This is useful when compiling Shiboken
        # for debug, since the BindingManager destructor has an
        # assert that checks if the wrapper mapper is empty.
        parent.childEvent = None

    def testChildEventMonkeyPatchWithInheritance(self):
        #Test if the new childEvent injected on a QObject's extension class instance is called from C++
        parent = Duck()
        def childEvent(obj, event):
            QObject.childEvent(obj, event)
            self.duck_childEvent_called = True
        child = QObject()
        child.setParent(parent)
        parent.childEvent = MethodType(childEvent, parent, QObject)
        child = QObject()
        child.setParent(parent)
        self.assertTrue(self.duck_childEvent_called)
        # This is done to decrease the refcount of the vm object
        # allowing the object wrapper to be deleted before the
        # BindingManager. This is useful when compiling Shiboken
        # for debug, since the BindingManager destructor has an
        # assert that checks if the wrapper mapper is empty.
        parent.childEvent = None

if __name__ == '__main__':
    unittest.main()

