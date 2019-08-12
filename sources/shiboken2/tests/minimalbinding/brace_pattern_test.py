#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

from __future__ import absolute_import, print_function

import re
import sys
import os

import shiboken2
type.__signature__   # trigger bootstrap

from shibokensupport.signature.lib.tool import build_brace_pattern
import unittest

"""
This test tests the brace pattern from signature.lib.tool
against a slower reference implementation.
The pattern is crucial, because it is used heavily in signature.parser .
"""

# A slow reference parser for braces and strings
def check(s):
    open, close = "[{(<", "]})>"
    escape, quote = "\\", '"'
    instring = blind = False
    stack = []
    for c in s:
        if instring:
            if blind:
                blind = False
            elif c == escape:
                blind = True
            elif c == quote:
                instring = False
                stack.pop()
            continue
        if c in open:
            stack.append(c)
        elif c in close:
            pos = close.index(c)
            if ((len(stack) > 0) and
                (open[pos] == stack[len(stack)-1])):
                stack.pop()
            else:
                return False
        elif c == escape:
            return False
        elif c == quote:
            instring = True
            stack.append(c)
    return len(stack) == 0


class TestBracePattern(unittest.TestCase):
    tests = [
        (r'{[]{()}}', True),
        (r'[{}{})(]', False),
        (r'[{}{} ")(" ]', True),
        (r'[{}{} ")(\" ]', False),
        (r'[{}{} ")(\" ]"]', True),
        (r'a < b ( c [ d { "} \"\"}" } ] ) >', True),
        (r'a < b ( c [ d {  } ] ) >', True),
        (r'a < b ( c [ d { "huh" } ] ) >', True),
        (r'a < b ( c [ d { "huh\" \" \\\"" } ] ) >', True),
    ]

    def test_checkfunc(self):
        for test, result in self.tests:
            if result:
                self.assertTrue(check(test))
            else:
                self.assertFalse(check(test))

    def test_the_brace_pattern(self):
        func = re.compile(build_brace_pattern(5) + "$", re.VERBOSE).match
        for test, result in self.tests:
            if result:
                self.assertTrue(func(test))
            else:
                self.assertFalse(func(test))


if __name__ == '__main__':
    unittest.main()
