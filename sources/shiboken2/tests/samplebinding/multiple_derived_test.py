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

'''Test cases for multiple inheritance'''

import sys
import unittest

from sample import Base1, Base2, Base3, Base4, Base5, Base6
from sample import MDerived1, MDerived2, MDerived3, MDerived4, MDerived5, SonOfMDerived1

class ExtMDerived1(MDerived1):
    def __init__(self):
        MDerived1.__init__(self)
        self.multiplier = 20
        self.base2Method_called = False
    def base2Method(self):
        return Base2.base2Method(self) * self.multiplier

class MultipleDerivedTest(unittest.TestCase):
    '''Test cases for multiple inheritance'''

    def testIsInstance(self):
        '''MDerived1 is instance of its parents Base1 and Base2.'''
        a = MDerived1()
        self.assertTrue(isinstance(a, MDerived1))
        self.assertTrue(isinstance(a, Base1))
        self.assertTrue(isinstance(a, Base2))

    def testIsSubclass(self):
        '''MDerived1 is subclass of its parents Base1 and Base2.'''
        self.assertTrue(issubclass(MDerived1, Base1))
        self.assertTrue(issubclass(MDerived1, Base2))

    def testCallToFunctionWithBase1ArgumentThatCastsBackToMDerived1(self):
        '''MDerived1 is passed as an Base1 argument to a method that returns it casted back to MDerived1.'''
        a = MDerived1()
        b = MDerived1.transformFromBase1(a)
        self.assertEqual(a, b)

    def testCallToFunctionWithBase2ArgumentThatCastsBackToMDerived1(self):
        '''MDerived1 is passed as an Base2 argument to a method that returns it casted back to MDerived1.'''
        a = MDerived1()
        b = MDerived1.transformFromBase2(a)
        self.assertEqual(a, b)

    def testPythonClassIsInstance(self):
        '''Python defined class ExtMDerived1 is instance of its parents MDerived1, Base1 and Base2.'''
        a = ExtMDerived1()
        self.assertTrue(isinstance(a, ExtMDerived1))
        self.assertTrue(isinstance(a, MDerived1))
        self.assertTrue(isinstance(a, Base1))
        self.assertTrue(isinstance(a, Base2))

    def testPythonClassIsSubclass(self):
        '''Python defined class ExtMDerived1 is subclass of its parents MDerived1, Base1 and Base2.'''
        self.assertTrue(issubclass(ExtMDerived1, MDerived1))
        self.assertTrue(issubclass(ExtMDerived1, Base1))
        self.assertTrue(issubclass(ExtMDerived1, Base2))

    def testCastFromMDerived1ToBases(self):
        '''MDerived1 is casted by C++ to its parents and the binding must return the MDerived1 wrapper.'''
        a = MDerived1()
        refcnt = sys.getrefcount(a)
        b1 = a.castToBase1()
        b2 = a.castToBase2()
        self.assertTrue(isinstance(b1, MDerived1))
        self.assertTrue(isinstance(b2, MDerived1))
        self.assertEqual(a, b1)
        self.assertEqual(a, b2)
        self.assertEqual(sys.getrefcount(a), refcnt + 2)

    def testCastFromExtMDerived1ToMDerived1Bases(self):
        '''Python defined class ExtMDerived1 is casted by C++ to MDerived1 parents and the binding must return the correct ExtMDerived1 instance.'''
        a = ExtMDerived1()
        refcnt = sys.getrefcount(a)
        b1 = a.castToBase1()
        self.assertTrue(isinstance(b1, MDerived1))
        self.assertTrue(isinstance(b1, ExtMDerived1))
        b2 = a.castToBase2()
        self.assertTrue(isinstance(b2, MDerived1))
        self.assertTrue(isinstance(b2, ExtMDerived1))
        self.assertEqual(a, b1)
        self.assertEqual(a, b2)
        self.assertEqual(sys.getrefcount(a), refcnt + 2)

    def testCastFromSonOfMDerived1ToBases(self):
        '''SonOfMDerived1 is casted by C++ to its parents and the binding must return the SonOfMDerived1 wrapper.'''
        a = SonOfMDerived1()
        refcnt = sys.getrefcount(a)
        md1 = a.castToMDerived1()
        b1 = a.castToBase1()
        b2 = a.castToBase2()
        self.assertTrue(isinstance(md1, SonOfMDerived1))
        self.assertTrue(isinstance(b2, SonOfMDerived1))
        self.assertTrue(isinstance(b2, SonOfMDerived1))
        self.assertEqual(a, md1)
        self.assertEqual(a, b1)
        self.assertEqual(a, b2)
        self.assertEqual(sys.getrefcount(a), refcnt + 3)

    def testReimplementedBase2VirtualMethodOnClassInheritingFromMDerived1(self):
        a = ExtMDerived1()
        value = a.base2Method()
        self.assertTrue(value, Base2.base2Method(a) * a.multiplier)

    def testCastFromMDerived2ToBases(self):
        '''MDerived2 is casted by C++ to its parents and the binding must return the MDerived2 wrapper.'''
        a = MDerived2()
        refcnt = sys.getrefcount(a)
        b3 = a.castToBase3()
        b4 = a.castToBase4()
        b5 = a.castToBase5()
        b6 = a.castToBase6()
        self.assertTrue(isinstance(b3, MDerived2))
        self.assertTrue(isinstance(b4, MDerived2))
        self.assertTrue(isinstance(b5, MDerived2))
        self.assertTrue(isinstance(b6, MDerived2))
        self.assertEqual(a, b3)
        self.assertEqual(a, b4)
        self.assertEqual(a, b5)
        self.assertEqual(a, b6)
        self.assertEqual(sys.getrefcount(a), refcnt + 4)

    def testCastFromMDerived3ToBases(self):
        '''MDerived3 is casted by C++ to its parents and the binding must return the MDerived3 wrapper.'''
        a = MDerived3()
        refcnt = sys.getrefcount(a)
        md1 = a.castToMDerived1()
        md2 = a.castToMDerived2()
        b1 = a.castToBase1()
        b2 = a.castToBase2()
        b3 = a.castToBase3()
        b4 = a.castToBase4()
        b5 = a.castToBase5()
        b6 = a.castToBase6()
        self.assertTrue(isinstance(md1, MDerived3))
        self.assertTrue(isinstance(md2, MDerived3))
        self.assertTrue(isinstance(b1, MDerived3))
        self.assertTrue(isinstance(b2, MDerived3))
        self.assertTrue(isinstance(b3, MDerived3))
        self.assertTrue(isinstance(b4, MDerived3))
        self.assertTrue(isinstance(b5, MDerived3))
        self.assertTrue(isinstance(b6, MDerived3))
        self.assertEqual(a, md1)
        self.assertEqual(a, md2)
        self.assertEqual(a, b1)
        self.assertEqual(a, b2)
        self.assertEqual(a, b3)
        self.assertEqual(a, b4)
        self.assertEqual(a, b5)
        self.assertEqual(a, b6)
        self.assertEqual(sys.getrefcount(a), refcnt + 8)

    def testCastFromMDerived4ToBases(self):
        '''MDerived4 is casted by C++ to its parents and the binding must return the MDerived4 wrapper.'''
        a = MDerived4()
        refcnt = sys.getrefcount(a)
        b3 = a.castToBase3()
        b4 = a.castToBase4()
        self.assertTrue(isinstance(b3, MDerived4))
        self.assertTrue(isinstance(b4, MDerived4))
        self.assertEqual(a, b3)
        self.assertEqual(a, b4)
        self.assertEqual(sys.getrefcount(a), refcnt + 2)

    def testCastFromMDerived5ToBases(self):
        '''MDerived5 is casted by C++ to its parents and the binding must return the MDerived5 wrapper.'''
        a = MDerived5()
        refcnt = sys.getrefcount(a)
        b3 = a.castToBase3()
        b4 = a.castToBase4()
        self.assertTrue(isinstance(b3, MDerived5))
        self.assertTrue(isinstance(b4, MDerived5))
        self.assertEqual(a, b3)
        self.assertEqual(a, b4)
        self.assertEqual(sys.getrefcount(a), refcnt + 2)

    def testCastFromMDerived3ToBase3(self):
        '''MDerived3 is casted by C++ to Base3 grandparent using both the inherited and reimplement castToBase3 methods.'''
        a = MDerived3()
        refcnt = sys.getrefcount(a)
        b3_reimplemented = a.castToBase3()
        b3_inherited = MDerived2.castToBase3(a)
        self.assertTrue(isinstance(b3_reimplemented, MDerived3))
        self.assertTrue(isinstance(b3_inherited, MDerived3))
        self.assertEqual(a, b3_reimplemented)
        self.assertEqual(a, b3_inherited)
        self.assertEqual(sys.getrefcount(a), refcnt + 2)

if __name__ == '__main__':
    unittest.main()

