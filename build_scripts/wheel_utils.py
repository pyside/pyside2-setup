#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

import os
import time

from distutils.errors import DistutilsSetupError
from distutils.sysconfig import get_config_var
from distutils.util import get_platform
from distutils.version import LooseVersion

from .options import OPTION
from .qtinfo import QtInfo
from .utils import memoize, get_python_dict
from .versions import PYSIDE


@memoize
def get_package_timestamp():
    """ In a Coin CI build the returned timestamp will be the
        Coin integration id timestamp. For regular builds it's
        just the current timestamp or a user provided one."""
    option_value = OPTION["PACKAGE_TIMESTAMP"]
    return option_value if option_value else int(time.time())


def get_qt_version():
    qtinfo = QtInfo()
    qt_version = qtinfo.version

    if not qt_version:
        m = "Failed to query the Qt version with qmake {0}".format(qtinfo.qmake_command)
        raise DistutilsSetupError(m)

    if LooseVersion(qtinfo.version) < LooseVersion("5.7"):
        m = "Incompatible Qt version detected: {}. A Qt version >= 5.7 is required.".format(qt_version)
        raise DistutilsSetupError(m)

    return qt_version


@memoize
def get_package_version():
    """ Returns the version string for the PySide6 package. """
    setup_script_dir = os.getcwd()
    pyside_version_py = os.path.join(
        setup_script_dir, "sources", PYSIDE, "pyside_version.py")
    d = get_python_dict(pyside_version_py)

    final_version = "{}.{}.{}".format(
        d['major_version'], d['minor_version'], d['patch_version'])
    release_version_type = d['release_version_type']
    pre_release_version = d['pre_release_version']
    if pre_release_version and release_version_type:
        final_version += release_version_type + pre_release_version
    if release_version_type.startswith("comm"):
        final_version += "." + release_version_type

    # Add the current timestamp to the version number, to suggest it
    # is a development snapshot build.
    if OPTION["SNAPSHOT_BUILD"]:
        final_version += ".dev{}".format(get_package_timestamp())
    return final_version


def macos_qt_min_deployment_target():
    target = QtInfo().macos_min_deployment_target

    if not target:
        raise DistutilsSetupError("Failed to query for Qt's QMAKE_MACOSX_DEPLOYMENT_TARGET.")
    return target


@memoize
def macos_pyside_min_deployment_target():
    """
    Compute and validate PySide6 MACOSX_DEPLOYMENT_TARGET value.
    Candidate sources that are considered:
        - setup.py provided value
        - maximum value between minimum deployment target of the
          Python interpreter and the minimum deployment target of
          the Qt libraries.
    If setup.py value is provided, that takes precedence.
    Otherwise use the maximum of the above mentioned two values.
    """
    python_target = get_config_var('MACOSX_DEPLOYMENT_TARGET') or None
    qt_target = macos_qt_min_deployment_target()
    setup_target = OPTION["MACOS_DEPLOYMENT_TARGET"]

    qt_target_split = [int(x) for x in qt_target.split('.')]
    if python_target:
        python_target_split = [int(x) for x in python_target.split('.')]
    if setup_target:
        setup_target_split = [int(x) for x in setup_target.split('.')]

    message = ("Can't set MACOSX_DEPLOYMENT_TARGET value to {} because "
               "{} was built with minimum deployment target set to {}.")
    # setup.py provided OPTION["MACOS_DEPLOYMENT_TARGET"] value takes
    # precedence.
    if setup_target:
        if python_target and setup_target_split < python_target_split:
            raise DistutilsSetupError(message.format(setup_target, "Python",
                                                     python_target))
        if setup_target_split < qt_target_split:
            raise DistutilsSetupError(message.format(setup_target, "Qt",
                                                     qt_target))
        # All checks clear, use setup.py provided value.
        return setup_target

    # Setup.py value not provided,
    # use same value as provided by Qt.
    if python_target:
        maximum_target = '.'.join([str(e) for e in max(python_target_split, qt_target_split)])
    else:
        maximum_target = qt_target
    return maximum_target


@memoize
def macos_plat_name():
    deployment_target = macos_pyside_min_deployment_target()
    # Example triple "macosx-10.12-x86_64".
    plat = get_platform().split("-")
    plat_name = "{}-{}-{}".format(plat[0], deployment_target, plat[2])
    return plat_name
