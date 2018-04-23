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

import fnmatch, os
from ..utils import copydir, copyfile, macos_fix_rpaths_for_library

def prepare_standalone_package_macos(self, executables, vars):
    built_modules = vars['built_modules']

    # Directory filter for skipping unnecessary files.
    def general_dir_filter(dir_name, parent_full_path, dir_full_path):
        if fnmatch.fnmatch(dir_name, "*.dSYM"):
            return False
        return True

    # <qt>/lib/* -> <setup>/PySide2/Qt/lib
    if self.qt_is_framework_build():
        framework_built_modules = [
            'Qt' + name + '.framework' for name in built_modules]

        def framework_dir_filter(dir_name, parent_full_path,
                dir_full_path):
            if '.framework' in dir_name:
                if dir_name.startswith('QtWebEngine') and not \
                            self.is_webengine_built(built_modules):
                    return False
            if dir_name in ['Headers', 'fonts']:
                return False
            if dir_full_path.endswith('Versions/Current'):
                return False
            if dir_full_path.endswith('Versions/5/Resources'):
                return False
            if dir_full_path.endswith('Versions/5/Helpers'):
                return False
            return general_dir_filter(dir_name, parent_full_path,
                dir_full_path)

        copydir("{qt_lib_dir}", "{pyside_package_dir}/PySide2/Qt/lib",
            recursive=True, vars=vars,
            ignore=["*.la", "*.a", "*.cmake", "*.pc", "*.prl"],
            dir_filter_function=framework_dir_filter)

        # Fix rpath for WebEngine process executable. The already
        # present rpath does not work because it assumes a symlink
        # from Versions/5/Helpers, thus adding two more levels of
        # directory hierarchy.
        if self.is_webengine_built(built_modules):
            qt_lib_path = "{pyside_package_dir}/PySide2/Qt/lib".format(
                **vars)
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
            ignored_modules.extend(['libQt5WebEngine*.dylib'])
        if 'WebKit' not in built_modules:
            ignored_modules.extend(['libQt5WebKit*.dylib'])
        accepted_modules = ['libQt5*.5.dylib']

        copydir("{qt_lib_dir}",
            "{pyside_package_dir}/PySide2/Qt/lib",
            filter=accepted_modules,
            ignore=ignored_modules,
            recursive=True, vars=vars, force_copy_symlinks=True)

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

            # Fix rpath for WebEngine process executable.
            pyside_package_dir = vars['pyside_package_dir']
            qt_libexec_path = "{}/PySide2/Qt/libexec".format(pyside_package_dir)
            binary = "QtWebEngineProcess"
            final_path = os.path.join(qt_libexec_path, binary)
            rpath = "@loader_path/../lib"
            macos_fix_rpaths_for_library(final_path, rpath)

            # Copy the qt.conf file to libexec.
            copyfile(
                "{build_dir}/pyside2/PySide2/qt.conf",
                "{pyside_package_dir}/PySide2/Qt/libexec",
                vars=vars)

    # <qt>/plugins/* -> <setup>/PySide2/Qt/plugins
    copydir("{qt_plugins_dir}",
        "{pyside_package_dir}/PySide2/Qt/plugins",
        filter=["*.dylib"],
        recursive=True,
        dir_filter_function=general_dir_filter,
        vars=vars)

    # <qt>/qml/* -> <setup>/PySide2/Qt/qml
    copydir("{qt_qml_dir}",
        "{pyside_package_dir}/PySide2/Qt/qml",
        filter=None,
        recursive=True,
        force=False,
        dir_filter_function=general_dir_filter,
        vars=vars)

    # <qt>/translations/* -> <setup>/PySide2/Qt/translations
    copydir("{qt_translations_dir}",
        "{pyside_package_dir}/PySide2/Qt/translations",
        filter=["*.qm", "*.pak"],
        force=False,
        vars=vars)
