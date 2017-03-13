#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
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

"""
testrunner

Provide an interface to the pyside tests.

- find the latest build dir.
  This is found by the build_history in setup.py,
  near the end of pyside_build.run()

- run 'make test' and record the output
  (not ready)

- compare the result list with the current blacklist

- return the correct error status
  (zero if expected includes observed, else 1)

Recommended build process:
There is no need to install the project.
Building the project with something like

    python setup.py build --build-tests --qmake=<qmakepath> --ignore-git --debug

is sufficient. The tests are run by changing into the latest build dir and there
into pyside2, then 'make test'.

"""

import os
import sys
import re
import subprocess
import zipfile
import argparse

PY3 = sys.version_info[0] == 3  # from the six module
from subprocess import PIPE
if PY3:
    from subprocess import TimeoutExpired
    from io import StringIO
else:
    class SubprocessError(Exception): pass
    # this is a fake, just to keep the source compatible.
    # timeout support is in python 3.3 and above.
    class TimeoutExpired(SubprocessError): pass
    from StringIO import StringIO
from collections import namedtuple

# Change the cwd to our source dir
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))
script_dir = os.getcwd()

LogEntry = namedtuple("LogEntry", ["log_dir", "build_dir"])


class BuildLog(object):
    """
    This class is a convenience wrapper around a list of log entries.

    The list of entries is sorted by date and checked for consistency.
    For simplicity and readability, the log entries are named tuples.

    """
    def __init__(self, script_dir=script_dir):
        history_dir = os.path.join(script_dir, 'build_history')
        build_history = []
        for timestamp in os.listdir(history_dir):
            log_dir = os.path.join(history_dir, timestamp)
            if not os.path.isdir(log_dir):
                continue
            fpath = os.path.join(log_dir, 'build_dir.txt')
            if not os.path.exists(fpath):
                print("Warning: %s not found, skipped" % fpath)
                continue
            with open(fpath) as f:
                build_dir = f.read().strip()
                if not os.path.exists(build_dir):
                    rel_dir, low_part = os.path.split(build_dir)
                    rel_dir, two_part = os.path.split(rel_dir)
                    if two_part.startswith("pyside") and two_part.endswith("build"):
                        build_dir = os.path.abspath(os.path.join(two_part, low_part))
                        if os.path.exists(build_dir):
                            print("Note: build_dir was probably moved.")
                        else:
                            print("Warning: missing build dir %s" % build_dir)
                            continue
            entry = LogEntry(log_dir, build_dir)
            build_history.append(entry)
        # we take the latest build for now.
        build_history.sort()
        self.history = build_history
        self._buildno = None

    def set_buildno(self, buildno):
        self.history[buildno] # test
        self._buildno = buildno

    @property
    def selected(self):
        if self._buildno is None:
            return None
        if self.history is None:
            return None
        return self.history[self._buildno]

    @property
    def classifiers(self):
        if not self.selected:
            raise ValueError('+++ No build with the configuration found!')
        # Python2 legacy: Correct 'linux2' to 'linux', recommended way.
        platform = 'linux' if sys.platform.startswith('linux') else sys.platform
        res = [platform]
        # the rest must be guessed from the given filename
        path = self.selected.build_dir
        base = os.path.basename(path)
        res.extend(base.split('-'))
        # add the python version py2 and py3
        # also add the keys qt5 and qt5.6 etc.
        for entry in res:
            if entry.startswith("py"):
                key = entry[:3]
                if key not in res:
                    res.append(key)
            if entry.startswith("qt"):
                key = entry[:3]
                if key not in res:
                    res.append(key)
                key = entry[:5]
                if key not in res:
                    res.append(key)
            # this will become more difficult when the version has two digits
        return res


