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

import os, re, sys
from .linux import prepare_standalone_package_linux
from .macos import prepare_standalone_package_macos
from ..options import *
from ..utils import copydir, copyfile, rmtree, makefile
from ..utils import regenerate_qt_resources

def prepare_packages_posix(self, vars):
    executables = []
    # <build>/shiboken2/doc/html/* ->
    #   <setup>/PySide2/docs/shiboken2
    copydir(
        "{build_dir}/shiboken2/doc/html",
        "{pyside_package_dir}/PySide2/docs/shiboken2",
        force=False, vars=vars)
    # <install>/lib/site-packages/PySide2/* -> <setup>/PySide2
    copydir(
        "{site_packages_dir}/PySide2",
        "{pyside_package_dir}/PySide2",
        vars=vars)
    # <install>/lib/site-packages/shiboken2.so ->
    #   <setup>/PySide2/shiboken2.so
    shiboken_module_name = 'shiboken2.so'
    shiboken_src_path = "{site_packages_dir}".format(**vars)
    maybe_shiboken_names = [f for f in os.listdir(shiboken_src_path)
                            if re.match(r'shiboken.*\.so', f)]
    if maybe_shiboken_names:
        shiboken_module_name = maybe_shiboken_names[0]
    vars.update({'shiboken_module_name': shiboken_module_name})
    copyfile(
        "{site_packages_dir}/{shiboken_module_name}",
        "{pyside_package_dir}/PySide2/{shiboken_module_name}",
        vars=vars)
    # <install>/lib/site-packages/pyside2uic/* ->
    #   <setup>/pyside2uic
    copydir(
        "{site_packages_dir}/pyside2uic",
        "{pyside_package_dir}/pyside2uic",
        force=False, vars=vars)
    if sys.version_info[0] > 2:
        rmtree("{pyside_package_dir}/pyside2uic/port_v2".format(**vars))
    else:
        rmtree("{pyside_package_dir}/pyside2uic/port_v3".format(**vars))
    # <install>/bin/pyside2-uic -> PySide2/scripts/uic.py
    makefile(
        "{pyside_package_dir}/PySide2/scripts/__init__.py",
        vars=vars)
    copyfile(
        "{install_dir}/bin/pyside2-uic",
        "{pyside_package_dir}/PySide2/scripts/uic.py",
        force=False, vars=vars)
    # <install>/bin/* -> PySide2/
    executables.extend(copydir(
        "{install_dir}/bin/",
        "{pyside_package_dir}/PySide2",
        filter=[
            "pyside2-lupdate",
            "pyside2-rcc",
            "shiboken2",
        ],
        recursive=False, vars=vars))
    # <install>/lib/lib* -> PySide2/
    config = self.get_built_pyside_config(vars)
    def adjusted_lib_name(name, version):
        postfix = ''
        if sys.platform.startswith('linux'):
            postfix = '.so.' + version
        elif sys.platform == 'darwin':
            postfix = '.' + version + '.dylib'
        return name + postfix
    copydir(
        "{install_dir}/lib/",
        "{pyside_package_dir}/PySide2",
        filter=[
            adjusted_lib_name("libpyside*",
                config['pyside_library_soversion']),
            adjusted_lib_name("libshiboken*",
                config['shiboken_library_soversion']),
        ],
        recursive=False, vars=vars, force_copy_symlinks=True)
    # <install>/share/PySide2/typesystems/* ->
    #   <setup>/PySide2/typesystems
    copydir(
        "{install_dir}/share/PySide2/typesystems",
        "{pyside_package_dir}/PySide2/typesystems",
        vars=vars)
    # <install>/include/* -> <setup>/PySide2/include
    copydir(
        "{install_dir}/include",
        "{pyside_package_dir}/PySide2/include",
        vars=vars)
    # <source>/pyside2/PySide2/support/* ->
    #   <setup>/PySide2/support/*
    copydir(
        "{build_dir}/pyside2/PySide2/support",
        "{pyside_package_dir}/PySide2/support",
        vars=vars)
    if not OPTION_NOEXAMPLES:
        # examples/* -> <setup>/PySide2/examples
        copydir(os.path.join(self.script_dir, "examples"),
                "{pyside_package_dir}/PySide2/examples",
                force=False, vars=vars)
        # Re-generate examples Qt resource files for Python 3
        # compatibility
        if sys.version_info[0] == 3:
            examples_path = "{pyside_package_dir}/PySide2/examples".format(
                **vars)
            pyside_rcc_path = "{install_dir}/bin/pyside2-rcc".format(
                **vars)
            pyside_rcc_options = '-py3'
            regenerate_qt_resources(examples_path, pyside_rcc_path,
                pyside_rcc_options)
    # Copy Qt libs to package
    if OPTION_STANDALONE:
        vars['built_modules'] = config['built_modules']
        if sys.platform == 'darwin':
            prepare_standalone_package_macos(self, executables, vars)
        else:
            prepare_standalone_package_linux(self, executables, vars)

        # Copy over clang before rpath patching.
        self.prepare_standalone_clang(is_win=False)

    # Update rpath to $ORIGIN
    if (sys.platform.startswith('linux') or
            sys.platform.startswith('darwin')):
        self.update_rpath("{pyside_package_dir}/PySide2".format(**vars),
            executables)
