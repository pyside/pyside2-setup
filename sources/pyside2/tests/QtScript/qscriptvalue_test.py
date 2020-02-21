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

from __future__ import print_function

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

from PySide2.QtScript import *

# Required for eval() to work
import PySide2.QtScript

from helper.usesqapplication import UsesQApplication

class TestQScriptValue (UsesQApplication):

    def testOperator(self):
        engine = QScriptEngine()
        value = engine.evaluate('x = {"a": 1, "b":2}')
        self.assertEqual(value['a'], 1)
        self.assertRaises(KeyError, value.__getitem__, 'c')
        value = engine.evaluate('x = ["x", "y", "z"]')
        self.assertEqual(value[2], 'z')
        self.assertRaises(IndexError, value.__getitem__, 23)

    def testRepr(self):
        value = QScriptValue("somePerson = { firstName: 'John', lastName: 'Doe' }")
        print(repr(value))
        value2 = eval(repr(value))
        self.assertEqual(value.toString(), value2.toString())
        self.assertEqual(value.toVariant(), value2.toVariant())

    def testIteratorProtocol(self):
        engine = QScriptEngine()
        value = engine.evaluate('x = {"a": 1, "b":2}')
        d = {}
        for k, v in QScriptValueIterator(value):
            d[k] = v
        self.assertEqual(d, {'a': 1, 'b': 2})

        d = {}
        for k, v in value:
            d[k] = v
        self.assertEqual(d, {'a': 1, 'b': 2})

if __name__ == '__main__':
    unittest.main()
