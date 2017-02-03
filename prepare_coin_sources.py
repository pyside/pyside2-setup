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
import os
import sys
import shutil
from subprocess import PIPE, Popen
from utils import option_value
from utils import run_process

git_server = "git://code.qt.io"

QT_CI_TESTED_SUBMODULE = option_value("module")
SUBMODULE_BRANCH = option_value("branch")

submodules = {
        "shiboken2" : "shiboken",
        "pyside2" : "pyside",
        "pyside2-tools" : "pyside-tools",
        "pyside2-examples" : "examples"
}

def usage():
    print("""\
This is a utility script for pyside-setup to prepare its source tree for testing
by the Qt Continuous Integration (CI). The script will checkout all submodules in the
pyside-setup/ sources directory except the one under test in the CI. The submodule
to be tested is expected to be found as a sibling directory of pyside-setup,
from which it is moved under the pyside-setup/sources directory.

Usage:
python prepare-sources.py --module=pyside/<submodule> --branch=<branch>

--module
    Module is one of the submodules in the pyside-setup/source directory.
    The script expects to find this module as a sibling directory of
    pyside-setup. The directory is moved under pyside-setup/sources and
    renamed as expected.

--branch
    Branch is the one that will be checked out when cloning pyside-setup's
    submodules from git.
    """)
    sys.exit(0)

def prepare_sources():
    for module_name, repo_name in submodules.items():
        module_dir = os.path.join("sources", module_name)
        if os.path.exists(module_dir) and repo_name != QT_CI_TESTED_SUBMODULE:
            sub_repo = git_server + "/pyside/" + repo_name + ".git"
            try:
                shutil.move(module_dir, module_dir + "_removed")
            except Exception as e:
                raise Exception("!!!!!!!!!!!!! Failed to rename %s " % module_dir)
            git_checkout_cmd = ["git", "clone", sub_repo, module_dir]
            if run_process(git_checkout_cmd) != 0:
                raise Exception("!!!!!!!!!!!!! Failed to clone the git submodule %s" % sub_repo)
    print("************************* CLONED **********************")
    for module_name, repo_name in submodules.items():
        print("***** Preparing %s" % module_name)
        if repo_name == QT_CI_TESTED_SUBMODULE:
            print("Skipping tested module %s and using sources from Coin storage instead" % module_name)
            module_dir = os.path.join("sources", module_name)
            storage_dir = os.path.join("..", QT_CI_TESTED_SUBMODULE)
            try:
                shutil.move(module_dir, module_dir + "_replaced_as_tested")
            except Exception as e:
                raise Exception("!!!!!!!!!!!!! Failed to rename %s " % module_dir)
            shutil.move(storage_dir, module_dir)
        else:
            module_dir = os.path.join("sources", module_name)
            os.chdir(module_dir)
            #Make sure  the branch exists, if not use dev
            _branch = SUBMODULE_BRANCH
            git_list_branch_cmd = ["git", "branch", "--list", _branch]
            shell = (sys.platform == "win32")
            result = Popen(git_list_branch_cmd, stdout=PIPE, shell=shell)
            if len(result.communicate()[0].split())==0:
                print("Warning: Requested %s branch doesn't exist so we'll fall back to 'dev' branch instead"\
                % SUBMODULE_BRANCH)
                _branch = "dev"
            print("Checking out submodule %s to branch %s" % (module_name, _branch))
            git_checkout_cmd = ["git", "checkout", _branch]
            if run_process(git_checkout_cmd) != 0:
                print("Failed to initialize the git submodule %s" % module_name)
            else:
                print("Submodule %s has branch %s checked out" % (module_name, _branch))
            os.chdir(script_dir)


if QT_CI_TESTED_SUBMODULE is not None:
    QT_CI_TESTED_SUBMODULE = QT_CI_TESTED_SUBMODULE.split('/')[1]

if SUBMODULE_BRANCH is None:
    usage()

script_dir = os.getcwd()

if __name__ == "__main__":
    prepare_sources()
