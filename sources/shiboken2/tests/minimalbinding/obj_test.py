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

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()
from minimal import Obj

class ExtObj(Obj):
    def __init__(self, objId):
        Obj.__init__(self, objId)
        self.virtual_method_called = False

    def virtualMethod(self, val):
        self.virtual_method_called = True
        return not Obj.virtualMethod(self, val)

    def passObjectType(self, obj):
        obj.setObjId(obj.objId() + 1)
        return obj

    def passObjectTypeReference(self, obj):
        obj.setObjId(obj.objId() + 1)
        return obj


class ObjTest(unittest.TestCase):

    def testNormalMethod(self):
        objId = 123
        obj = Obj(objId)
        self.assertEqual(obj.objId(), objId)

    def testNormalMethodFromExtendedClass(self):
        objId = 123
        obj = ExtObj(objId)
        self.assertEqual(obj.objId(), objId)

    def testVirtualMethod(self):
        obj = Obj(0)
        even_number = 8
        self.assertEqual(obj.virtualMethod(even_number), obj.callVirtualMethod(even_number))

    def testVirtualMethodFromExtendedClass(self):
        obj = ExtObj(0)
        even_number = 8
        self.assertEqual(obj.virtualMethod(even_number), obj.callVirtualMethod(even_number))
        self.assertTrue(obj.virtual_method_called)

    def testPassObjectType(self):
        obj = Obj(0)
        self.assertEqual(obj, obj.passObjectType(obj))
        self.assertEqual(obj, obj.callPassObjectType(obj))

    def testPassObjectTypeNone(self):
        obj = Obj(0)
        self.assertEqual(None, obj.passObjectType(None))
        self.assertEqual(None, obj.callPassObjectType(None))

    def testPassObjectTypeReference(self):
        obj = Obj(0)
        self.assertEqual(obj, obj.passObjectTypeReference(obj))
        self.assertEqual(obj, obj.callPassObjectTypeReference(obj))

    def testPassObjectTypeFromExtendedClass(self):
        obj = ExtObj(0)
        self.assertEqual(obj.objId(), 0)
        sameObj = obj.passObjectType(obj)
        self.assertEqual(obj, sameObj)
        self.assertEqual(sameObj.objId(), 1)
        sameObj = obj.callPassObjectType(obj)
        self.assertEqual(obj, sameObj)
        self.assertEqual(sameObj.objId(), 2)

    def testPassObjectTypeReferenceFromExtendedClass(self):
        obj = ExtObj(0)
        self.assertEqual(obj.objId(), 0)
        sameObj = obj.passObjectTypeReference(obj)
        self.assertEqual(obj, sameObj)
        self.assertEqual(sameObj.objId(), 1)
        sameObj = obj.callPassObjectTypeReference(obj)
        self.assertEqual(obj, sameObj)
        self.assertEqual(sameObj.objId(), 2)


if __name__ == '__main__':
    unittest.main()

