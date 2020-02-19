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
import re
import subprocess
from distutils.spawn import find_executable


class QtInfo(object):
    def __init__(self, qmake_command=None):
        self.initialized = False

        if qmake_command:
            self._qmake_command = qmake_command
        else:
            self._qmake_command = [find_executable("qmake"), ]

        # Dict to cache qmake values.
        self._query_dict = {}
        # Dict to cache mkspecs variables.
        self._mkspecs_dict = {}
        # Initialize the properties.
        self._init_properties()

    def get_qmake_command(self):
        qmake_command_string = self._qmake_command[0]
        for entry in self._qmake_command[1:]:
            qmake_command_string += " {}".format(entry)
        return qmake_command_string

    def get_version(self):
        return self.get_property("QT_VERSION")

    def get_bins_path(self):
        return self.get_property("QT_INSTALL_BINS")

    def get_libs_path(self):
        return self.get_property("QT_INSTALL_LIBS")

    def get_libs_execs_path(self):
        return self.get_property("QT_INSTALL_LIBEXECS")

    def get_plugins_path(self):
        return self.get_property("QT_INSTALL_PLUGINS")

    def get_prefix_path(self):
        return self.get_property("QT_INSTALL_PREFIX")

    def get_imports_path(self):
        return self.get_property("QT_INSTALL_IMPORTS")

    def get_translations_path(self):
        return self.get_property("QT_INSTALL_TRANSLATIONS")

    def get_headers_path(self):
        return self.get_property("QT_INSTALL_HEADERS")

    def get_docs_path(self):
        return self.get_property("QT_INSTALL_DOCS")

    def get_qml_path(self):
        return self.get_property("QT_INSTALL_QML")

    def get_macos_deployment_target(self):
        """ Return value is a macOS version or None. """
        return self.get_property("QMAKE_MACOSX_DEPLOYMENT_TARGET")

    def get_build_type(self):
        """
        Return value is either debug, release, debug_release, or None.
        """
        return self.get_property("BUILD_TYPE")

    def get_src_dir(self):
        """ Return path to Qt src dir or None.. """
        return self.get_property("QT_INSTALL_PREFIX/src")

    def get_property(self, prop_name):
        if prop_name not in self._query_dict:
            return None
        return self._query_dict[prop_name]

    def get_properties(self):
        return self._query_dict

    def get_mkspecs_variables(self):
        return self._mkspecs_dict

    def _get_qmake_output(self, args_list=[]):
        cmd = self._qmake_command + args_list
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=False)
        output = proc.communicate()[0]
        proc.wait()
        if proc.returncode != 0:
            return ""
        if sys.version_info >= (3,):
            output = str(output, 'ascii').strip()
        else:
            output = output.strip()
        return output

    def _parse_query_properties(self, process_output):
        props = {}
        if not process_output:
            return props
        lines = [s.strip() for s in process_output.splitlines()]
        for line in lines:
            if line and ':' in line:
                key, value = line.split(':', 1)
                props[key] = value
        return props

    def _get_query_properties(self):
        output = self._get_qmake_output(['-query'])
        self._query_dict = self._parse_query_properties(output)

    def _parse_qt_build_type(self):
        key = 'QT_CONFIG'
        if key not in self._mkspecs_dict:
            return None

        qt_config = self._mkspecs_dict[key]
        if 'debug_and_release' in qt_config:
            return 'debug_and_release'

        split = qt_config.split(' ')
        if 'release' in split and 'debug' in split:
            return 'debug_and_release'

        if 'release' in split:
            return 'release'

        if 'debug' in split:
            return 'debug'

        return None

    def _get_other_properties(self):
        # Get the src property separately, because it is not returned by
        # qmake unless explicitly specified.
        key = 'QT_INSTALL_PREFIX/src'
        result = self._get_qmake_output(['-query', key])
        self._query_dict[key] = result

        # Get mkspecs variables and cache them.
        self._get_qmake_mkspecs_variables()

        # Get macOS minimum deployment target.
        key = 'QMAKE_MACOSX_DEPLOYMENT_TARGET'
        if key in self._mkspecs_dict:
            self._query_dict[key] = self._mkspecs_dict[key]

        # Figure out how Qt was built:
        #   debug mode, release mode, or both.
        build_type = self._parse_qt_build_type()
        if build_type:
            self._query_dict['BUILD_TYPE'] = build_type

    def _init_properties(self):
        self._get_query_properties()
        self._get_other_properties()

    def _get_qmake_mkspecs_variables(self):
        # Create empty temporary qmake project file.
        temp_file_name = 'qmake_fake_empty_project.txt'
        open(temp_file_name, 'a').close()

        # Query qmake for all of its mkspecs variables.
        qmake_output = self._get_qmake_output(['-E', temp_file_name])
        lines = [s.strip() for s in qmake_output.splitlines()]
        pattern = re.compile(r"^(.+?)=(.+?)$")
        for line in lines:
            found = pattern.search(line)
            if found:
                key = found.group(1).strip()
                value = found.group(2).strip()
                self._mkspecs_dict[key] = value

        # We need to clean up after qmake, which always creates a
        # .qmake.stash file after a -E invocation.
        qmake_stash_file = os.path.join(os.getcwd(), ".qmake.stash")
        if os.path.exists(qmake_stash_file):
            os.remove(qmake_stash_file)

        # Also clean up the temporary empty project file.
        if os.path.exists(temp_file_name):
            os.remove(temp_file_name)

    version = property(get_version)
    bins_dir = property(get_bins_path)
    libs_dir = property(get_libs_path)
    lib_execs_dir = property(get_libs_execs_path)
    plugins_dir = property(get_plugins_path)
    prefix_dir = property(get_prefix_path)
    qmake_command = property(get_qmake_command)
    imports_dir = property(get_imports_path)
    translations_dir = property(get_translations_path)
    headers_dir = property(get_headers_path)
    docs_dir = property(get_docs_path)
    qml_dir = property(get_qml_path)
    macos_min_deployment_target = property(get_macos_deployment_target)
    build_type = property(get_build_type)
    src_dir = property(get_src_dir)
