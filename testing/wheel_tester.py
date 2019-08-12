#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

"""
This script is used by Coin (coin_test_instructions.py specifically) to
test installation of generated wheels, and test building of the
"buildable" examples samplebinding and scriptableapplication.

It can also be invoked regularly from the command line via
python testing/wheel_tester.py --qmake=some-value --cmake=some-value

The qmake and cmake arguments can also be omitted, and they will be
looked up in your PATH.

Make sure that some generated wheels already exist in the dist/
directory (e.g. setup.py bdist_wheel was already executed).
"""
from __future__ import print_function, absolute_import

from argparse import ArgumentParser, RawTextHelpFormatter
import os, sys

try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
this_dir = os.path.dirname(this_file)
setup_script_dir = os.path.abspath(os.path.join(this_dir, '..'))
sys.path.append(setup_script_dir)

from build_scripts.options import OPTION_QMAKE
from build_scripts.options import OPTION_CMAKE

from build_scripts.utils import find_files_using_glob
from build_scripts.utils import find_glob_in_path
from build_scripts.utils import run_process, run_process_output
from build_scripts.utils import rmtree
import distutils.log as log
import platform

log.set_verbosity(1)


def find_executable_qmake():
    return find_executable('qmake', OPTION_QMAKE)


def find_executable_cmake():
    return find_executable('cmake', OPTION_CMAKE)


def find_executable(executable, command_line_value):
    value = command_line_value
    option_str = '--{}'.format(executable)

    if value:
        log.info("{} option given: {}".format(option_str, value))
        if not os.path.exists(value):
            raise RuntimeError("No executable exists at: {}".format(value))
    else:
        log.info("No {} option given, trying to find {} in PATH.".format(option_str, executable))
        paths = find_glob_in_path(executable)
        log.info("{} executables found in PATH: {}".format(executable, paths))
        if not paths:
            raise RuntimeError(
                "No {} option was specified and no {} was found "
                "in PATH.".format(option_str, executable))
        else:
            value = paths[0]
            log.info("Using {} found in PATH: {}".format(executable, value))
    log.info("")
    return value


QMAKE_PATH = find_executable_qmake()
CMAKE_PATH = find_executable_cmake()


def get_wheels_dir():
    return os.path.join(setup_script_dir, "dist")


def get_examples_dir():
    return os.path.join(setup_script_dir, "examples")


def package_prefix_names():
    return ["shiboken2", "shiboken2_generator", "PySide2"]


def clean_egg_info():
    # After a successful bdist_wheel build, some .egg-info directories
    # are left over, which confuse pip when invoking it via
    # python -m pip, making pip think that the packages are already
    # installed in the root source directory.
    # Clean up the .egg-info directories to fix this, it should be
    # safe to do so.
    paths = find_files_using_glob(setup_script_dir, "*.egg-info")
    for p in paths:
        log.info("Removing {}".format(p))
        rmtree(p)


def install_wheel(wheel_path):
    log.info("Installing wheel: {}".format(wheel_path))
    exit_code = run_process([sys.executable, "-m", "pip", "install", wheel_path])
    log.info("")
    if exit_code:
        raise RuntimeError("Error while installing wheel {}".format(wheel_path))


def try_install_wheels(wheels_dir, py_version):
    clean_egg_info()
    all_wheels_pattern = "*.whl"
    all_wheels = find_files_using_glob(wheels_dir, all_wheels_pattern)

    if len(all_wheels) > 1:
        log.info("Found the following wheels in {}: ".format(wheels_dir))
        for wheel in all_wheels:
            log.info(wheel)
    else:
        log.info("No wheels found in {}".format(wheels_dir))
    log.info("")

    for p in package_prefix_names():
        pattern = "{}-*cp{}*.whl".format(p, py_version)
        files = find_files_using_glob(wheels_dir, pattern)
        if files and len(files) == 1:
            wheel_path = files[0]
            install_wheel(wheel_path)
        elif len(files) > 1:
            raise RuntimeError("More than one wheel found for specific package and version.")
        else:
            raise RuntimeError("No wheels compatible with Python {} found "
                               "for testing.".format(py_version))


def is_unix():
    if sys.platform.startswith("linux") or sys.platform == "darwin":
        return True
    return False


def generate_build_cmake():
    args = [CMAKE_PATH]
    if is_unix():
        args.extend(["-G", "Unix Makefiles"])
    else:
        args.extend(["-G", "NMake Makefiles"])
    args.append("-DCMAKE_BUILD_TYPE=Release")
    args.append("-Dpython_interpreter={}".format(sys.executable))

    # Specify prefix path so find_package(Qt5) works.
    qmake_dir = os.path.abspath(os.path.join(os.path.dirname(QMAKE_PATH), ".."))
    args.append("-DCMAKE_PREFIX_PATH={}".format(qmake_dir))

    args.append("..")

    exit_code = run_process(args)
    if exit_code:
        raise RuntimeError("Failure while running cmake.")
    log.info("")


