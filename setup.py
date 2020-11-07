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

"""
This is a distutils setup-script for the Qt for Python project.
For more information see README.md
"""

import os
import sys

# Change the current directory to setup.py's dir.
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file_original = this_file
this_file = os.path.abspath(this_file)
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))

# Save the original command line arguments to pass them on to the setup
# mechanism.
original_argv = list(sys.argv)

# If setup.py was invoked via -c "some code" or -m some_command, make sure
# to replace the first argv to be the script name, so that sub-invocations
# continue to work.
if original_argv and original_argv[0] in ['-c', '-m']:
    original_argv[0] = this_file_original

from build_scripts.main import get_package_version, check_allowed_python_version
from build_scripts.setup_runner import SetupRunner

# The __version__ variable is just for PEP compliance, and shouldn't be
# used as a value source. Use get_package_version() instead.
__version__ = get_package_version()

check_allowed_python_version()

setup_runner = SetupRunner(original_argv)
setup_runner.run_setup()
