#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

from __future__ import print_function, absolute_import

"""
Supporting isolation of warnings

Warnings in unittests are not isolated.
We sometimes use warnings to conveniently accumulate error messages
and eventually report them afterwards as error.
"""

import sys
import os
import warnings
import re
from contextlib import contextmanager

warn_name = "__warningregistry__"
ignore_re = 'Not importing directory .*'

@contextmanager
def isolate_warnings():
    save_warnings = {}
    for name, mod in sys.modules.items():
        if mod and hasattr(mod, warn_name):
            save_warnings[name] = mod.__dict__[warn_name]
            delattr(mod, warn_name)
        else:
            save_warnings[name] = None
    yield
    for name, warn in save_warnings.items():
        mod = sys.modules[name]
        if mod:
            setattr(mod, warn_name, warn)
            if warn is None:
                delattr(mod, warn_name)

@contextmanager
def suppress_warnings():
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        yield

def check_warnings():
    for name, mod in sys.modules.items():
        if mod:
            reg = getattr(mod, warn_name, None)
            if reg:
                # XXX We need to filter warnings for Python 2.
                # This should be avoided by renaming the duplicate folders.
                for k in reg:
                    if type(k) is tuple and re.match(ignore_re, k[0]):
                        continue
                    return True
    return False

def warn(message, category=None, stacklevel=2):
    """Issue a warning with the default 'RuntimeWarning'"""
    if category is None:
        category = UserWarning
    warnings.warn(message, category, stacklevel)


def linux_distribution():
    """Returns the Linux distribution"""
    # We have to be more specific because we had differences between
    # RHEL 6.6 and RHEL 7.4 .
    # Note: The platform module is deprecated. We need to switch to the
    # distro package, ASAP! The distro has been extracted from Python,
    # because it changes more often than the Python version.
    distribution = []
    try:
        import distro
        distribution = distro.linux_distribution()
    except ImportError:
        # platform.linux_distribution() was removed in 3.8
        if sys.version_info[:2] < (3, 8):
            import platform
            distribution = platform.linux_distribution()
    if distribution:
        return "".join(distribution[:2]).lower()
    warnings.warn('Cannot determine Linux distribution, please install distro',
                  UserWarning)
    return ""


# Python2 legacy: Correct 'linux2' to 'linux', recommended way.
if sys.platform.startswith('linux'):
    platform_name = linux_distribution()
    # this currently happens on opensuse in 5.14:
    if not platform_name:
        # We intentionally crash when that last resort is also absent:
        platform_name = os.environ["MACHTYPE"]
    platform_name = re.sub('[^0-9a-z]', '_', platform_name)
else:
    platform_name = sys.platform
# In the linux case, we need more information.

is_py3 = sys.version_info[0] == 3
is_ci = os.environ.get("QTEST_ENVIRONMENT", "") == "ci"

def get_script_dir():
    script_dir = os.path.normpath(os.path.dirname(__file__))
    while "sources" not in os.listdir(script_dir):
        script_dir = os.path.dirname(script_dir)
    return script_dir

def qt_version():
    from PySide6.QtCore import __version__
    return tuple(map(int, __version__.split(".")))

# Format a registry file name for version.
def _registry_filename(version, use_ci_module):
    name = "exists_{}_{}_{}_{}{}.py".format(platform_name,
        version[0], version[1], version[2], "_ci" if use_ci_module else "")
    return os.path.join(os.path.dirname(__file__), name)

# Return the expected registry file name.
def get_refpath(use_ci_module=is_ci):
    return _registry_filename(qt_version(), use_ci_module)

# Return the registry file name, either that of the current
# version or fall back to a previous patch release.
def get_effective_refpath(use_ci_module=is_ci):
    refpath = get_refpath(use_ci_module)
    if os.path.exists(refpath):
        return refpath
    version = qt_version()
    major, minor, patch = version[:3]
    while patch >= 0:
        file = _registry_filename((major, minor, patch), use_ci_module)
        if os.path.exists(file):
            return file
        patch -= 1
    return refpath

# Import the CI version of the platform module
def import_refmodule(use_ci_module=is_ci):
    refpath = get_effective_refpath(use_ci_module)
    modname = os.path.basename(os.path.splitext(refpath)[0])
    return __import__(modname)

# eof
