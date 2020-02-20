#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################


import os
import sys


def _prepend_path_var(var_name, paths):
    """Prepend additional paths to a path environment variable
       like PATH, LD_LIBRARY_PATH"""
    old_paths = os.environ.get(var_name)
    new_paths = os.pathsep.join(paths)
    if old_paths:
        new_paths += '{}{}'.format(os.pathsep, old_paths)
    os.environ[var_name] = new_paths


def init_paths():
    """Sets the correct import paths (Python modules and C++ library
       paths) for testing shiboken depending on a single
       environment variable BUILD_DIR pointing to the build
       directory."""
    VAR_NAME = 'BUILD_DIR'
    build_dir = os.environ.get(VAR_NAME)
    if not build_dir:
        raise ValueError('{} is not set!'.format(VAR_NAME))
    if not os.path.isdir(build_dir):
        raise ValueError('{} is not a directory!'.format(build_dir))
    src_dir = os.path.dirname(os.path.abspath(__file__))
    if src_dir not in sys.path:  # Might be present due to import of this file
        sys.path.append(src_dir)  # For py3kcompat
    shiboken_dir = os.path.join(build_dir, 'shiboken2')
    sys.path.append(os.path.join(shiboken_dir, 'shibokenmodule'))
    lib_dirs = [os.path.join(shiboken_dir, 'libshiboken')]
    shiboken_test_dir = os.path.join(shiboken_dir, 'tests')
    for module in ['minimal', 'sample', 'smart', 'other']:
        module_dir = os.path.join(shiboken_test_dir, module + 'binding')
        sys.path.append(module_dir)
        lib_dir = os.path.join(shiboken_test_dir, 'lib' + module)
        lib_dirs.append(lib_dir)
    if sys.platform == 'win32':
        if sys.version_info >= (3, 8, 0):
            for lib_dir in lib_dirs:
                os.add_dll_directory(lib_dir)
        else:
            _prepend_path_var('PATH', lib_dirs)
    else:
        _prepend_path_var('LD_LIBRARY_PATH', lib_dirs)
