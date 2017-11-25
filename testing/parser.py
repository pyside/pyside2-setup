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

import os
import re
from collections import namedtuple
from .helper import StringIO

_EXAMPLE = """
Example output:

ip1 n        sharp mod_name                                            code      tim
-----------------------------------------------------------------------------------------
114/391 Test #114: QtCore_qfileinfo_test-42 ........................   Passed    0.10 sec
        Start 115: QtCore_qfile_test
115/391 Test #115: QtCore_qfile_test ...............................***Failed    0.11 sec
        Start 116: QtCore_qflags_test

We will only look for the dotted lines and calculate everything from that.
The summary statistics at the end will be ignored. That allows us to test
this functionality with short timeout values.

Note the field "mod_name". I had split this before, but it is necessary
to use the combination as the key, because the test names are not unique.
"""

# validation of our pattern:

_TEST_PAT = r"""
              ^                          # start
              \s*                        # any whitespace ==: WS
              ([0-9]+)/([0-9]+)          #                         ip1 "/" n
              \s+                        # some WS
              Test                       #                         "Test"
              \s+                        # some WS
              \#                         # sharp symbol            "#"
              ([0-9]+)                   #                         sharp
              :                          # colon symbol            ':'
              \s+                        # some WS
              ([\w-]+)                   #                         mod_name
              .*?                        # whatever (non greedy)
              (                          #
                (Passed)                 # either "Passed", None
                |                        #
                \*\*\*(\w+.*?)           # or     None, "Something"
              )                          #                         code
              \s+                        # some WS
              ([0-9]+\.[0-9]+)           #                         tim
              \s+                        # some WS
              sec                        #                         "sec"
              \s*                        # any WS
              $                          # end
              """
assert re.match(_TEST_PAT, _EXAMPLE.splitlines()[5], re.VERBOSE)
assert len(re.match(_TEST_PAT, _EXAMPLE.splitlines()[5], re.VERBOSE).groups()) == 8
assert len(re.match(_TEST_PAT, _EXAMPLE.splitlines()[7], re.VERBOSE).groups()) == 8

TestResult = namedtuple("TestResult", ["idx", "mod_name", "passed",
                                       "code", "time"])
def _parse_tests(test_log):
    """
    Create a TestResult object for every entry.
    """
    result = []
    if isinstance(test_log, StringIO):
        lines = test_log.readlines()
    elif test_log is not None and os.path.exists(test_log):
        with open(test_log) as f:
            lines = f.readlines()
    else:
        lines = []
    pat = _TEST_PAT
    for line in lines:
        match = re.match(pat, line, re.VERBOSE)
        if match:
            idx, n, sharp, mod_name, much_stuff, code1, code2, tim = tup = match.groups()
            # either code1 or code2 is None
            code = code1 or code2
            if idx != sharp:
                raise ValueError("inconsistent log lines or program error: %s" % tup)
            idx, n, code, tim = int(idx), int(n), code.lower(), float(tim)
            res = TestResult(idx, mod_name, code == "passed", code, tim)
            result.append(res)
    return result


class TestParser(object):
    def __init__(self, test_log):
        self._result = _parse_tests(test_log)

    @property
    def result(self):
        return self._result

    def __len__(self):
        return len(self._result)

    def iter_blacklist(self, blacklist):
        bl = blacklist
        for line in self._result:
            mod_name = line.mod_name
            passed = line.passed
            match = bl.find_matching_line(line)
            if not passed:
                if match:
                    res = "BFAIL"
                else:
                    res = "FAIL"
            else:
                if match:
                    res = "BPASS"
                else:
                    res = "PASS"
            yield mod_name, res