class TestRunner(object):
    def __init__(self, log_entry, project):
        self.log_entry = log_entry
        built_path = log_entry.build_dir
        self.test_dir = os.path.join(built_path, project)
        log_dir = log_entry.log_dir
        self.logfile = os.path.join(log_dir, project + ".log")
        os.environ['CTEST_OUTPUT_ON_FAILURE'] = '1'
        self._setup()

    def _setup(self):
        if sys.platform == 'win32':
            # Windows: Helper implementing 'which' command using 'where.exe'
            def winWhich(binary):
                cmd = ['where.exe', binary]
                stdOut = subprocess.Popen(cmd, stdout=subprocess.PIPE).stdout
                result = stdOut.readlines()
                stdOut.close()
                if len(result) > 0:
                    return re.compile('\\s+').sub(' ', result[0].decode('utf-8'))
                return None

            self.makeCommand = 'nmake'
            qmakeSpec = os.environ.get('QMAKESPEC')
            if qmakeSpec is not None and 'g++' in qmakeSpec:
                self.makeCommand = 'mingw32-make'
            # Can 'tee' be found in the environment (MSYS-git installation with usr/bin in path)?
            self.teeCommand = winWhich('tee.exe')
            if self.teeCommand is None:
                git = winWhich('git.exe')
                if 'cmd' in git:
                    # Check for a MSYS-git installation with 'cmd' in the path and grab 'tee' from usr/bin
                    index = git.index('cmd')
                    self.teeCommand = git[0:index] + 'usr\\bin\\tee.exe'
        else:
            self.makeCommand = 'make'
            self.teeCommand = 'tee'

    def run(self, timeout = 300):
        """
        perform a test run in a given build. The build can be stopped by a
        keyboard interrupt for testing this script. Also, a timeout can
        be used.
        """

        if sys.platform == "win32":
            cmd = (self.makeCommand, 'test')
            tee_cmd = (self.teeCommand, self.logfile)
            print("running", cmd, 'in', self.test_dir, ',\n  logging to', self.logfile, 'using ', tee_cmd)
            make = subprocess.Popen(cmd, cwd=self.test_dir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            tee = subprocess.Popen(tee_cmd, cwd=self.test_dir, stdin=make.stdout, shell=True)
        else:
            cmd = (self.makeCommand, 'test')
            tee_cmd = (self.teeCommand, self.logfile)
            print("running", cmd, 'in', self.test_dir, ',\n  logging to', self.logfile, 'using ', tee_cmd)
            make = subprocess.Popen(cmd, cwd=self.test_dir, stdout=subprocess.PIPE)
            tee = subprocess.Popen(tee_cmd, cwd=self.test_dir, stdin=make.stdout)
        make.stdout.close()
        try:
            if PY3:
                output = tee.communicate(timeout=timeout)[0]
            else:
                output = tee.communicate()[0]
        except (TimeoutExpired, KeyboardInterrupt):
            print()
            print("aborted")
            tee.kill()
            make.kill()
            outs, errs = tee.communicate()
        finally:
            print("End of the test run")
        tee.wait()


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

def decorate(mod_name):
    """
    Write the combination of "modulename_funcname"
    in the Qt-like form "modulename::funcname"
    """
    if "_" not in mod_name:
        return mod_name
    if "::" in mod_name:
        return mod_name
    name, rest = mod_name.split("_", 1)
    return name + "::" + rest

def create_read_write(filename):
    if os.path.isfile(filename):
        # existing file, open for read and write
        return open(filename, 'r+')
    elif os.path.exists(filename):
        # a directory?
        raise argparse.ArgumentTypeError(None, "invalid file argument: %s" % filename)
    else:
        try:
            return open(filename, 'w')
        except IOError:
            raise argparse.ArgumentError(None, "cannot create file: %s" % filename)

def learn_blacklist(fname, result, selected):
    with open(fname, "r+") as f:
        _remove_from_blacklist(f.name)
        _add_to_blacklist(f.name, result)
        _update_header(f.name, selected)

def _remove_from_blacklist(old_blname):
    # get rid of existing classifiers
    classifierset = set(builds.classifiers)

    # for every line, remove comments and see if the current set if an exact
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


if __name__ == '__main__':
    # create the top-level parser
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="subparser_name")

    # create the parser for the "test" command
    parser_test = subparsers.add_parser("test")
    group = parser_test.add_mutually_exclusive_group(required=False)
    group.add_argument("--blacklist", "-b", type=argparse.FileType('r'),
                        help="a Qt blacklist file")
    group.add_argument("--learn", "-l", type=create_read_write,
                        help="add new entries to a blacklist file")
    parser_test.add_argument("--skip", action='store_true',
                        help="skip the tests if they were run before")
    parser_test.add_argument("--environ", nargs='+',
                        help="use name=value ... to set environment variables")
    parser_test.add_argument("--buildno", default=-1, type=int,
                        help="use build number n (0-based), latest = -1 (default)")
    all_projects = "shiboken2 pyside2 pyside2-tools".split()
    tested_projects = "shiboken2 pyside2".split()
    parser_test.add_argument("--projects", nargs='+', type=str,
                        default=tested_projects,
                        choices=all_projects,
                        help="use 'pyside2' (default) or other projects")
    parser_getcwd = subparsers.add_parser("getcwd")
    parser_getcwd.add_argument("filename", type=argparse.FileType('w'),
                        help="write the build dir name into a file")
    parser_getcwd.add_argument("--buildno", default=-1, type=int,
                        help="use build number n (0-based), latest = -1 (default)")
    parser_list = subparsers.add_parser("list")
    args = parser.parse_args()

    builds = BuildLog(script_dir)
    if hasattr(args, "buildno"):
        try:
            builds.set_buildno(args.buildno)
        except IndexError:
            print("history out of range. Try '%s list'" % __file__)
            sys.exit(1)

    if args.subparser_name == "getcwd":
        print(builds.selected.build_dir, file=args.filename)
        print(builds.selected.build_dir, "written to file", args.filename.name)
        sys.exit(0)
    elif args.subparser_name == "test":
        pass # we do it afterwards
    elif args.subparser_name == "list":
        rp = os.path.relpath
        print()
        print("History")
        print("-------")
        for idx, build in enumerate(builds.history):
            print(idx, rp(build.log_dir), rp(build.build_dir))
        print()
        print("Note: only the last history entry of a folder is valid!")
        sys.exit(0)
    else:
        parser.print_help()
        sys.exit(1)

    if args.blacklist:
        args.blacklist.close()
        bl = BlackList(args.blacklist.name)
    elif args.learn:
        args.learn.close()
        learn_blacklist(args.learn.name, result.result, builds.selected)
        bl = BlackList(args.learn.name)
    else:
        bl = BlackList(None)
    if args.environ:
        for line in args.environ:
            things = line.split("=")
            if len(things) != 2:
                raise ValueError("you need to pass one or more name=value pairs.")
            key, value = things
            os.environ[key] = value

    q = 5 * [0]

    # now loop over the projects and accumulate
    for project in args.projects:
        runner = TestRunner(builds.selected, project)
        if os.path.exists(runner.logfile) and args.skip:
            print("Parsing existing log file:", runner.logfile)
        else:
            runner.run(10 * 60)
        result = TestParser(runner.logfile)
        r = 5 * [0]
        print("********* Start testing of %s *********" % project)
        print("Config: Using", " ".join(builds.classifiers))
        for test, res in result.iter_blacklist(bl):
            print("%-6s" % res, ":", decorate(test) + "()")
            r[0] += 1 if res == "PASS" else 0
            r[1] += 1 if res == "FAIL" else 0
            r[2] += 1 if res == "SKIPPED" else 0 # not yet supported
            r[3] += 1 if res == "BFAIL" else 0
            r[4] += 1 if res == "BPASS" else 0
        print("Totals:", sum(r), "tests.",
              "{} passed, {} failed, {} skipped, {} blacklisted, {} bpassed."
              .format(*r))
        print("********* Finished testing of %s *********" % project)
        print()
        q = list(map(lambda x, y: x+y, r, q))

    if len(args.projects) > 1:
        print("All above projects:", sum(q), "tests.",
              "{} passed, {} failed, {} skipped, {} blacklisted, {} bpassed."
              .format(*q))
        print()

    # nag us about unsupported projects
    ap, tp = set(all_projects), set(tested_projects)
    if ap != tp:
        print("+++++ Note: please support", " ".join(ap-tp), "+++++")
        print()

    for project in args.projects:
        runner = TestRunner(builds.selected, project)
        result = TestParser(runner.logfile)
        for test, res in result.iter_blacklist(bl):
            if res == "FAIL":
                raise ValueError("At least one failure was not blacklisted")
        # the makefile does run, although it does not find any tests.
        # We simply check if any tests were found.
        if len(result) == 0:
            path = builds.selected.build_dir
            project = os.path.join(path, args.project)
            raise ValueError("there are no tests in %s" % project)
