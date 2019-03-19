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
from build_scripts.options import has_option
from build_scripts.options import option_value
from build_scripts.utils import install_pip_dependencies
from build_scripts.utils import install_pip_wheel_package
from build_scripts.utils import get_qtci_virtualEnv
from build_scripts.utils import run_instruction
from build_scripts.utils import rmtree
from build_scripts.utils import acceptCITestConfiguration
from build_scripts.utils import get_ci_qmake_path
import os

# Values must match COIN thrift
CI_HOST_OS = option_value("os")
CI_TARGET_OS = option_value("targetOs")
CI_HOST_ARCH = option_value("hostArch")
CI_TARGET_ARCH = option_value("targetArch")
CI_HOST_OS_VER = option_value("osVer")
CI_ENV_INSTALL_DIR = option_value("instdir")
CI_ENV_AGENT_DIR = option_value("agentdir") or "."
CI_COMPILER = option_value("compiler")
CI_FEATURES = []
_ci_features = option_value("features")
if _ci_features is not None:
    for f in _ci_features.split(', '):
        CI_FEATURES.append(f)

CI_RELEASE_CONF = has_option("packaging")

def call_testrunner(python_ver, buildnro):
    _pExe, _env, env_pip, env_python = get_qtci_virtualEnv(python_ver, CI_HOST_OS, CI_HOST_ARCH, CI_TARGET_ARCH)
    rmtree(_env, True)
    run_instruction(["virtualenv", "-p", _pExe,  _env], "Failed to create virtualenv")
    install_pip_dependencies(env_pip, ["numpy", "PyOpenGL", "setuptools", "six", "pyinstaller"])
    install_pip_wheel_package(env_pip)
    cmd = [env_python, "testrunner.py", "test",
                  "--blacklist", "build_history/blacklist.txt",
                  "--buildno=" + buildnro]
    run_instruction(cmd, "Failed to run testrunner.py")

    qmake_path = get_ci_qmake_path(CI_ENV_INSTALL_DIR, CI_HOST_OS)

    # Try to install built wheels, and build some buildable examples.
    if CI_RELEASE_CONF:
        wheel_tester_path = os.path.join("testing", "wheel_tester.py")
        cmd = [env_python, wheel_tester_path, qmake_path]
        run_instruction(cmd, "Error while running wheel_tester.py")

def run_test_instructions():
    if not acceptCITestConfiguration(CI_HOST_OS, CI_HOST_OS_VER, CI_TARGET_ARCH, CI_COMPILER):
        exit()

    # Remove some environment variables that impact cmake
    for env_var in ['CC', 'CXX']:
        if os.environ.get(env_var):
            del os.environ[env_var]

    os.chdir(CI_ENV_AGENT_DIR)
    testRun = 0
    # We didn't build for Python 2 in win
    if CI_HOST_OS != "Windows":
        call_testrunner("", str(testRun))
        testRun =+ 1
    # We know that second build was with python3
    if CI_RELEASE_CONF and CI_HOST_OS_VER not in ["RHEL_6_6"]:
        call_testrunner("3", str(testRun))

if __name__ == "__main__":
    run_test_instructions()
