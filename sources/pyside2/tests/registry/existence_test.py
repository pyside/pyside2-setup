#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

from __future__ import print_function, absolute_import

import os
import sys
import unittest
from textwrap import dedent
from init_platform import (enum_all, generate_all, is_ci,
   getEffectiveRefPath, getRefPath, qtVersion)
from util import isolate_warnings, check_warnings, suppress_warnings, warn
from PySide2 import *

refPath = getRefPath()
effectiveRefPath = getEffectiveRefPath()
effectiveRefPathRoot = os.path.splitext(effectiveRefPath)[0]
pyc = effectiveRefPathRoot + ".pyc"
if os.path.exists(pyc) and not os.path.exists(effectiveRefPath):
    # on Python2 the pyc file would be imported
    os.unlink(pyc)
module = os.path.basename(effectiveRefPathRoot)

if refPath != effectiveRefPath:
    print("*** Falling back to ", effectiveRefPath, " since expected ",
        refPath, " does not exist")

home_dir = effectiveRefPath
for _ in "abcde":
    home_dir = os.path.dirname(home_dir)
shortpath = os.path.relpath(effectiveRefPath, home_dir)
try:
    exec("import {} as sig_exists".format(module))
    print("found:", shortpath)
    have_refmodule = True
except ImportError:
    print("*** not found:", shortpath)
    have_refmodule = False
except SyntaxError:
    print("*** not a python file, removed:", shortpath)
    os.unlink(effectiveRefPath)
    have_refmodule = False
if have_refmodule and not hasattr(sig_exists, "dict"):
    print("*** wrong module without 'dict', removed:", shortpath)
    os.unlink(effectiveRefPath)
    have_refmodule = False


@unittest.skipIf(not have_refmodule,
                 "not activated for this platform or version")
class TestSignaturesExists(unittest.TestCase):
    """
    This is the current simple attempt to support a signature self test.
    You can activate it for your platform by supplying your own reference
    file. Simply run init_platform.py and add the generated file to the
    repository.
    """
    def test_signatures(self):
        found_sigs = enum_all()
        with isolate_warnings():
            for key, value in sig_exists.dict.items():
                name = key.rsplit(".", 1)[-1]
                if name in ("next", "__next__"): # ignore problematic cases
                    continue
                if key not in found_sigs:
                    warn("missing key: '{}'".format(key))
                elif isinstance(value, list) and len(value) != len(found_sigs[key]):
                    warn("multi-signature count mismatch: '{}'".format(key))
            if is_ci and check_warnings():
                raise RuntimeError("There are errors, see above.")

    def test_error_is_raised(self):
        found_sigs = enum_all()
        # make sure that errors are actually raised
        found_sigs.pop(list(found_sigs.keys())[42])
        with isolate_warnings(), suppress_warnings():
            for key, value in sig_exists.dict.items():
                name = key.rsplit(".", 1)[-1]
                if name in ("next", "__next__"): # ignore problematic cases
                    continue
                if key not in found_sigs:
                    warn("missing key: '{}'".format(key))
                elif isinstance(value, list) and len(value) != len(found_sigs[key]):
                    warn("multi-signature count mismatch: '{}'".format(key))
            self.assertTrue(check_warnings())

tested_versions = (5, 6), (5, 9), (5, 11)

if not have_refmodule and is_ci and qtVersion()[:2] in tested_versions:
    class TestFor_CI_Init(unittest.TestCase):
        """
        This helper class generates the reference file for CI.
        It creates an output listing that can be used to check
        the result back in.
        """
        generate_all()
        sys.stderr.flush()
        print("BEGIN_FILE", shortpath, file=sys.stderr)
        with open(refPath) as f:
            print(f.read(), file=sys.stderr)
        print("END_FILE", shortpath, file=sys.stderr)
        sys.stderr.flush()
        raise RuntimeError(dedent("""
            {line}
            **  This is the initial call. You should check this file in:
            **  {}
            **""").format(shortpath, line=79 * "*"))

if __name__ == '__main__':
    unittest.main()
