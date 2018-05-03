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

import shiboken2 as shiboken
import unittest
from sample import *

class MultipleInherited (ObjectType, Point):
    def __init__(self):
        ObjectType.__init__(self)
        Point.__init__(self)

class TestShiboken(unittest.TestCase):
    def testIsValid(self):
        self.assertTrue(shiboken.isValid(object()))
        self.assertTrue(shiboken.isValid(None))

        bb = BlackBox()
        item = ObjectType()
        ticket = bb.keepObjectType(item)
        bb.disposeObjectType(ticket)
        self.assertFalse(shiboken.isValid(item))

    def testWrapInstance(self):
        addr = ObjectType.createObjectType()
        obj = shiboken.wrapInstance(addr, ObjectType)
        self.assertFalse(shiboken.createdByPython(obj))
        obj.setObjectName("obj")
        self.assertEqual(obj.objectName(), "obj")
        self.assertEqual(addr, obj.identifier())
        self.assertFalse(shiboken.createdByPython(obj))

        # avoid mem leak =]
        bb = BlackBox()
        self.assertTrue(shiboken.createdByPython(bb))
        bb.disposeObjectType(bb.keepObjectType(obj))

    def testIsOwnedByPython(self):
        obj = ObjectType()
        self.assertTrue(shiboken.ownedByPython(obj))
        p = ObjectType()
        obj.setParent(p)
        self.assertFalse(shiboken.ownedByPython(obj))

    def testDump(self):
        """Just check if dump doesn't crash on certain use cases"""
        p = ObjectType()
        obj = ObjectType(p)
        obj2 = ObjectType(obj)
        obj3 = ObjectType(obj)
        self.assertEqual(shiboken.dump(None), "Ordinary Python type.")
        shiboken.dump(obj)

        model = ObjectModel(p)
        v = ObjectView(model, p)
        shiboken.dump(v)

        m = MultipleInherited()
        shiboken.dump(m)
        self.assertEqual(len(shiboken.getCppPointer(m)), 2)

        # Don't crash even after deleting an object
        shiboken.invalidate(obj)
        shiboken.dump(obj)  # deleted
        shiboken.dump(p)    # child deleted
        shiboken.dump(obj2) # parent deleted

    def testDelete(self):
        obj = ObjectType()
        child = ObjectType(obj)
        self.assertTrue(shiboken.isValid(obj))
        self.assertTrue(shiboken.isValid(child))
        # Note: this test doesn't assure that the object dtor was really called
        shiboken.delete(obj)
        self.assertFalse(shiboken.isValid(obj))
        self.assertFalse(shiboken.isValid(child))

    def testVersionAttr(self):
        self.assertEqual(type(shiboken.__version__), str)
        self.assertTrue(len(shiboken.__version__) >= 5)
        self.assertEqual(type(shiboken.__version_info__), tuple)
        self.assertEqual(len(shiboken.__version_info__), 5)

    def testAllWrappers(self):
        obj = ObjectType()
        self.assertTrue(obj in shiboken.getAllValidWrappers())
        shiboken.delete(obj)
        self.assertFalse(obj in shiboken.getAllValidWrappers())

if __name__ == '__main__':
    unittest.main()
