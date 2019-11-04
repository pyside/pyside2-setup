# -*- coding: utf-8 -*-

#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Copyright (C) 2019 Andreas Beckermann
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import unittest
from testbinding import PySideCPP, TestObject

class QObjectDerivedReprTest(unittest.TestCase):
    """Test the __repr__ implementation of QObject derived classes"""

    def testReprWithoutNamespace(self):
        """Test that classes outside a namespace that have a operator<<(QDebug,...) defined use that
        for __repr__"""
        t = TestObject(123)

        # We don't define __str__, so str(q) should call __repr__
        self.assertEqual(t.__repr__(), str(t))

        # __repr__ should use the operator<<(QDebug,...) implementation
        self.assertIn('TestObject(id=123)', str(t))

    def testReprWithNamespace(self):
        """Test that classes inside a namespace that have a operator<<(QDebug,...) defined use that
        for __repr__"""
        t = PySideCPP.TestObjectWithNamespace(None)

        # We don't define __str__, so str(q) should call __repr__
        self.assertEqual(t.__repr__(), str(t))

        # __repr__ should use the operator<<(QDebug,...) implementation
        self.assertIn('TestObjectWithNamespace("TestObjectWithNamespace")', str(t))

    def testReprInject(self):
        """Test that injecting __repr__ via typesystem overrides the operator<<(QDebug, ...)"""
        t = PySideCPP.TestObject2WithNamespace(None)

        # We don't define __str__, so str(q) should call __repr__
        self.assertEqual(t.__repr__(), str(t))

        # __repr__ should use the operator<<(QDebug,...) implementation
        self.assertEqual(str(t), "TestObject2WithNamespace(injected_repr)")

if __name__ == '__main__':
    unittest.main()

