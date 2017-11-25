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
import sys
import re
import subprocess

from collections import namedtuple
from textwrap import dedent

from .buildlog import builds
from .helper import decorate, PY3, TimeoutExpired


class TestRunner(object):
    def __init__(self, log_entry, project):
        self.log_entry = log_entry
        built_path = log_entry.build_dir
        self.test_dir = os.path.join(built_path, project)
        log_dir = log_entry.log_dir
        self.logfile = os.path.join(log_dir, project + ".log")
        os.environ['CTEST_OUTPUT_ON_FAILURE'] = '1'
        self._setup()

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

    def _run(self, cmd_tuple, timeout):
        """
        Perform a test run in a given build

        The build can be stopped by a keyboard interrupt for testing
        this script. Also, a timeout can be used.

        After the change to directly using ctest, we no longer use
        "--force-new-ctest-process". Until now this han no drawbacks
        but was a littls faster.
        """

        self.cmd = cmd_tuple
        shell_option = sys.platform == "win32"
        print(dedent("""\
            running {cmd}
                 in {test_dir}
            """).format(**self.__dict__))
        ctest_process = subprocess.Popen(self.cmd,
                                         cwd=self.test_dir,
                                         stderr=subprocess.STDOUT,
                                         shell=shell_option)
        try:
            comm = ctest_process.communicate
            output = (comm(timeout=timeout) if PY3 else comm())[0]
        except (TimeoutExpired, KeyboardInterrupt):
            print()
            print("aborted, partial resut")
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
        ctest_process.wait()

    def run(self, timeout=10 * 60):
        cmd = self.ctestCommand, "--output-log", self.logfile
        self._run(cmd, timeout)
