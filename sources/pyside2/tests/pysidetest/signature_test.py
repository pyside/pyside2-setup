#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of PySide2.
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

from __future__ import print_function

import sys
import os
import unittest
from collections import OrderedDict
from pprint import pprint
import PySide2

"""
This test shows that we have over 14500 signatures,
and that they all can be created.
"""

all_modules = list("PySide2." + x for x in PySide2.__all__)

from PySide2.support.signature import parser, inspect

_do_print = True if os.isatty(sys.stdout.fileno()) else False

def dprint(*args, **kw):
    if _do_print:
        print(*args, **kw)

def enum_module(mod_name):
    __import__(mod_name)
    count = 0
    module = sys.modules[mod_name]
    dprint()
    dprint("Module", mod_name)
    members = inspect.getmembers(module, inspect.isclass)
    for class_name, klass in members:
        signature = getattr(klass, '__signature__', None)
        dprint()
        # class_members = inspect.getmembers(klass)
        # gives us also the inherited things.
        dprint("  class {}:".format(class_name))
        if signature is None:
            pass # initialization not called?
        elif isinstance(signature, list):
            dprint("    with overloading():")
            for overload in signature:
                dprint("      def __init__" + str(overload))
        else:
            dprint("    def __init__" + str(signature))
        count += 1
        class_members = list(klass.__dict__.items())
        for func_name, func in class_members:
            signature = getattr(func, '__signature__', None)
            if signature is not None:
                if isinstance(signature, list):
                    dprint("    with overloading():")
                    for overload in signature:
                        dprint("      def", func_name + str(overload))
                else:
                    dprint("    def", func_name + str(signature))
                count += 1
    return count

def enum_all():
    result = OrderedDict()
    total = 0
    for mod_name in all_modules:
        result[mod_name] = enum_module(mod_name)
        total += result[mod_name]
    pprint(result if sys.version_info >= (3,) else list(result.items()))
    print("Total", total)
    return result


class PySideSignatureTest(unittest.TestCase):
    def testAllSignaturesCanBuild(self):
        # This test touches all attributes
        result = enum_all()
        # We omit the number of functions test. This is too vague.
        for mod_name, count in result.items():
            pass
        # If an attribute could not be computed, then we will have a warning
        # in the warningregistry.
        if hasattr(parser, "__warningregistry__"):
            raise RuntimeError("There are errors, see above.")

    def testSignatureExist(self):
        t1 = type(PySide2.QtCore.QObject.children.__signature__)
        self.assertEqual(t1, inspect.Signature)
        t2 = type(PySide2.QtCore.QObject.__dict__["children"].__signature__)
        self.assertEqual(t2, t1)
        obj = PySide2.QtWidgets.QApplication.palette
        t3 = type(obj.__signature__)
        self.assertEqual(t3, list)
        self.assertEqual(len(obj.__signature__), 3)
        for thing in obj.__signature__:
            self.assertEqual(type(thing), inspect.Signature)
        sm = PySide2.QtWidgets.QApplication.__dict__["palette"]
        self.assertFalse(callable(sm))
        self.assertEqual(sm.__func__, obj)
        self.assertTrue(hasattr(sm, "__signature__") and
                        sm.__signature__ is not None)

    def testSignatureIsCached(self):
        # see if we get the same object
        ob1 = PySide2.QtCore.QObject.children.__signature__
        ob2 = PySide2.QtCore.QObject.children.__signature__
        self.assertTrue(ob1 is ob2)
        # same with multi signature
        ob1 = PySide2.QtWidgets.QApplication.palette.__signature__
        ob2 = PySide2.QtWidgets.QApplication.palette.__signature__
        self.assertTrue(ob1 is ob2)

    def testModuleIsInitialized(self):
        assert PySide2.QtWidgets.QApplication.__signature__ is not None

if __name__ == "__main__":
    unittest.main()
