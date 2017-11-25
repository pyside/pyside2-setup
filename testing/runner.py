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
                if not git:
                    # In COIN we have only git.cmd in path
                    git = winWhich('git.cmd')
                if 'cmd' in git:
                    # Check for a MSYS-git installation with 'cmd' in the path and grab 'tee' from usr/bin
                    index = git.index('cmd')
                    self.teeCommand = git[0:index] + 'bin\\tee.exe'
                    if not os.path.exists(self.teeCommand):
                        self.teeCommand = git[0:index] + 'usr\\bin\\tee.exe' # git V2.8.X
                    if not os.path.exists(self.teeCommand):
                        raise "Cannot locate 'tee' command"

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
