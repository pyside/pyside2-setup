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

from ..utils import (copydir, copyfile, copy_icu_libs, find_files_using_glob,
                     linux_set_rpaths, linux_run_read_elf, linux_get_rpaths,
                     rpaths_has_origin)
from ..config import config


def prepare_standalone_package_linux(self, vars):
    built_modules = vars['built_modules']

    constrain_modules = None
    copy_plugins = True
    copy_qml = True
    copy_translations = True
    copy_qt_conf = True
    should_copy_icu_libs = True

    if config.is_internal_shiboken_generator_build():
        constrain_modules = ["Core", "Network", "Xml", "XmlPatterns"]
        copy_plugins = False
        copy_qml = False
        copy_translations = False
        copy_qt_conf = False
        should_copy_icu_libs = False

    # <qt>/lib/* -> <setup>/{st_package_name}/Qt/lib
    destination_lib_dir = "{st_build_dir}/{st_package_name}/Qt/lib"

    accepted_modules = ['libQt5*.so.?']
    if constrain_modules:
        accepted_modules = ["libQt5" + module + "*.so.?" for module in constrain_modules]
    accepted_modules.append("libicu*.so.??")

    copydir("{qt_lib_dir}", destination_lib_dir,
            filter=accepted_modules,
            recursive=False, vars=vars, force_copy_symlinks=True)

    if should_copy_icu_libs:
        # Check if ICU libraries were copied over to the destination
        # Qt libdir.
        resolved_destination_lib_dir = destination_lib_dir.format(**vars)
        maybe_icu_libs = find_files_using_glob(resolved_destination_lib_dir, "libicu*")

        # If no ICU libraries are present in the Qt libdir (like when
        # Qt is built against system ICU, or in the Coin CI where ICU
        # libs are in a different directory) try to find out / resolve
        # which ICU libs are used by QtCore (if used at all) using a
        # custom written ldd, and copy the ICU libs to the Pyside Qt
        # dir if necessary. We choose the QtCore lib to inspect, by
        # checking which QtCore library the shiboken6 executable uses.
        if not maybe_icu_libs:
            copy_icu_libs(self._patchelf_path, resolved_destination_lib_dir)

    # Patching designer to use the Qt libraries provided in the wheel
    if config.is_internal_pyside_build():
        designer_path = "{st_build_dir}/{st_package_name}/designer".format(**vars)
        rpaths = linux_get_rpaths(designer_path)
        if not rpaths or not rpaths_has_origin(rpaths):
            rpaths.insert(0, '$ORIGIN/../lib')
            new_rpaths_string = ":".join(rpaths)
            linux_set_rpaths(self._patchelf_path, designer_path, new_rpaths_string)

    if self.is_webengine_built(built_modules):
        copydir("{qt_lib_execs_dir}",
                "{st_build_dir}/{st_package_name}/Qt/libexec",
                filter=None,
                recursive=False,
                vars=vars)

        copydir("{qt_prefix_dir}/resources",
                "{st_build_dir}/{st_package_name}/Qt/resources",
                filter=None,
                recursive=False,
                vars=vars)

    if copy_plugins:
        # <qt>/plugins/* -> <setup>/{st_package_name}/Qt/plugins
        copydir("{qt_plugins_dir}",
                "{st_build_dir}/{st_package_name}/Qt/plugins",
                filter=["*.so"],
                recursive=True,
                vars=vars)

    if copy_qml:
        # <qt>/qml/* -> <setup>/{st_package_name}/Qt/qml
        copydir("{qt_qml_dir}",
                "{st_build_dir}/{st_package_name}/Qt/qml",
                filter=None,
                force=False,
                recursive=True,
                ignore=["*.so.debug"],
                vars=vars)

    if copy_translations:
        # <qt>/translations/* ->
        # <setup>/{st_package_name}/Qt/translations
        copydir("{qt_translations_dir}",
                "{st_build_dir}/{st_package_name}/Qt/translations",
                filter=["*.qm", "*.pak"],
                force=False,
                vars=vars)

    if copy_qt_conf:
        # Copy the qt.conf file to libexec.
        copyfile(
            f"{{build_dir}}/{PYSIDE}/{{st_package_name}}/qt.conf",
            "{st_build_dir}/{st_package_name}/Qt/libexec",
            vars=vars)
