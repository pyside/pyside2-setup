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

import os
import sys
from .linux import prepare_standalone_package_linux
from .macos import prepare_standalone_package_macos

from ..config import config
from ..options import *
from ..utils import copydir, copyfile, rmtree, makefile
from ..utils import regenerate_qt_resources


def prepare_packages_posix(self, vars):
    executables = []

    # <install>/lib/site-packages/{st_package_name}/* ->
    # <setup>/{st_package_name}
    # This copies the module .so/.dylib files and various .py files
    # (__init__, config, git version, etc.)
    copydir(
        "{site_packages_dir}/{st_package_name}",
        "{st_build_dir}/{st_package_name}",
        vars=vars)

    generated_config = self.get_built_pyside_config(vars)

    def adjusted_lib_name(name, version):
        postfix = ''
        if sys.platform.startswith('linux'):
            postfix = '.so.' + version
        elif sys.platform == 'darwin':
            postfix = '.' + version + '.dylib'
        return name + postfix

    if config.is_internal_shiboken_module_build():
        # <build>/shiboken2/doc/html/* ->
        #   <setup>/{st_package_name}/docs/shiboken2
        copydir(
            "{build_dir}/shiboken2/doc/html",
            "{st_build_dir}/{st_package_name}/docs/shiboken2",
            force=False, vars=vars)

        # <install>/lib/lib* -> {st_package_name}/
        copydir(
            "{install_dir}/lib/",
            "{st_build_dir}/{st_package_name}",
            filter=[
                adjusted_lib_name("libshiboken*",
                                  generated_config['shiboken_library_soversion']),
            ],
            recursive=False, vars=vars, force_copy_symlinks=True)

    if config.is_internal_shiboken_generator_build():
        # <install>/bin/* -> {st_package_name}/
        executables.extend(copydir(
            "{install_dir}/bin/",
            "{st_build_dir}/{st_package_name}",
            filter=[
                "shiboken2",
            ],
            recursive=False, vars=vars))

        # Used to create scripts directory.
        makefile(
            "{st_build_dir}/{st_package_name}/scripts/shiboken_tool.py",
            vars=vars)

        # For setting up setuptools entry points.
        copyfile(
            "{install_dir}/bin/shiboken_tool.py",
            "{st_build_dir}/{st_package_name}/scripts/shiboken_tool.py",
            force=False, vars=vars)

    if config.is_internal_shiboken_generator_build() or config.is_internal_pyside_build():
        # <install>/include/* -> <setup>/{st_package_name}/include
        copydir(
            "{install_dir}/include/{cmake_package_name}",
            "{st_build_dir}/{st_package_name}/include",
            vars=vars)

    if config.is_internal_pyside_build():
        # <install>/lib/site-packages/pyside2uic/* ->
        #   <setup>/pyside2uic
        copydir(
            "{site_packages_dir}/pyside2uic",
            "{st_build_dir}/pyside2uic",
            force=False, vars=vars)
        if sys.version_info[0] > 2:
            rmtree("{st_build_dir}/pyside2uic/port_v2".format(**vars))
        else:
            rmtree("{st_build_dir}/pyside2uic/port_v3".format(**vars))

        # <install>/bin/pyside2-uic -> {st_package_name}/scripts/uic.py
        makefile(
            "{st_build_dir}/{st_package_name}/scripts/__init__.py",
            vars=vars)
        copyfile(
            "{install_dir}/bin/pyside2-uic",
            "{st_build_dir}/{st_package_name}/scripts/uic.py",
            force=False, vars=vars)

        # For setting up setuptools entry points
        copyfile(
            "{install_dir}/bin/pyside_tool.py",
            "{st_build_dir}/{st_package_name}/scripts/pyside_tool.py",
            force=False, vars=vars)

        # <install>/bin/* -> {st_package_name}/
        executables.extend(copydir(
            "{install_dir}/bin/",
            "{st_build_dir}/{st_package_name}",
            filter=[
                "pyside2-lupdate",
                "pyside2-rcc",
            ],
            recursive=False, vars=vars))

        # <install>/lib/lib* -> {st_package_name}/
        copydir(
            "{install_dir}/lib/",
            "{st_build_dir}/{st_package_name}",
            filter=[
                adjusted_lib_name("libpyside*",
                                  generated_config['pyside_library_soversion']),
            ],
            recursive=False, vars=vars, force_copy_symlinks=True)

        # <install>/share/{st_package_name}/typesystems/* ->
        #   <setup>/{st_package_name}/typesystems
        copydir(
            "{install_dir}/share/{st_package_name}/typesystems",
            "{st_build_dir}/{st_package_name}/typesystems",
            vars=vars)

        # <install>/share/{st_package_name}/glue/* ->
        #   <setup>/{st_package_name}/glue
        copydir(
            "{install_dir}/share/{st_package_name}/glue",
            "{st_build_dir}/{st_package_name}/glue",
            vars=vars)

        # <source>/pyside2/{st_package_name}/support/* ->
        #   <setup>/{st_package_name}/support/*
        copydir(
            "{build_dir}/pyside2/{st_package_name}/support",
            "{st_build_dir}/{st_package_name}/support",
            vars=vars)

        # <source>/pyside2/{st_package_name}/*.pyi ->
        #   <setup>/{st_package_name}/*.pyi
        copydir(
            "{build_dir}/pyside2/{st_package_name}",
            "{st_build_dir}/{st_package_name}",
            filter=["*.pyi"],
            vars=vars)

        if not OPTION_NOEXAMPLES:
            # examples/* -> <setup>/{st_package_name}/examples
            copydir(os.path.join(self.script_dir, "examples"),
                    "{st_build_dir}/{st_package_name}/examples",
                    force=False, vars=vars)
            # Re-generate examples Qt resource files for Python 3
            # compatibility
            if sys.version_info[0] == 3:
                examples_path = "{st_build_dir}/{st_package_name}/examples".format(
                    **vars)
                pyside_rcc_path = "{install_dir}/bin/pyside2-rcc".format(
                    **vars)
                pyside_rcc_options = '-py3'
                regenerate_qt_resources(examples_path, pyside_rcc_path,
                    pyside_rcc_options)

    # Copy Qt libs to package
    if OPTION_STANDALONE:
        if config.is_internal_pyside_build() or config.is_internal_shiboken_generator_build():
            vars['built_modules'] = generated_config['built_modules']
            if sys.platform == 'darwin':
                prepare_standalone_package_macos(self, vars)
            else:
                prepare_standalone_package_linux(self, vars)

        if config.is_internal_shiboken_generator_build():
            # Copy over clang before rpath patching.
            self.prepare_standalone_clang(is_win=False)

    # Update rpath to $ORIGIN
    if sys.platform.startswith('linux') or sys.platform.startswith('darwin'):
        rpath_path = "{st_build_dir}/{st_package_name}".format(**vars)
        self.update_rpath(rpath_path, executables)
