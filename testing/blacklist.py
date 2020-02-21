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

from __future__ import print_function

"""
testing/blacklist.py

Take a blacklist file and build classifiers for all tests.

find_matching_line() adds info using classifiers.
"""

from .helper import decorate, StringIO
from .buildlog import builds


class BlackList(object):
    def __init__(self, blname):
        if blname == None:
            f = StringIO()
            self.raw_data = []
        else:
            with open(blname) as f:
                self.raw_data = f.readlines()
        # keep all lines, but see what is not relevant
        lines = self.raw_data[:]

        def filtered_line(line):
            if '#' in line:
                line = line[0 : line.index('#')]
            return line.split()

        # now put every bracketed line in a test
        # and use subsequent identifiers for a match
        def is_test(fline):
            return fline and fline[0].startswith("[")

        self.tests = {}

        if not lines:
            # nothing supplied
            return

        self.index = {}
        for idx, line in enumerate(lines):
            fline = filtered_line(line)
            if not fline:
                continue
            if is_test(fline):
                break
            # we have a global section
            name = ''
            self.tests[name] = []
        for idx, line in enumerate(lines):
            fline = filtered_line(line)
            if is_test(fline):
                # a new name
                name = decorate(fline[0][1:-1])
                self.tests[name] = []
                self.index[name] = idx
            elif fline:
                # a known name with a new entry
                self.tests[name].append(fline)

    def find_matching_line(self, test):
        """
        Take a test result.
        Find a line in the according blacklist file where all keys of the line are found.
        If line not found, do nothing.
        if line found and test passed, it is a BPASS.
        If line found and test failed, it is a BFAIL.
        """
        classifiers = set(builds.classifiers)

        if "" in self.tests:
            # this is a global section
            for line in self.tests[""]:
                keys = set(line)
                if keys <= classifiers:
                    # found a match!
                    return line
        mod_name = test.mod_name
        if mod_name not in self.tests and decorate(mod_name) not in self.tests:
            return None
        if mod_name in self.tests:
            thing = mod_name
        else:
            thing = decorate(mod_name)
        for line in self.tests[thing]:
            keys = set(line)
            if keys <= classifiers:
                # found a match!
                return line
        else:
            return None # nothing found
