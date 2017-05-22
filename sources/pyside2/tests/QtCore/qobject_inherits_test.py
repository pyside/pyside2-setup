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

'''Test cases for QObject methods'''

import unittest

from PySide2.QtCore import QObject

class InheritsCase(unittest.TestCase):
    '''Test case for QObject.inherits'''

    def testCppInheritance(self):
        #QObject.inherits() for c++ classes
        #An class inherits itself
        self.assertTrue(QObject().inherits('QObject'))

    def testPythonInheritance(self):
        #QObject.inherits() for python classes

        class Dummy(QObject):
            #Dummy class
            pass

        self.assertTrue(Dummy().inherits('QObject'))
        self.assertTrue(Dummy().inherits('Dummy'))
        self.assertTrue(not Dummy().inherits('FooBar'))

    def testPythonMultiInheritance(self):
        #QObject.inherits() for multiple inheritance
        # QObject.inherits(classname) should fail if classname isn't a
        # QObject subclass

        class Parent(object):
            #Dummy parent
            pass
        class Dummy(QObject, Parent):
            #Dummy class
            pass

        self.assertTrue(Dummy().inherits('QObject'))
        self.assertTrue(not Dummy().inherits('Parent'))

    def testSetAttributeBeforeCallingInitOnQObjectDerived(self):
        '''Test for bug #428.'''
        class DerivedObject(QObject):
            def __init__(self):
                self.member = 'member'
                QObject.__init__(self)
        obj0 = DerivedObject()
        # The second instantiation of DerivedObject will generate an exception
        # that will not come to surface immediately.
        obj1 = DerivedObject()
        # The mere calling of the object method causes
        # the exception to "reach the surface".
        obj1.objectName()

    def testMultipleInheritance(self):
        def declareClass():
            class Foo(object, QObject):
                pass

        self.assertRaises(TypeError, declareClass)

if __name__ == '__main__':
    unittest.main()
