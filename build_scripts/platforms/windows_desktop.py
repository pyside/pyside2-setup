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

import functools
import os, re, sys
from ..options import *
from ..utils import copydir, copyfile, rmtree, makefile
from ..utils import regenerate_qt_resources, filter_match
def prepare_packages_win32(self, vars):
    # For now, debug symbols will not be shipped into the package.
    copy_pdbs = False
    pdbs = []
    if (self.debug or self.build_type == 'RelWithDebInfo') and copy_pdbs:
        pdbs = ['*.pdb']
    # <install>/lib/site-packages/PySide2/* -> <setup>/PySide2
    copydir(
        "{site_packages_dir}/PySide2",
        "{pyside_package_dir}/PySide2",
        vars=vars)
    built_modules = self.get_built_pyside_config(vars)['built_modules']

    # <build>/pyside2/PySide2/*.pdb -> <setup>/PySide2
    copydir(
        "{build_dir}/pyside2/PySide2",
        "{pyside_package_dir}/PySide2",
        filter=pdbs,
        recursive=False, vars=vars)

    # <build>/shiboken2/doc/html/* ->
    #   <setup>/PySide2/docs/shiboken2
    copydir(
        "{build_dir}/shiboken2/doc/html",
        "{pyside_package_dir}/PySide2/docs/shiboken2",
        force=False, vars=vars)

    # <install>/lib/site-packages/shiboken2.pyd ->
    #   <setup>/PySide2/shiboken2.pyd
    shiboken_module_name = 'shiboken2.pyd'
    shiboken_src_path = "{site_packages_dir}".format(**vars)
    maybe_shiboken_names = [f for f in os.listdir(shiboken_src_path)
                            if re.match(r'shiboken.*\.pyd', f)]
    if maybe_shiboken_names:
        shiboken_module_name = maybe_shiboken_names[0]
    vars.update({'shiboken_module_name': shiboken_module_name})
    copyfile(
        "{site_packages_dir}/{shiboken_module_name}",
        "{pyside_package_dir}/PySide2/{shiboken_module_name}",
        vars=vars)
    # @TODO: Fix this .pdb file not to overwrite release
    # {shibokengenerator}.pdb file.
    # Task-number: PYSIDE-615
    copydir(
        "{build_dir}/shiboken2/shibokenmodule",
        "{pyside_package_dir}/PySide2",
        filter=pdbs,
        recursive=False, vars=vars)

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

    # <install>/bin/*.exe,*.dll,*.pdb -> PySide2/
    copydir(
        "{install_dir}/bin/",
        "{pyside_package_dir}/PySide2",
        filter=["*.exe", "*.dll"],
        recursive=False, vars=vars)
    # @TODO: Fix this .pdb file not to overwrite release
    # {shibokenmodule}.pdb file.
    # Task-number: PYSIDE-615
    copydir(
        "{build_dir}/shiboken2/generator",
        "{pyside_package_dir}/PySide2",
        filter=pdbs,
        recursive=False, vars=vars)

    # <install>/lib/*.lib -> PySide2/
    copydir(
        "{install_dir}/lib/",
        "{pyside_package_dir}/PySide2",
        filter=["*.lib"],
        recursive=False, vars=vars)

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

    # <ssl_libs>/* -> <setup>/PySide2/openssl
    copydir("{ssl_libs_dir}", "{pyside_package_dir}/PySide2/openssl",
        filter=[
            "libeay32.dll",
            "ssleay32.dll"],
        force=False, vars=vars)

    # <qt>/bin/*.dll and Qt *.exe -> <setup>/PySide2
    qt_artifacts_permanent = [
        "opengl*.dll",
        "d3d*.dll",
        "libEGL*.dll",
        "libGLESv2*.dll",
        "designer.exe",
        "linguist.exe",
        "lrelease.exe",
        "lupdate.exe",
        "lconvert.exe",
        "qtdiag.exe"
    ]
    copydir("{qt_bin_dir}", "{pyside_package_dir}/PySide2",
        filter=qt_artifacts_permanent,
        recursive=False, vars=vars)

    # <qt>/bin/*.dll and Qt *.pdbs -> <setup>/PySide2 part two
    # File filter to copy only debug or only release files.
    qt_dll_patterns = ["Qt5*{}.dll", "lib*{}.dll"]
    if copy_pdbs:
        qt_dll_patterns += ["Qt5*{}.pdb", "lib*{}.pdb"]
    def qt_build_config_filter(patterns, file_name, file_full_path):
        release = [a.format('') for a in patterns]
        debug = [a.format('d') for a in patterns]

        # If qt is not a debug_and_release build, that means there
        # is only one set of shared libraries, so we can just copy
        # them.
        if self.qtinfo.build_type != 'debug_and_release':
            if filter_match(file_name, release):
                return True
            return False

        # In debug_and_release case, choosing which files to copy
        # is more difficult. We want to copy only the files that
        # match the PySide2 build type. So if PySide2 is built in
        # debug mode, we want to copy only Qt debug libraries
        # (ending with "d.dll"). Or vice versa. The problem is that
        # some libraries have "d" as the last character of the
        # actual library name (for example Qt5Gamepad.dll and
        # Qt5Gamepadd.dll). So we can't just match a pattern ending
        # in "d". Instead we check if there exists a file with the
        # same name plus an additional "d" at the end, and using
        # that information we can judge if the currently processed
        # file is a debug or release file.

        # e.g. ["Qt5Cored", ".dll"]
        file_split = os.path.splitext(file_name)
        file_base_name = file_split[0]
        file_ext = file_split[1]
        # e.g. "/home/work/qt/qtbase/bin"
        file_path_dir_name = os.path.dirname(file_full_path)
        # e.g. "Qt5Coredd"
        maybe_debug_name = file_base_name + 'd'
        if self.debug:
            filter = debug
            def predicate(path): return not os.path.exists(path)
        else:
            filter = release
            def predicate(path): return os.path.exists(path)
        # e.g. "/home/work/qt/qtbase/bin/Qt5Coredd.dll"
        other_config_path = os.path.join(file_path_dir_name,
            maybe_debug_name + file_ext)

        if (filter_match(file_name, filter) and
                predicate(other_config_path)):
            return True
        return False

    qt_dll_filter = functools.partial(qt_build_config_filter,
        qt_dll_patterns)
    copydir("{qt_bin_dir}", "{pyside_package_dir}/PySide2",
        file_filter_function=qt_dll_filter,
        recursive=False, vars=vars)

    # <qt>/plugins/* -> <setup>/PySide2/plugins
    plugin_dll_patterns = ["*{}.dll"]
    pdb_pattern = "*{}.pdb"
    if copy_pdbs:
        plugin_dll_patterns += [pdb_pattern]
    plugin_dll_filter = functools.partial(qt_build_config_filter,
        plugin_dll_patterns)
    copydir("{qt_plugins_dir}", "{pyside_package_dir}/PySide2/plugins",
        file_filter_function=plugin_dll_filter,
        vars=vars)

    # <qt>/translations/* -> <setup>/PySide2/translations
    copydir("{qt_translations_dir}",
        "{pyside_package_dir}/PySide2/translations",
        filter=["*.qm", "*.pak"],
        force=False,
        vars=vars)

    # <qt>/qml/* -> <setup>/PySide2/qml
    qml_dll_patterns = ["*{}.dll"]
    qml_ignore_patterns = qml_dll_patterns + [pdb_pattern]
    # Remove the "{}" from the patterns
    qml_ignore = [a.format('') for a in qml_ignore_patterns]
    if copy_pdbs:
        qml_dll_patterns += [pdb_pattern]
    qml_ignore = [a.format('') for a in qml_dll_patterns]
    qml_dll_filter = functools.partial(qt_build_config_filter,
        qml_dll_patterns)
    copydir("{qt_qml_dir}", "{pyside_package_dir}/PySide2/qml",
        ignore=qml_ignore,
        force=False,
        recursive=True,
        vars=vars)
    copydir("{qt_qml_dir}", "{pyside_package_dir}/PySide2/qml",
        file_filter_function=qml_dll_filter,
        force=False,
        recursive=True,
        vars=vars)

    if self.is_webengine_built(built_modules):
        copydir("{qt_prefix_dir}/resources",
            "{pyside_package_dir}/PySide2/resources",
            filter=None,
            recursive=False,
            vars=vars)

        filter = 'QtWebEngineProcess{}.exe'.format(
            'd' if self.debug else '')
        copydir("{qt_bin_dir}",
            "{pyside_package_dir}/PySide2",
            filter=[filter],
            recursive=False, vars=vars)

    # Copy the qt.conf file to prefix dir.
    copyfile(
        "{build_dir}/pyside2/PySide2/qt.conf",
        "{pyside_package_dir}/PySide2",
        vars=vars)

    self.prepare_standalone_clang(is_win=True)

    # pdb files for libshiboken and libpyside
    copydir(
        "{build_dir}/shiboken2/libshiboken",
        "{pyside_package_dir}/PySide2",
        filter=pdbs,
        recursive=False, vars=vars)
    copydir(
        "{build_dir}/pyside2/libpyside",
        "{pyside_package_dir}/PySide2",
        filter=pdbs,
        recursive=False, vars=vars)
