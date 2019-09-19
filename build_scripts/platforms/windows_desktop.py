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
import os
import sys
import fnmatch

from ..config import config
from ..options import *
from ..utils import copydir, copyfile, rmtree, makefile
from ..utils import regenerate_qt_resources, filter_match
from ..utils import download_and_extract_7z


def prepare_packages_win32(self, vars):
    # For now, debug symbols will not be shipped into the package.
    copy_pdbs = False
    pdbs = []
    if (self.debug or self.build_type == 'RelWithDebInfo') and copy_pdbs:
        pdbs = ['*.pdb']

    # <install>/lib/site-packages/{st_package_name}/* ->
    # <setup>/{st_package_name}
    # This copies the module .pyd files and various .py files
    # (__init__, config, git version, etc.)
    copydir(
        "{site_packages_dir}/{st_package_name}",
        "{st_build_dir}/{st_package_name}",
        vars=vars)

    if config.is_internal_shiboken_module_build():
        # <build>/shiboken2/doc/html/* ->
        #   <setup>/{st_package_name}/docs/shiboken2
        copydir(
            "{build_dir}/shiboken2/doc/html",
            "{st_build_dir}/{st_package_name}/docs/shiboken2",
            force=False, vars=vars)

        # <install>/bin/*.dll -> {st_package_name}/
        copydir(
            "{install_dir}/bin/",
            "{st_build_dir}/{st_package_name}",
            filter=["shiboken*.dll"],
            recursive=False, vars=vars)

        # <install>/lib/*.lib -> {st_package_name}/
        copydir(
            "{install_dir}/lib/",
            "{st_build_dir}/{st_package_name}",
            filter=["shiboken*.lib"],
            recursive=False, vars=vars)

        # @TODO: Fix this .pdb file not to overwrite release
        # {shibokengenerator}.pdb file.
        # Task-number: PYSIDE-615
        copydir(
            "{build_dir}/shiboken2/shibokenmodule",
            "{st_build_dir}/{st_package_name}",
            filter=pdbs,
            recursive=False, vars=vars)

        # pdb files for libshiboken and libpyside
        copydir(
            "{build_dir}/shiboken2/libshiboken",
            "{st_build_dir}/{st_package_name}",
            filter=pdbs,
            recursive=False, vars=vars)

    if config.is_internal_shiboken_generator_build():
        # <install>/bin/*.dll -> {st_package_name}/
        copydir(
            "{install_dir}/bin/",
            "{st_build_dir}/{st_package_name}",
            filter=["shiboken*.exe"],
            recursive=False, vars=vars)

        # Used to create scripts directory.
        makefile(
            "{st_build_dir}/{st_package_name}/scripts/shiboken_tool.py",
            vars=vars)

        # For setting up setuptools entry points.
        copyfile(
            "{install_dir}/bin/shiboken_tool.py",
            "{st_build_dir}/{st_package_name}/scripts/shiboken_tool.py",
            force=False, vars=vars)

        # @TODO: Fix this .pdb file not to overwrite release
        # {shibokenmodule}.pdb file.
        # Task-number: PYSIDE-615
        copydir(
            "{build_dir}/shiboken2/generator",
            "{st_build_dir}/{st_package_name}",
            filter=pdbs,
            recursive=False, vars=vars)

    if config.is_internal_shiboken_generator_build() or config.is_internal_pyside_build():
        # <install>/include/* -> <setup>/{st_package_name}/include
        copydir(
            "{install_dir}/include/{cmake_package_name}",
            "{st_build_dir}/{st_package_name}/include",
            vars=vars)

    if config.is_internal_pyside_build():
        # <build>/pyside2/{st_package_name}/*.pdb ->
        # <setup>/{st_package_name}
        copydir(
            "{build_dir}/pyside2/{st_package_name}",
            "{st_build_dir}/{st_package_name}",
            filter=pdbs,
            recursive=False, vars=vars)

        makefile(
            "{st_build_dir}/{st_package_name}/scripts/__init__.py",
            vars=vars)

        # For setting up setuptools entry points
        copyfile(
            "{install_dir}/bin/pyside_tool.py",
            "{st_build_dir}/{st_package_name}/scripts/pyside_tool.py",
            force=False, vars=vars)

        # <install>/bin/*.exe,*.dll -> {st_package_name}/
        copydir(
            "{install_dir}/bin/",
            "{st_build_dir}/{st_package_name}",
            filter=["pyside*.exe", "pyside*.dll", "uic.exe", "rcc.exe"],
            recursive=False, vars=vars)

        # <install>/lib/*.lib -> {st_package_name}/
        copydir(
            "{install_dir}/lib/",
            "{st_build_dir}/{st_package_name}",
            filter=["pyside*.lib"],
            recursive=False, vars=vars)

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

        copydir(
            "{build_dir}/pyside2/libpyside",
            "{st_build_dir}/{st_package_name}",
            filter=pdbs,
            recursive=False, vars=vars)

        if not OPTION_NOEXAMPLES:
            def pycache_dir_filter(dir_name, parent_full_path, dir_full_path):
                if fnmatch.fnmatch(dir_name, "__pycache__"):
                    return False
                return True
            # examples/* -> <setup>/{st_package_name}/examples
            copydir(os.path.join(self.script_dir, "examples"),
                    "{st_build_dir}/{st_package_name}/examples",
                    force=False, vars=vars, dir_filter_function=pycache_dir_filter)
            # Re-generate examples Qt resource files for Python 3
            # compatibility
            if sys.version_info[0] == 3:
                examples_path = "{st_build_dir}/{st_package_name}/examples".format(
                    **vars)
                pyside_rcc_path = "{install_dir}/bin/rcc.exe".format(
                    **vars)
                pyside_rcc_options = '-g python'
                regenerate_qt_resources(examples_path, pyside_rcc_path,
                    pyside_rcc_options)

        if vars['ssl_libs_dir']:
            # <ssl_libs>/* -> <setup>/{st_package_name}/openssl
            copydir("{ssl_libs_dir}", "{st_build_dir}/{st_package_name}/openssl",
                filter=[
                    "libeay32.dll",
                    "ssleay32.dll"],
                force=False, vars=vars)

    if config.is_internal_shiboken_module_build():
        # The C++ std library dlls need to be packaged with the
        # shiboken module, because libshiboken uses C++ code.
        copy_msvc_redist_files(vars, "{build_dir}/msvc_redist".format(**vars))

    if config.is_internal_pyside_build() or config.is_internal_shiboken_generator_build():
        copy_qt_artifacts(self, copy_pdbs, vars)


