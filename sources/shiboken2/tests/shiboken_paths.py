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


def get_dir_env_var(var_name):
    """Return a directory set by an environment variable"""
    result = os.environ.get(var_name)
    if not result:
        raise ValueError('{} is not set!'.format(var_name))
    if not os.path.isdir(result):
        raise ValueError('{} is not a directory!'.format(result))
    return result


def get_build_dir():
    """
    Return the env var `BUILD_DIR`.
    If not set (interactive mode), take the last build history entry.
    """
    try:
        return get_dir_env_var('BUILD_DIR')
    except ValueError:
        look_for = "testing"
        here = os.path.dirname(__file__)
        while look_for not in os.listdir(here):
            here = os.path.dirname(here)
            if len(here) <= 3:
                raise SystemError(look_for + " not found!")
        try:
            sys.path.insert(0, here)
            from testing.buildlog import builds
            if not builds.history:
                raise
            return builds.history[-1].build_dir
        finally:
            del sys.path[0]


def _prepend_path_var(var_name, paths):
    """Prepend additional paths to a path environment variable
       like PATH, LD_LIBRARY_PATH"""
    old_paths = os.environ.get(var_name)
    new_paths = os.pathsep.join(paths)
    if old_paths:
        new_paths += '{}{}'.format(os.pathsep, old_paths)
    os.environ[var_name] = new_paths


def add_python_dirs(python_dirs):
    """Add directories to the Python path unless present.
       Care is taken that the added directories come before
       site-packages."""
    new_paths = []
    for python_dir in python_dirs:
        new_paths.append(python_dir)
        if python_dir in sys.path:
            sys.path.remove(python_dir)
    sys.path[:0] = new_paths


def add_lib_dirs(lib_dirs):
    """Add directories to the platform's library path."""
    if sys.platform == 'win32':
        if sys.version_info >= (3, 8, 0):
            for lib_dir in lib_dirs:
                os.add_dll_directory(lib_dir)
        else:
            _prepend_path_var('PATH', lib_dirs)
    else:
        _prepend_path_var('LD_LIBRARY_PATH', lib_dirs)


def shiboken_paths(include_shiboken_tests=False):
    """Return a tuple of python directories/lib directories to be set for
       using the shiboken2 module from the build directory or running the
       shiboken tests depending on a single environment variable BUILD_DIR
       pointing to the build directory."""
    src_dir = os.path.dirname(os.path.abspath(__file__))
    python_dirs = []
    if include_shiboken_tests:
        python_dirs.append(src_dir)  # For shiboken_test_helper
    shiboken_dir = os.path.join(get_build_dir(), 'shiboken2')
    python_dirs.append(os.path.join(shiboken_dir, 'shibokenmodule'))
    lib_dirs = [os.path.join(shiboken_dir, 'libshiboken')]
    if include_shiboken_tests:
        shiboken_test_dir = os.path.join(shiboken_dir, 'tests')
        for module in ['minimal', 'sample', 'smart', 'other']:
            module_dir = os.path.join(shiboken_test_dir, module + 'binding')
            python_dirs.append(module_dir)
            lib_dir = os.path.join(shiboken_test_dir, 'lib' + module)
            lib_dirs.append(lib_dir)
    return (python_dirs, lib_dirs)


def init_paths():
    """Sets the correct import paths (Python modules and C++ library
       paths) for testing shiboken depending on a single
       environment variable BUILD_DIR pointing to the build
       directory."""
    paths = shiboken_paths(True)
    add_python_dirs(paths[0])
    add_lib_dirs(paths[1])
