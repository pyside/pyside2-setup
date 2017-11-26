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
import argparse
from textwrap import dedent

from .helper import script_dir, decorate
from .buildlog import builds
from .blacklist import BlackList
from .runner import TestRunner
from .parser import TestParser

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

def main():
    # create the top-level command parser
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="subparser_name")

    # create the parser for the "test" command
    parser_test = subparsers.add_parser("test")
    group = parser_test.add_mutually_exclusive_group(required=False)
    blacklist_default = os.path.join(script_dir, 'build_history', 'blacklist.txt')
    group.add_argument("--blacklist", "-b", type=argparse.FileType('r'),
                       default=blacklist_default,
                       help='a Qt blacklist file (default: {})'.format(blacklist_default))
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
    else:
        bl = BlackList(None)
    if args.environ:
        for line in args.environ:
            things = line.split("=")
            if len(things) != 2:
                raise ValueError("you need to pass one or more name=value pairs.")
            key, value = things
            os.environ[key] = value

    print(dedent("""\
        System:
          Platform={platform}
          Executable={executable}
          Version={version_lf}
          API version={api_version}

        Environment:""")
        .format(version_lf=sys.version.replace("\n", " "), **sys.__dict__))
    for key, value in sorted(os.environ.items()):
        print("  {}={}".format(key, value))
    print()

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
