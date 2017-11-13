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

from __future__ import print_function, absolute_import

import os
import sys
import unittest
import warnings
from init_platform import enum_all, generate_all, is_ci, outname
from util import isolate_warnings, check_warnings
from PySide2 import *

refmodule_name = outname[:-3] # no .py

sys.path.insert(0, os.path.dirname(__file__))
try:
    exec("import {} as sig_exists".format(refmodule_name))
    print("found:", refmodule_name)
    have_refmodule = True
except ImportError:
    print("*** not found:", refmodule_name)
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
                if key not in found_sigs:
                    warnings.warn("missing key: '{}'".format(key), RuntimeWarning)
                elif isinstance(value, list) and len(value) != len(found_sigs[key]):
                    warnings.warn("different sig length: '{}'".format(key), RuntimeWarning)
            if is_ci and check_warnings():
                raise RuntimeError("There are errors, see above.")

    def test_error_is_raised(self):
        found_sigs = enum_all()
        # make sure that errors are actually raised
        found_sigs.pop(list(found_sigs.keys())[42])
        with isolate_warnings():
            for key, value in sig_exists.dict.items():
                if key not in found_sigs:
                    warnings.warn("ignore missing key: '{}'".format(key), RuntimeWarning)
                elif isinstance(value, list) and len(value) != len(found_sigs[key]):
                    warnings.warn("ignore different sig length: '{}'".format(key), RuntimeWarning)
            self.assertTrue(check_warnings())

if not have_refmodule and is_ci:
    class TestFor_CI_Init(unittest.TestCase):
        """
        This helper class generates the reference file for CI.
        It creates an output listing that can be used to check
        the result back in.
        """
        fname = generate_all()
        print("BEGIN", fname, file=sys.stderr)
        with open(fname) as f:
            print(f.read(), file=sys.stderr)
        print("END", fname, file=sys.stderr)
        raise RuntimeError("This is the initial call. You should check this file in.")

if __name__ == '__main__':
    unittest.main()
