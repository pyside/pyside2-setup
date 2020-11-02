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

import fnmatch
import os
from ..utils import copydir, copyfile, macos_fix_rpaths_for_library, macos_add_rpath
from ..config import config
from ..versions import PYSIDE, SHIBOKEN


def prepare_standalone_package_macos(self, vars):
    built_modules = vars['built_modules']

    constrain_modules = None
    copy_plugins = True
    copy_qml = True
    copy_translations = True
    copy_qt_conf = True

    if config.is_internal_shiboken_generator_build():
        constrain_modules = ["Core", "Network", "Xml", "XmlPatterns"]
        constrain_frameworks = ['Qt' + name + '.framework' for name in constrain_modules]
        copy_plugins = False
        copy_qml = False
        copy_translations = False
        copy_qt_conf = False

    # Directory filter for skipping unnecessary files.
    def general_dir_filter(dir_name, parent_full_path, dir_full_path):
        if fnmatch.fnmatch(dir_name, "*.dSYM"):
            return False
        return True

    # Filter out debug plugins and qml plugins in the
    # debug_and_release config.
    no_copy_debug = True

    def file_variant_filter(file_name, file_full_path):
        if self.qtinfo.build_type != 'debug_and_release':
            return True
        if file_name.endswith('_debug.dylib') and no_copy_debug:
            return False
        return True

    # Patching designer to use the Qt libraries provided in the wheel
    if config.is_internal_pyside_build():
        designer_bundle = "{st_build_dir}/{st_package_name}/Designer.app".format(**vars)
        designer_binary = "{}/Contents/MacOS/Designer".format(designer_bundle)
        rpath = "@loader_path/../../../Qt/lib"
        macos_add_rpath(rpath, designer_binary)

    # <qt>/lib/* -> <setup>/{st_package_name}/Qt/lib
    if self.qt_is_framework_build():
        def framework_dir_filter(dir_name, parent_full_path, dir_full_path):
            if '.framework' in dir_name:
                if (dir_name.startswith('QtWebEngine')
                        and not self.is_webengine_built(built_modules)):
                    return False
                if constrain_modules and dir_name not in constrain_frameworks:
                    return False

            if dir_name in ['Headers', 'fonts']:
                return False
            if dir_full_path.endswith('Versions/Current'):
                return False
            if dir_full_path.endswith('Versions/5/Resources'):
                return False
            if dir_full_path.endswith('Versions/5/Helpers'):
                return False
            return general_dir_filter(dir_name, parent_full_path, dir_full_path)

        # Filter out debug frameworks in the
        # debug_and_release config.
        no_copy_debug = True

        def framework_variant_filter(file_name, file_full_path):
            if self.qtinfo.build_type != 'debug_and_release':
                return True
            dir_path = os.path.dirname(file_full_path)
            in_framework = dir_path.endswith("Versions/5")
            if file_name.endswith('_debug') and in_framework and no_copy_debug:
                return False
            return True

        copydir("{qt_lib_dir}", "{st_build_dir}/{st_package_name}/Qt/lib",
                recursive=True, vars=vars,
                ignore=["*.la", "*.a", "*.cmake", "*.pc", "*.prl"],
                dir_filter_function=framework_dir_filter,
                file_filter_function=framework_variant_filter)

        # Fix rpath for WebEngine process executable. The already
        # present rpath does not work because it assumes a symlink
        # from Versions/5/Helpers, thus adding two more levels of
        # directory hierarchy.
        if self.is_webengine_built(built_modules):
            qt_lib_path = "{st_build_dir}/{st_package_name}/Qt/lib".format(**vars)
            bundle = "QtWebEngineCore.framework/Helpers/"
            bundle += "QtWebEngineProcess.app"
            binary = "Contents/MacOS/QtWebEngineProcess"
            webengine_process_path = os.path.join(bundle, binary)
            final_path = os.path.join(qt_lib_path, webengine_process_path)
            rpath = "@loader_path/../../../../../"
            macos_fix_rpaths_for_library(final_path, rpath)
    else:
        ignored_modules = []
        if not self.is_webengine_built(built_modules):
            ignored_modules.extend(['libQt6WebEngine*.dylib'])
        accepted_modules = ['libQt6*.6.dylib']
        if constrain_modules:
            accepted_modules = ["libQt6" + module + "*.6.dylib" for module in constrain_modules]

        copydir("{qt_lib_dir}",
                "{st_build_dir}/{st_package_name}/Qt/lib",
                filter=accepted_modules,
                ignore=ignored_modules,
                file_filter_function=file_variant_filter,
                recursive=True, vars=vars, force_copy_symlinks=True)

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

            # Fix rpath for WebEngine process executable.
            qt_libexec_path = "{st_build_dir}/{st_package_name}/Qt/libexec".format(**vars)
            binary = "QtWebEngineProcess"
            final_path = os.path.join(qt_libexec_path, binary)
            rpath = "@loader_path/../lib"
            macos_fix_rpaths_for_library(final_path, rpath)

            if copy_qt_conf:
                # Copy the qt.conf file to libexec.
                copyfile(
                    f"{{build_dir}}/{PYSIDE}/{{st_package_name}}/qt.conf",
                    "{st_build_dir}/{st_package_name}/Qt/libexec",
                    vars=vars)

    if copy_plugins:
        # <qt>/plugins/* -> <setup>/{st_package_name}/Qt/plugins
        copydir("{qt_plugins_dir}",
                "{st_build_dir}/{st_package_name}/Qt/plugins",
                filter=["*.dylib"],
                recursive=True,
                dir_filter_function=general_dir_filter,
                file_filter_function=file_variant_filter,
                vars=vars)

    if copy_qml:
        # <qt>/qml/* -> <setup>/{st_package_name}/Qt/qml
        copydir("{qt_qml_dir}",
                "{st_build_dir}/{st_package_name}/Qt/qml",
                filter=None,
                recursive=True,
                force=False,
                dir_filter_function=general_dir_filter,
                file_filter_function=file_variant_filter,
                vars=vars)

    if copy_translations:
        # <qt>/translations/* ->
        # <setup>/{st_package_name}/Qt/translations
        copydir("{qt_translations_dir}",
                "{st_build_dir}/{st_package_name}/Qt/translations",
                filter=["*.qm", "*.pak"],
                force=False,
                vars=vars)
