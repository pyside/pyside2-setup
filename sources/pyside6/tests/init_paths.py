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

SRC_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(os.path.dirname(os.path.dirname(SRC_DIR)),
                             'shiboken6', 'tests'))
from shiboken_paths import (get_dir_env_var, get_build_dir, add_python_dirs,
                            add_lib_dirs, shiboken_paths)


def _get_qt_dir():
    """Retrieve the location of Qt."""
    result = None
    try:
        result = get_dir_env_var('QT_DIR')
    except Exception:
        # This is just a convenience for command line invocation independent
        # of ctest. Normally, QT_DIR should be set to avoid repetitive patch
        # searches.
        print('The environment variable QT_DIR is not set; falling back to path search',
              file=sys.stderr)
        qmake = 'qmake.exe' if sys.platform == 'win32' else 'qmake'
        for path in os.environ.get('PATH').split(os.pathsep):
            if path and os.path.isfile(os.path.join(path, qmake)):
                result = os.path.dirname(path)
                break
        if not result:
            raise ValueError('Unable to locate Qt. Please set the environment variable QT_DIR')
        print('Qt as determined by path search: {}'.format(result), file=sys.stderr)
    return result


def _get_qt_lib_dir():
    """Retrieve the location of the Qt library directory."""
    sub_dir = 'bin' if sys.platform == 'win32' else 'lib'
    return os.path.join(_get_qt_dir(), sub_dir)  # For testbinding


def _init_test_paths(shiboken_tests=False, testbindings_module=False):
    """Sets the correct import paths (Python modules and C++ library paths)
       for PySide tests and shiboken6 tests using depending on the environment
       variables BUILD_DIR and QT_DIR pointing to the build directory and
       Qt directory, respectively."""
    src_dir = os.path.dirname(os.path.abspath(__file__))

    python_dirs = [os.path.join(src_dir, 'util')]  # Helper module

    pyside_build_dir = os.path.join(get_build_dir(), 'pyside6')
    python_dirs.append(pyside_build_dir)   # for PySide6
    lib_dirs = [os.path.join(pyside_build_dir, 'libpyside')]

    if testbindings_module:
        python_dirs.append(os.path.join(pyside_build_dir,
                                        'tests', 'pysidetest'))
        lib_dirs.append(_get_qt_lib_dir())

    shiboken_path_tuple = shiboken_paths(shiboken_tests)
    python_dirs.extend(shiboken_path_tuple[0])
    lib_dirs.extend(shiboken_path_tuple[1])

    add_python_dirs(python_dirs)
    add_lib_dirs(lib_dirs)


def init_test_paths(testbindings_module=False):
    """Sets the correct import paths for PySide6 tests, optionally including
       testbindings."""
    _init_test_paths(False, testbindings_module)


def init_all_test_paths():
    """Sets the correct import paths for PySide6 and shiboken6 tests
       (for registry checking only)."""
    _init_test_paths(True, True)