def generate_build_qmake():
    exit_code = run_process([QMAKE_PATH, "..", "python_interpreter={}".format(sys.executable)])
    if exit_code:
        raise RuntimeError("Failure while running qmake.")
    log.info("")


def raise_error_pyinstaller(msg):
    print()
    print("PYINST: {msg}".format(**locals()))
    print("PYINST:   sys.version         = {}".format(sys.version.splitlines()[0]))
    print("PYINST:   platform.platform() = {}".format(platform.platform()))
    print("PYINST: See the error message above.")
    print()
    for line in run_process_output([sys.executable, "-m", "pip", "list"]):
        print("PyInstaller pip list:  ", line)
    print()
    raise(RuntimeError(msg))


def compile_using_pyinstaller():
    src_path = os.path.join("..", "hello.py")
    spec_path = os.path.join("..", "hello_app.spec")
    exit_code = run_process([sys.executable, "-m", "PyInstaller", spec_path])
        # to create the spec file, this setting was used:
        #"--name=hello_app", "--console", "--log-level=DEBUG", src_path])
        # By using a spec file, we avoid all the probing that might disturb certain
        # platforms and also save some analysis time.
    if exit_code:
        # 2019-04-28 Raising on error is again enabled
        raise_error_pyinstaller("Failure while compiling script using PyInstaller.")
    log.info("")


def run_make():
    args = []
    if is_unix():
        executable = "make"
    else:
        executable = "nmake"
    args.append(executable)

    exit_code = run_process(args)
    if exit_code:
        raise RuntimeError("Failure while running {}.".format(executable))
    log.info("")


def run_make_install():
    args = []
    if is_unix():
        executable = "make"
    else:
        executable = "nmake"
    args.append(executable)
    args.append("install")

    exit_code = run_process(args)
    if exit_code:
        raise RuntimeError("Failed while running {} install.".format(executable))
    log.info("")


def run_compiled_script(binary_path):
    args = [binary_path]
    exit_code = run_process(args)
    if exit_code:
        raise_error_pyinstaller("Failure while executing compiled script: {}".format(binary_path))
    log.info("")


def execute_script(script_path):
    args = [sys.executable, script_path]
    exit_code = run_process(args)
    if exit_code:
        raise RuntimeError("Failure while executing script: {}".format(script_path))
    log.info("")


def prepare_build_folder(src_path, build_folder_name):
    build_path = os.path.join(src_path, build_folder_name)

    # The script can be called for both Python 2 and Python 3 wheels, so
    # preparing a build folder should clean any previous existing build.
    if os.path.exists(build_path):
        log.info("Removing {}".format(build_path))
        rmtree(build_path)

    log.info("Creating {}".format(build_path))
    os.makedirs(build_path)
    os.chdir(build_path)


def try_build_examples():
    examples_dir = get_examples_dir()

    # This script should better go to the last place, here.
    # But because it is most likely to break, we put it here for now.
    log.info("Attempting to build hello.py using PyInstaller.")
    # PyInstaller is loaded by coin_build_instructions.py, but not when
    # testing directly this script.
    src_path = os.path.join(examples_dir, "installer_test")
    prepare_build_folder(src_path, "pyinstaller")

    compile_using_pyinstaller()
    run_compiled_script(os.path.join(src_path,
                            "pyinstaller", "dist", "hello_app", "hello_app"))

    log.info("Attempting to build and run samplebinding using cmake.")
    src_path = os.path.join(examples_dir, "samplebinding")
    prepare_build_folder(src_path, "cmake")
    generate_build_cmake()
    run_make()
    run_make_install()
    execute_script(os.path.join(src_path, "main.py"))

    log.info("Attempting to build scriptableapplication using cmake.")
    src_path = os.path.join(examples_dir, "scriptableapplication")
    prepare_build_folder(src_path, "cmake")
    generate_build_cmake()
    run_make()

    log.info("Attempting to build scriptableapplication using qmake.")
    src_path = os.path.join(examples_dir, "scriptableapplication")
    prepare_build_folder(src_path, "qmake")
    generate_build_qmake()
    run_make()


def run_wheel_tests(install_wheels):
    wheels_dir = get_wheels_dir()
    py_version = sys.version_info[0]

    if install_wheels:
        log.info("Attempting to install wheels.\n")
        try_install_wheels(wheels_dir, py_version)

    log.info("Attempting to build examples.\n")
    try_build_examples()

    log.info("All tests passed!")


if __name__ == "__main__":
    parser = ArgumentParser(description="wheel_tester",
                           formatter_class=RawTextHelpFormatter)
    parser.add_argument('--no-install-wheels', '-n', action='store_true',
                        help='Do not install wheels'
                             ' (for developer builds with virtualenv)')
    options = parser.parse_args()
    run_wheel_tests(not options.no_install_wheels)
