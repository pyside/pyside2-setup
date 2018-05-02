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

from ..options import *
from ..utils import copydir, copyfile, copy_icu_libs, find_files_using_glob

def prepare_standalone_package_linux(self, executables, vars):
    built_modules = vars['built_modules']

    # <qt>/lib/* -> <setup>/PySide2/Qt/lib
    destination_lib_dir = "{pyside_package_dir}/PySide2/Qt/lib"
    copydir("{qt_lib_dir}", destination_lib_dir,
        filter=[
            "libQt5*.so.?",
            "libicu*.so.??",
        ],
        recursive=False, vars=vars, force_copy_symlinks=True)

    # Check if ICU libraries were copied over to the destination
    # Qt libdir.
    resolved_destination_lib_dir = destination_lib_dir.format(**vars)
    maybe_icu_libs = find_files_using_glob(resolved_destination_lib_dir,
        "libicu*")

    # If no ICU libraries are present in the Qt libdir (like when
    # Qt is built against system ICU, or in the Coin CI where ICU
    # libs are in a different directory) try to find out / resolve
    # which ICU libs are used by QtCore (if used at all) using a
    # custom written ldd, and copy the ICU libs to the Pyside Qt
    # dir if necessary. We choose the QtCore lib to inspect, by
    # checking which QtCore library the shiboken2 executable uses.
    if not maybe_icu_libs:
        copy_icu_libs(resolved_destination_lib_dir)

    if self.is_webengine_built(built_modules):
        copydir("{qt_lib_execs_dir}",
            "{pyside_package_dir}/PySide2/Qt/libexec",
            filter=None,
            recursive=False,
            vars=vars)

        copydir("{qt_prefix_dir}/resources",
            "{pyside_package_dir}/PySide2/Qt/resources",
            filter=None,
            recursive=False,
            vars=vars)

    # <qt>/plugins/* -> <setup>/PySide2/Qt/plugins
    copydir("{qt_plugins_dir}",
        "{pyside_package_dir}/PySide2/Qt/plugins",
        filter=["*.so"],
        recursive=True,
        vars=vars)

    # <qt>/qml/* -> <setup>/PySide2/Qt/qml
    copydir("{qt_qml_dir}",
        "{pyside_package_dir}/PySide2/Qt/qml",
        filter=None,
        force=False,
        recursive=True,
        vars=vars)

    # <qt>/translations/* -> <setup>/PySide2/Qt/translations

    copydir("{qt_translations_dir}",
        "{pyside_package_dir}/PySide2/Qt/translations",
        filter=["*.qm", "*.pak"],
        force=False,
        vars=vars)

    # Copy the qt.conf file to libexec.
    copyfile(
        "{build_dir}/pyside2/PySide2/qt.conf",
        "{pyside_package_dir}/PySide2/Qt/libexec",
        vars=vars)
