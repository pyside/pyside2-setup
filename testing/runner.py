#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
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
import sys
import re
import subprocess
import inspect

from collections import namedtuple
from textwrap import dedent

from .buildlog import builds
from .helper import decorate, PY3, TimeoutExpired

# Get the dir path to the utils module
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
this_dir = os.path.dirname(this_file)
build_scripts_dir = os.path.abspath(os.path.join(this_dir, '../build_scripts'))

sys.path.append(build_scripts_dir)
from utils import detect_clang

class TestRunner(object):
    def __init__(self, log_entry, project, index):
        self.log_entry = log_entry
        built_path = log_entry.build_dir
        self.test_dir = os.path.join(built_path, project)
        log_dir = log_entry.log_dir
        if index is not None:
            self.logfile = os.path.join(log_dir, project + ".{}.log".format(index))
        else:
            self.logfile = os.path.join(log_dir, project + ".log")
        os.environ['CTEST_OUTPUT_ON_FAILURE'] = '1'
        self._setup_clang()
        self._setup()

    def _setup_clang(self):
        if sys.platform != "win32":
            return
        clang_dir = detect_clang()
        if clang_dir[0]:
            clang_bin_dir = os.path.join(clang_dir[0], 'bin')
            path = os.environ.get('PATH')
            if not clang_bin_dir in path:
                os.environ['PATH'] = clang_bin_dir + os.pathsep + path
                print("Adding %s as detected by %s to PATH" % (clang_bin_dir, clang_dir[1]))

    def _find_ctest(self):
        """
        Find ctest in the Makefile

        We no longer use make, but the ctest command directly.
        It is convenient to look for the ctest program using the Makefile.
        This serves us two purposes:

          - there is no dependency of the PATH variable,
          - each project is checked whether ctest was configured.
        """
        make_path = os.path.join(self.test_dir, "Makefile")
        look_for = "--force-new-ctest-process"
        line = None
        with open(make_path) as makefile:
            for line in makefile:
                if look_for in line:
                    break
            else:
                # We have probably forgotten to build the tests.
                # Give a nice error message with a shortened but exact path.
                rel_path = os.path.relpath(make_path)
                msg = dedent("""\n
                    {line}
                    **  ctest is not in '{}'.
                    *   Did you forget to build the tests with '--build-tests' in setup.py?
                    """).format(rel_path, line=79 * "*")
                raise RuntimeError(msg)
        # the ctest program is on the left to look_for
        assert line, "Did not find {}".format(look_for)
        ctest = re.search(r'(\S+|"([^"]+)")\s+' + look_for, line).groups()
        return ctest[1] or ctest[0]

    def _setup(self):
        self.ctestCommand = self._find_ctest()

    def _run(self, cmd_tuple, label, timeout):
        """
        Perform a test run in a given build

        The build can be stopped by a keyboard interrupt for testing
        this script. Also, a timeout can be used.

        After the change to directly using ctest, we no longer use
        "--force-new-ctest-process". Until now this has no drawbacks
        but was a little faster.
        """

        self.cmd = cmd_tuple
        # We no longer use the shell option. It introduces wrong handling
        # of certain characters which are not yet correctly escaped:
        # Especially the "^" caret char is treated as an escape, and pipe symbols
        # without a caret are interpreted as such which leads to weirdness.
        # Since we have all commands with explicit paths and don't use shell
        # commands, this should work fine.
        print(dedent("""\
            running {cmd}
                 in {test_dir}
            """).format(**self.__dict__))
        ctest_process = subprocess.Popen(self.cmd,
                                         cwd=self.test_dir,
                                         stdout=subprocess.PIPE,
                                         stderr=subprocess.STDOUT)
        def py_tee(input, output, label):
            '''
            A simple (incomplete) tee command in Python

            This script simply logs everything from input to output
            while the output gets some decoration. The specific reason
            to have this script at all is:

            - it is necessary to have some decoration as prefix, since
              we run commands several times

            - collecting all output and then decorating is not nice if
              you have to wait for a long time

            The special escape is for the case of an embedded file in
            the output.
            '''
            def xprint(*args, **kw):
                print(*args, file=output, **kw)

            # 'for line in input:' would read into too large chunks
            labelled = True
            while True:
                line = input.readline()
                if not line:
                    break
                if line.startswith('BEGIN_FILE'):
                    labelled = False
                txt = line.rstrip()
                xprint(label, txt) if label and labelled else xprint(txt)
                if line.startswith('END_FILE'):
                    labelled = True

        tee_src = dedent("""\
            from __future__ import print_function
            import sys
            {}
            py_tee(sys.stdin, sys.stdout, '{label}')
            """).format(dedent(inspect.getsource(py_tee)), label=label)
        tee_cmd = (sys.executable, "-E", "-u", "-c", tee_src)
        tee_process = subprocess.Popen(tee_cmd,
                                       cwd=self.test_dir,
                                       stdin=ctest_process.stdout)
        try:
            comm = tee_process.communicate
            output = (comm(timeout=timeout) if PY3 else comm())[0]
        except (TimeoutExpired, KeyboardInterrupt):
            print()
            print("aborted, partial result")
            ctest_process.kill()
            outs, errs = ctest_process.communicate()
            # ctest lists to a temp file. Move it to the log
            tmp_name = self.logfile + ".tmp"
            if os.path.exists(tmp_name):
                if os.path.exists(self.logfile):
                    os.unlink(self.logfile)
                os.rename(tmp_name, self.logfile)
            self.partial = True
        else:
            self.partial = False
        finally:
            print("End of the test run")
            print()
        tee_process.wait()

    def run(self, label, rerun, timeout):
        cmd = self.ctestCommand, "--output-log", self.logfile
        if rerun is not None:
            # cmd += ("--rerun-failed",)
            # For some reason, this worked never in the script file.
            # We pass instead the test names as a regex:
            words = "^(" + "|".join(rerun) + ")$"
            cmd += ("--tests-regex", words)
        self._run(cmd, label, timeout)
# eof
