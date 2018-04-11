#!/usr/bin/python
# -*- coding: utf-8 -*-

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

'''Tests for static methos conflicts with class methods'''

import unittest

from PySide2.QtCore import *

class Foo(QFile):
  pass

class DynObject(QObject):
    def slot(self):
        pass

class qmetaobject_test(unittest.TestCase):
    """
    def test_QMetaObject(self):
        qobj = QObject()
        qobj_metaobj = qobj.metaObject()
        self.assertEqual(qobj_metaobj.className(), "QObject")

        obj = QFile()
        m = obj.metaObject()
        self.assertEqual(m.className(), "QFile")
        self.assertNotEqual(m.methodCount(), qobj_metaobj.methodCount())

        obj = Foo()
        m = obj.metaObject()
        self.assertEqual(m.className(), "Foo")
        f = QFile()
        fm = f.metaObject()
        self.assertEqual(m.methodCount(), fm.methodCount())
    """        

    def test_DynamicSlotSignal(self):
        o = DynObject()
        o2 = QObject()

        o.connect(o2, SIGNAL("bars()"), o.slot)
        self.assertTrue(o2.metaObject().indexOfMethod("bars()") > -1)
        #self.assertTrue(o.metaObject().indexOfMethod("bar()") == -1)
        #self.assertTrue(o.metaObject().indexOfMethod("slot()") > -1)

        #slot_index = o.metaObject().indexOfMethod("slot()")

        #o.connect(o, SIGNAL("foo()"), o2, SIGNAL("bar()"))
        #signal_index = o.metaObject().indexOfMethod("foo()");

        #self.assertTrue(slot_index != signal_index)


if __name__ == '__main__':
    unittest.main()

