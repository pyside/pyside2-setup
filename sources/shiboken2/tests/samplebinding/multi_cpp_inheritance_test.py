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

'''Test cases for multiple inheritance'''

import sys
import unittest

from sample import *

class SimpleUseCase(ObjectType, Str):
    def __init__(self, name):
        ObjectType.__init__(self)
        Str.__init__(self, name)

class SimpleUseCaseReverse(Str, ObjectType):
    def __init__(self, name):
        ObjectType.__init__(self)
        Str.__init__(self, name)

class SimpleUseCase2(SimpleUseCase):
    def __init__(self, name):
        SimpleUseCase.__init__(self, name)

class ComplexUseCase(SimpleUseCase2, Point):
    def __init__(self, name):
        SimpleUseCase2.__init__(self, name)
        Point.__init__(self)

class ComplexUseCaseReverse(Point, SimpleUseCase2):
    def __init__(self, name):
        SimpleUseCase2.__init__(self, name)
        Point.__init__(self)

class MultipleCppDerivedTest(unittest.TestCase):
    def testInstanciation(self):
        s = SimpleUseCase("Hi")
        self.assertEqual(s, "Hi")
        s.setObjectName(s)
        self.assertEqual(s.objectName(), "Hi")

    def testInstanciation2(self):
        s = SimpleUseCase2("Hi")
        self.assertEqual(s, "Hi")
        s.setObjectName(s)
        self.assertEqual(s.objectName(), "Hi")

    def testComplexInstanciation(self):
        c = ComplexUseCase("Hi")
        self.assertEqual(c, "Hi")
        c.setObjectName(c)
        self.assertEqual(c.objectName(), "Hi")
        c.setX(2);
        self.assertEqual(c.x(), 2)

class MultipleCppDerivedReverseTest(unittest.TestCase):
    def testInstanciation(self):
        s = SimpleUseCaseReverse("Hi")
        self.assertEqual(s, "Hi")
        s.setObjectName(s)
        self.assertEqual(s.objectName(), "Hi")

    def testInstanciation2(self):
        s = SimpleUseCase2("Hi")
        self.assertEqual(s, "Hi")
        s.setObjectName(s)
        self.assertEqual(s.objectName(), "Hi")

    def testComplexInstanciation(self):
        c = ComplexUseCaseReverse("Hi")
        c.setObjectName(c)
        self.assertEqual(c.objectName(), "Hi")
        c.setX(2);
        self.assertEqual(c, Point(2, 0))

if __name__ == '__main__':
    unittest.main()
