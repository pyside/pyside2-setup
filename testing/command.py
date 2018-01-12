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
==========

Provide an interface to the pyside tests.
-----------------------------------------

This program can only be run if PySide was build with tests enabled.
All tests are run in a single pass, and if not blacklisted, an error
is raised at the end of the run.

Recommended build process:
There is no need to install the project.
Building the project with something like

    python setup.py build --build-tests --qmake=<qmakepath> --ignore-git --debug

is sufficient. The tests are run by changing into the latest build dir and there
into pyside2, then 'make test'.


New testing policy:
-------------------

The tests are now run 5 times, and errors are reported
when they appear at least 3 times. With the variable COIN_RERUN_FAILED_ONLY it is
possible to configure if all tests should be rerun or the failed ones, only.

The full mode can be tested locally by setting

    export COIN_RERUN_FAILED_ONLY=0
"""

import os
import sys
import argparse
from textwrap import dedent
from collections import OrderedDict
from timeit import default_timer as timer

from .helper import script_dir, decorate
from .buildlog import builds
from .blacklist import BlackList
from .runner import TestRunner
from .parser import TestParser

# Should we repeat only failed tests?
COIN_RERUN_FAILED_ONLY = True
COIN_THRESHOLD = 3    # report error if >=
COIN_TESTING = 5      # number of runs

if (os.environ.get("COIN_RERUN_FAILED_ONLY", "1").lower() in
    "0 f false n no".split()):
    COIN_RERUN_FAILED_ONLY = False

def test_project(project, args, blacklist, runs):
    ret = []

    # remove files from a former run
    for idx in range(runs):
        index = idx + 1
        runner = TestRunner(builds.selected, project, index)
        if os.path.exists(runner.logfile) and not args.skip:
            os.unlink(runner.logfile)
    # now start the real run
    for idx in range(runs):
        index = idx + 1
        runner = TestRunner(builds.selected, project, index)
        print()
        print("********* Start testing of %s *********" % project)
        print("Config: Using", " ".join(builds.classifiers))
        print()
        if os.path.exists(runner.logfile) and args.skip:
            print("Parsing existing log file:", runner.logfile)
        else:
            if index > 1 and COIN_RERUN_FAILED_ONLY:
                rerun = rerun_list
                if not rerun:
                    print("--- no re-runs found, stopping before test {} ---"
                          .format(index))
                    break
            else:
                rerun = None
            runner.run("RUN {}:".format(idx + 1), rerun, 10 * 60)
        result = TestParser(runner.logfile)
        r = 5 * [0]
        rerun_list = []
        print()
        for test, res in result.iter_blacklist(blacklist):
            print("RES {}:".format(index), end=" ")
            print("%-6s" % res, decorate(test) + "()")
            r[0] += 1 if res == "PASS" else 0
            r[1] += 1 if res == "FAIL!" else 0
            r[2] += 1 if res == "SKIPPED" else 0 # not yet supported
            r[3] += 1 if res == "BFAIL" else 0
            r[4] += 1 if res == "BPASS" else 0
            if res not in ("PASS", "BPASS"):
                rerun_list.append(test)
        print()
        print("Totals:", sum(r), "tests.",
              "{} passed, {} failed, {} skipped, {} blacklisted, {} bpassed."
              .format(*r))
        print()
        print("********* Finished testing of %s *********" % project)
        print()
        ret.append(r)

    return ret

def main():
    # create the top-level command parser
    start_time = timer()
    all_projects = "shiboken2 pyside2 pyside2-tools".split()
    tested_projects = "shiboken2 pyside2".split()
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=dedent("""\
        Run the tests for some projects, default = '{}'.

        Testing is now repeated up to {rep} times, and errors are
        only reported if they occur {thr} or more times.
        The environment variable COIN_RERUN_FAILED_ONLY controls if errors
        are only repeated if there are errors. The default is "1".
        """.format("' '".join(tested_projects), thr=COIN_THRESHOLD, rep=COIN_TESTING)))
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
    parser_test.add_argument("--projects", nargs='+', type=str,
                        default=tested_projects,
                        choices=all_projects,
                        help="use '{}'' (default) or other projects"
                        .format("' '".join(tested_projects)))
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

    runs = COIN_TESTING
    fail_crit = COIN_THRESHOLD
    # now loop over the projects and accumulate
    for project in args.projects:
        res = test_project(project, args, bl, runs)
        for idx, r in enumerate(res):
            q = list(map(lambda x, y: x+y, r, q))

    if len(args.projects) > 1:
        print("All above projects:", sum(q), "tests.",
              "{} passed, {} failed, {} skipped, {} blacklisted, {} bpassed."
              .format(*q))
        print()

    tot_res = OrderedDict()
    for project in args.projects:
        for idx in range(runs):
            index = idx + 1
            runner = TestRunner(builds.selected, project, index)
            result = TestParser(runner.logfile)
            for test, res in result.iter_blacklist(bl):
                key = project + ":" + test
                tot_res.setdefault(key, [])
                tot_res[key].append(res)
    tot_flaky = 0
    print("*" * 79)
    print("**")
    print("*   Summary Of All Tests")
    print("*")
    empty = True
    for test, res in tot_res.items():
        pass__c = res.count("PASS")
        bpass_c = res.count("BPASS")
        fail__c = res.count("FAIL!")
        bfail_c = res.count("BFAIL")
        fail2_c = fail__c + bfail_c
        if pass__c == len(res):
            continue
        elif bpass_c == runs and runs > 1:
            msg = "Remove blacklisting; test passes"
        elif fail__c == runs:
            msg = "Newly detected Real test failure!"
        elif bfail_c == runs:
            msg = "Keep blacklisting ;-("
        elif fail2_c > 0 and fail2_c < len(res):
            msg = "Flaky test"
            tot_flaky += 1
        else:
            continue
        empty = False
        padding = 6 * runs
        txt = " ".join(("{:<{width}}".format(piece, width=5) for piece in res))
        txt = (txt + padding * " ")[:padding]
        testpad = 36
        if len(test) < testpad:
            test += (testpad - len(test)) * " "
        print(txt, decorate(test), msg)
    if empty:
        print("*   (empty)")
    print("*")
    print("**")
    print("*" * 79)
    print()
    if runs > 1:
        print("Total flaky tests: errors but not always = {}".format(tot_flaky))
        print()
    else:
        print("For info about flaky tests, we need to perform more than one run.")
        print("Please activate the COIN mode:    'export QTEST_ENVIRONMENT=ci'")
        print()
    # nag us about unsupported projects
    ap, tp = set(all_projects), set(tested_projects)
    if ap != tp:
        print("+++++ Note: please support", " ".join(ap-tp), "+++++")
        print()

    stop_time = timer()
    used_time = stop_time - start_time
    # Now create an error if the criterion is met:
    try:
        err_crit = "'FAIL! >= {}'".format(fail_crit)
        for res in tot_res.values():
            if res.count("FAIL!") >= fail_crit:
                raise ValueError("At least one failure was not blacklisted "
                                 "and met the criterion {}"
                                 .format(err_crit))
        print("No test met the error criterion {}".format(err_crit))
    finally:
        print()
        print("Total time of whole Python script = {:0.2f} sec".format(used_time))
        print()
# eof
