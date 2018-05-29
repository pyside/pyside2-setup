#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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
from build_scripts.utils import has_option
from build_scripts.utils import option_value
from build_scripts.utils import install_pip_dependencies
from build_scripts.utils import get_qtci_virtualEnv
from build_scripts.utils import run_instruction
from build_scripts.utils import rmtree
import os

# Values must match COIN thrift
CI_HOST_OS = option_value("os")
CI_TARGET_OS = option_value("targetOs")
CI_HOST_ARCH = option_value("hostArch")
CI_TARGET_ARCH = option_value("targetArch")
CI_HOST_OS_VER = option_value("osVer")
CI_ENV_INSTALL_DIR = option_value("instdir")
CI_ENV_AGENT_DIR = option_value("agentdir")
CI_COMPILER = option_value("compiler")
CI_FEATURES = []
_ci_features = option_value("features")
if _ci_features is not None:
    for f in _ci_features.split(', '):
        CI_FEATURES.append(f)
CI_RELEASE_CONF = has_option("packaging")


def call_setup(python_ver):
    _pExe, _env, env_pip, env_python = get_qtci_virtualEnv(python_ver, CI_HOST_OS, CI_HOST_ARCH, CI_TARGET_ARCH)
    rmtree(_env, True)
    run_instruction(["virtualenv", "-p", _pExe,  _env], "Failed to create virtualenv")
    install_pip_dependencies(env_pip, ["six", "wheel"])
    cmd = [env_python, "setup.py"]
    # With 5.11 CI will create two sets of release binaries, one with msvc 2015 and one with msvc 2017
    # we shouldn't release the 2015 version.
    if CI_RELEASE_CONF and CI_COMPILER not in ["MSVC2017"]:
        cmd += ["bdist_wheel", "--standalone"]
    else:
        cmd += ["build"]
    if CI_HOST_OS == "MacOS":
        cmd += ["--qmake=" + CI_ENV_INSTALL_DIR + "/bin/qmake"]
    elif CI_HOST_OS == "Windows":

        cmd += ["--qmake=" + CI_ENV_INSTALL_DIR + "\\bin\\qmake.exe",
                "--openssl=C:\\openssl\\bin"]
    else:
        cmd += ["--qmake=" + CI_ENV_INSTALL_DIR + "/bin/qmake"]
    cmd += ["--build-tests",
            "--jobs=4",
            "--verbose-build",
            "--snapshot-build"]

    run_instruction(cmd, "Failed to run setup.py")

def run_build_instructions():
    # Disable unsupported configs for now
    if CI_HOST_OS_VER in ["WinRT_10"]:
        print("Disabled " + CI_HOST_OS_VER + " from Coin configuration")
        exit()
    if CI_HOST_ARCH == "X86_64" and CI_TARGET_ARCH == "X86":
        print("Disabled 32 bit build on 64 bit from Coin configuration, until toolchains provisioned")
        exit()

    # Uses default python, hopefully we have python2 installed on all hosts
    call_setup("")

    # In case of packaging build, we have to build also python3 wheel
    if CI_RELEASE_CONF and CI_HOST_OS_VER not in ["RHEL_6_6"]:
        call_setup("3")

if __name__ == "__main__":
    run_build_instructions()