def copy_msvc_redist_files(vars, redist_target_path):
    # MSVC redistributable file list.
    msvc_redist = [
        "concrt140.dll",
        "msvcp140.dll",
        "ucrtbase.dll",
        "vcamp140.dll",
        "vccorlib140.dll",
        "vcomp140.dll",
        "vcruntime140.dll"
    ]

    # Make a directory where the files should be extracted.
    if not os.path.exists(redist_target_path):
        os.makedirs(redist_target_path)

    # Extract Qt dependency dlls when building on Qt CI.
    in_coin = os.environ.get('COIN_LAUNCH_PARAMETERS', None)
    if in_coin is not None:
        redist_url = "http://download.qt.io/development_releases/prebuilt/vcredist/"
        zip_file = "pyside_qt_deps_64.7z"
        if "{target_arch}".format(**vars) == "32":
            zip_file = "pyside_qt_deps_32.7z"
        download_and_extract_7z(redist_url + zip_file, redist_target_path)
    else:
        print("Qt dependency DLLs (MSVC redist) will not be downloaded and extracted.")

    copydir(redist_target_path,
            "{st_build_dir}/{st_package_name}",
            filter=msvc_redist, recursive=False, vars=vars)


def copy_qt_artifacts(self, copy_pdbs, vars):
    built_modules = self.get_built_pyside_config(vars)['built_modules']

    constrain_modules = None
    copy_plugins = True
    copy_qml = True
    copy_translations = True
    copy_qt_conf = True
    copy_qt_permanent_artifacts = True
    copy_msvc_redist = False
    copy_clang = False

    if config.is_internal_shiboken_generator_build():
        constrain_modules = ["Core", "Network", "Xml", "XmlPatterns"]
        copy_plugins = False
        copy_qml = False
        copy_translations = False
        copy_qt_conf = False
        copy_qt_permanent_artifacts = False
        copy_msvc_redist = True
        copy_clang = True

    # <qt>/bin/*.dll and Qt *.exe -> <setup>/{st_package_name}
    qt_artifacts_permanent = [
        "opengl*.dll",
        "d3d*.dll",
        "designer.exe",
        "linguist.exe",
        "lrelease.exe",
        "lupdate.exe",
        "lconvert.exe",
        "qtdiag.exe"
    ]

    # Choose which EGL library variants to copy.
    qt_artifacts_egl = [
        "libEGL{}.dll",
        "libGLESv2{}.dll"
    ]
    if self.qtinfo.build_type != 'debug_and_release':
        egl_suffix = '*'
    elif self.debug:
        egl_suffix = 'd'
    else:
        egl_suffix = ''
    qt_artifacts_egl = [a.format(egl_suffix) for a in qt_artifacts_egl]

    artifacts = []
    if copy_qt_permanent_artifacts:
        artifacts += qt_artifacts_permanent
        artifacts += qt_artifacts_egl

    if copy_msvc_redist:
        # The target path has to be qt_bin_dir at the moment,
        # because the extracted archive also contains the opengl32sw
        # and the d3dcompiler dlls, which are copied not by this
        # function, but by the copydir below.
        copy_msvc_redist_files(vars, "{qt_bin_dir}".format(**vars))

    if artifacts:
        copydir("{qt_bin_dir}",
                "{st_build_dir}/{st_package_name}",
                filter=artifacts, recursive=False, vars=vars)

    # <qt>/bin/*.dll and Qt *.pdbs -> <setup>/{st_package_name} part two
    # File filter to copy only debug or only release files.
    if constrain_modules:
        qt_dll_patterns = ["Qt5" + x + "{}.dll" for x in constrain_modules]
        if copy_pdbs:
            qt_dll_patterns += ["Qt5" + x + "{}.pdb" for x in constrain_modules]
    else:
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
    copydir("{qt_bin_dir}",
            "{st_build_dir}/{st_package_name}",
            file_filter_function=qt_dll_filter,
            recursive=False, vars=vars)

    if copy_plugins:
        # <qt>/plugins/* -> <setup>/{st_package_name}/plugins
        plugin_dll_patterns = ["*{}.dll"]
        pdb_pattern = "*{}.pdb"
        if copy_pdbs:
            plugin_dll_patterns += [pdb_pattern]
        plugin_dll_filter = functools.partial(qt_build_config_filter,
            plugin_dll_patterns)
        copydir("{qt_plugins_dir}", "{st_build_dir}/{st_package_name}/plugins",
            file_filter_function=plugin_dll_filter,
            vars=vars)

    if copy_translations:
        # <qt>/translations/* -> <setup>/{st_package_name}/translations
        copydir("{qt_translations_dir}",
            "{st_build_dir}/{st_package_name}/translations",
            filter=["*.qm", "*.pak"],
            force=False,
            vars=vars)

    if copy_qml:
        # <qt>/qml/* -> <setup>/{st_package_name}/qml
        qml_dll_patterns = ["*{}.dll"]
        qml_ignore_patterns = qml_dll_patterns + [pdb_pattern]
        qml_ignore = [a.format('') for a in qml_ignore_patterns]

        # Copy all files that are not dlls and pdbs (.qml, qmldir).
        copydir("{qt_qml_dir}", "{st_build_dir}/{st_package_name}/qml",
            ignore=qml_ignore,
            force=False,
            recursive=True,
            vars=vars)

        if copy_pdbs:
            qml_dll_patterns += [pdb_pattern]
        qml_dll_filter = functools.partial(qt_build_config_filter,
            qml_dll_patterns)

        # Copy all dlls (and possibly pdbs).
        copydir("{qt_qml_dir}", "{st_build_dir}/{st_package_name}/qml",
            file_filter_function=qml_dll_filter,
            force=False,
            recursive=True,
            vars=vars)

    if self.is_webengine_built(built_modules):
        copydir("{qt_prefix_dir}/resources",
            "{st_build_dir}/{st_package_name}/resources",
            filter=None,
            recursive=False,
            vars=vars)

        filter = 'QtWebEngineProcess{}.exe'.format(
            'd' if self.debug else '')
        copydir("{qt_bin_dir}",
            "{st_build_dir}/{st_package_name}",
            filter=[filter],
            recursive=False, vars=vars)

    if copy_qt_conf:
        # Copy the qt.conf file to prefix dir.
        copyfile(
            "{build_dir}/pyside2/{st_package_name}/qt.conf",
            "{st_build_dir}/{st_package_name}",
            vars=vars)

    if copy_clang:
        self.prepare_standalone_clang(is_win=True)
