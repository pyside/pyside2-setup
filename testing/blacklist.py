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
                line = line[0:line.index('#')]
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
        passed = test.passed
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
            return None # noting found


"""
Simplified blacklist file
-------------------------

A comment reaches from '#' to the end of line.
The file starts with an optional global section.
A test is started with a [square-bracketed] section name.
A line matches if all keys in the line are found.
If a line matches, the corresponding test is marked BFAIL or BPASS depending if the test passed or
not.

Known keys are:

darwin
win32
linux
...

qt5.6.1
qt5.6.2
...

py3
py2

32bit
64bit

debug
release
"""

"""
Data Folding v2
===============

In the first layout of data folding, we distinguished complete domains
like "debug/release" and incomplete domains like "ubuntu/win32" which
can be extended to any number.

This version is simpler. We do a first pass over all data and collect
all data. Therefore, incomplete domains do not exist. The definition
of the current members of the domain goes into a special comment at
the beginning of the file.


Compressing a blacklist
-----------------------

When we have many samples of data, it is very likely to get very similar
entries. The redundancy is quite high, and we would like to compress
data without loosing information.

Consider the following data set:

[some::sample_test]
    darwin qt5.6.1 py3 64bit debug
    darwin qt5.6.1 py3 64bit release
    darwin qt5.6.1 py2 64bit debug
    darwin qt5.6.1 py2 64bit release
    win32 qt5.6.1 py3 64bit debug
    win32 qt5.6.1 py3 64bit release
    win32 qt5.6.1 py2 64bit debug
    win32 qt5.6.1 py2 64bit release

The keys "debug" and "release" build the complete set of keys in their
domain. When sorting the lines, we can identify all similar entries which
are only different by the keys "debug" and "release".

[some::sample_test]
    darwin qt5.6.1 py3 64bit
    darwin qt5.6.1 py2 64bit
    win32 qt5.6.1 py3 64bit
    win32 qt5.6.1 py2 64bit

We can do the same for "py3" and "py2", because we have again the complete
set of possible keys available:

[some::sample_test]
    darwin qt5.6.1 64bit
    win32 qt5.6.1 64bit

The operating system has the current keys "darwin" and "win32".
They are kept in a special commend, and we get:

# COMPRESSION: darwin win32
[some::sample_test]
    qt5.6.1 64bit


Expanding a blacklist
---------------------

All of the above steps are completely reversible.


Alternate implementation
------------------------

Instead of using a special comment, I am currently in favor of
the following:

The global section gets the complete set of variables, like so

# Globals
    darwin win32 linux
    qt5.6.1 qt5.6.2
    py3 py2
    32bit 64bit
    debug release
[some::sample_test]
    qt5.6.1 64bit

This approach has the advantage that it does not depend on comments.
The lines in the global section can always added without any conflict,
because these test results are impossible. Therefore, we list all our
keys without adding anything that could influence a test.
It makes also sense to have everything explicitly listed here.
"""

def learn_blacklist(fname, result, selected):
    with open(fname, "r+") as f:
        _remove_from_blacklist(f.name)
        _add_to_blacklist(f.name, result)
        _update_header(f.name, selected)

def _remove_from_blacklist(old_blname):
    # get rid of existing classifiers
    classifierset = set(builds.classifiers)

    # for every line, remove comments and see if the current set is an exact
    # match. We will touch only exact matches.
    def filtered_line(line):
        if '#' in line:
            line = line[0:line.index('#')]
        return line.split()

    with open(old_blname, "r") as f:
        lines = f.readlines()
    deletions = []
    for idx, line in enumerate(lines):
        fline = filtered_line(line)
        if not fline:
            continue
        if '[' in fline[0]:
            # a heading line
            continue
        if set(fline) == classifierset:
            deletions.append(idx)
    while deletions:
        delete = deletions.pop()
        del lines[delete]
    # remove all blank lines, but keep comments
    for idx, line in reversed(list(enumerate(lines))):
        if not line.split():
            del lines[idx]
    # remove all consecutive sections, but keep comments
    for idx, line in reversed(list(enumerate(lines))):
        fline = line.split()
        if fline and fline[0].startswith("["):
            if idx+1 == len(lines):
                # remove section at the end
                del lines[idx]
                continue
            gline = lines[idx+1].split()
            if gline and gline[0].startswith("["):
                # next section right after this, remove this
                del lines[idx]
    with open(old_blname, "w") as f:
        f.writelines(lines)

def _add_to_blacklist(old_blname, result):
    # insert new classifiers
    classifiers = "    " + " ".join(builds.classifiers) + "\n"
    insertions = []
    additions = []
    old_bl = BlackList(old_blname)
    lines = old_bl.raw_data[:]
    if lines and not lines[-1].endswith("\n"):
        lines[-1] += "\n"
    for test in result:
        if test.passed:
            continue
        if test.mod_name in old_bl.tests:
            # the test is already there, add to the first line
            idx = old_bl.index[test.mod_name]
            insertions.append(idx)
        if decorate(test.mod_name) in old_bl.tests:
            # the same, but the name was decorated
            idx = old_bl.index[decorate(test.mod_name)]
            insertions.append(idx)
        else:
            # the test is new, append it to the end
            additions.append("[" + decorate(test.mod_name) + "]\n")
    while insertions:
        this = insertions.pop()
        lines[this] += classifiers
    for line in additions:
        lines.append(line)
        lines.append(classifiers)
    # now write the data out
    with open(old_blname, "r+") as f:
        f.writelines(lines)

def _update_header(old_blname, selected):
    with open(old_blname) as f:
        lines = f.readlines()
    classifierset = set(builds.classifiers)
    for idx, line in reversed(list(enumerate(lines))):
        fline = line.split()
        if fline and fline[0].startswith('#'):
            if set(fline) >= classifierset:
                del lines[idx]

    classifiers = " ".join(builds.classifiers)
    path = selected.log_dir
    base = os.path.basename(path)
    test = '### test date = %s   classifiers = %s\n' % (base, classifiers)
    lines.insert(0, test)
    with open(old_blname, "w") as f:
        f.writelines(lines)
