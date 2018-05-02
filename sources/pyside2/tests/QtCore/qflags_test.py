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

'''Test cases for QFlags'''

import unittest
from PySide2.QtCore import Qt, QTemporaryFile, QFile, QIODevice, QObject

class QFlagTest(unittest.TestCase):
    '''Test case for usage of flags'''

    def testCallFunction(self):
        f = QTemporaryFile()
        self.assertTrue(f.open())
        fileName = f.fileName()
        f.close()

        f = QFile(fileName)
        self.assertEqual(f.open(QIODevice.Truncate | QIODevice.Text | QIODevice.ReadWrite), True)
        om = f.openMode()
        self.assertEqual(om & QIODevice.Truncate, QIODevice.Truncate)
        self.assertEqual(om & QIODevice.Text, QIODevice.Text)
        self.assertEqual(om & QIODevice.ReadWrite, QIODevice.ReadWrite)
        self.assertTrue(om == QIODevice.Truncate | QIODevice.Text | QIODevice.ReadWrite)
        f.close()


class QFlagOperatorTest(unittest.TestCase):
    '''Test case for operators in QFlags'''

    def testInvert(self):
        '''QFlags ~ (invert) operator'''
        self.assertEqual(type(~QIODevice.ReadOnly), QIODevice.OpenMode)

    def testOr(self):
        '''QFlags | (or) operator'''
        self.assertEqual(type(QIODevice.ReadOnly | QIODevice.WriteOnly), QIODevice.OpenMode)

    def testAnd(self):
        '''QFlags & (and) operator'''
        self.assertEqual(type(QIODevice.ReadOnly & QIODevice.WriteOnly), QIODevice.OpenMode)

    def testIOr(self):
        '''QFlags |= (ior) operator'''
        flag = Qt.WindowFlags()
        self.assertTrue(Qt.Widget == 0)
        self.assertFalse(flag & Qt.Widget)
        result = flag & Qt.Widget
        self.assertTrue(result == 0)
        flag |= Qt.WindowMinimizeButtonHint
        self.assertTrue(flag & Qt.WindowMinimizeButtonHint)

    def testInvertOr(self):
        '''QFlags ~ (invert) operator over the result of an | (or) operator'''
        self.assertEqual(type(~(Qt.ItemIsSelectable | Qt.ItemIsEditable)), Qt.ItemFlags)

    def testEqual(self):
        '''QFlags == operator'''
        flags = Qt.Window
        flags |= Qt.WindowMinimizeButtonHint
        flag_type = (flags & Qt.WindowType_Mask)
        self.assertEqual(flag_type, Qt.Window)

        self.assertEqual(Qt.KeyboardModifiers(Qt.ControlModifier), Qt.ControlModifier)

    def testOperatorBetweenFlags(self):
        '''QFlags & QFlags'''
        flags = Qt.NoItemFlags | Qt.ItemIsUserCheckable
        newflags = Qt.NoItemFlags | Qt.ItemIsUserCheckable
        self.assertTrue(flags & newflags)

    def testOperatorDifferentOrder(self):
        '''Different ordering of arguments'''
        flags = Qt.NoItemFlags | Qt.ItemIsUserCheckable
        self.assertEqual(flags | Qt.ItemIsEnabled, Qt.ItemIsEnabled | flags)

class QFlagsOnQVariant(unittest.TestCase):
    def testQFlagsOnQVariant(self):
        o = QObject()
        o.setProperty("foo", QIODevice.ReadOnly | QIODevice.WriteOnly)
        self.assertEqual(type(o.property("foo")), QIODevice.OpenMode)

class QFlagsWrongType(unittest.TestCase):
    def testWrongType(self):
        '''Wrong type passed to QFlags binary operators'''

        self.assertRaises(TypeError, Qt.NoItemFlags | '43')
        self.assertRaises(TypeError, Qt.NoItemFlags & '43')
        self.assertRaises(TypeError, 'jabba' & Qt.NoItemFlags)
        self.assertRaises(TypeError, 'hut' & Qt.NoItemFlags)
        self.assertRaises(TypeError, Qt.NoItemFlags & QObject())

if __name__ == '__main__':
    unittest.main()
