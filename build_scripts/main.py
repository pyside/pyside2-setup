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

from __future__ import print_function
from distutils.version import LooseVersion

import os
import time
from .config import config
from .utils import memoize, get_python_dict
from .options import OPTION

setup_script_dir = os.getcwd()
build_scripts_dir = os.path.join(setup_script_dir, 'build_scripts')
setup_py_path = os.path.join(setup_script_dir, "setup.py")

start_time = int(time.time())


def elapsed():
    return int(time.time()) - start_time


@memoize
def get_package_timestamp():
    """ In a Coin CI build the returned timestamp will be the
        Coin integration id timestamp. For regular builds it's
        just the current timestamp or a user provided one."""
    return OPTION["PACKAGE_TIMESTAMP"] if OPTION["PACKAGE_TIMESTAMP"] else start_time


@memoize
def get_package_version():
    """ Returns the version string for the PySide2 package. """
    pyside_version_py = os.path.join(
        setup_script_dir, "sources", "pyside2", "pyside_version.py")
    d = get_python_dict(pyside_version_py)

    final_version = "{}.{}.{}".format(
        d['major_version'], d['minor_version'], d['patch_version'])
    release_version_type = d['release_version_type']
    pre_release_version = d['pre_release_version']
    if pre_release_version and release_version_type:
        final_version += release_version_type + pre_release_version
    if release_version_type.startswith("comm"):
        final_version += "." + release_version_type

    # Add the current timestamp to the version number, to suggest it
    # is a development snapshot build.
    if OPTION["SNAPSHOT_BUILD"]:
        final_version += ".dev{}".format(get_package_timestamp())
    return final_version


def get_setuptools_extension_modules():
    # Setting py_limited_api on the extension is the "correct" thing
    # to do, but it doesn't actually do anything, because we
    # override build_ext. So this is just foolproofing for the
    # future.
    extension_args = ('QtCore', [])
    extension_kwargs = {}
    if OPTION["LIMITED_API"]:
        extension_kwargs['py_limited_api'] = True
    extension_modules = [Extension(*extension_args, **extension_kwargs)]
    return extension_modules


try:
    import setuptools
except ImportError:
    from ez_setup import use_setuptools
    use_setuptools()

import sys
import platform
import re

import distutils.log as log
from distutils.errors import DistutilsSetupError
from distutils.sysconfig import get_config_var
from distutils.sysconfig import get_python_lib
from distutils.spawn import find_executable
from distutils.command.build import build as _build
from distutils.command.build_ext import build_ext as _build_ext
from distutils.util import get_platform

from setuptools import Extension
from setuptools.command.install import install as _install
from setuptools.command.install_lib import install_lib as _install_lib
from setuptools.command.bdist_egg import bdist_egg as _bdist_egg
from setuptools.command.develop import develop as _develop
from setuptools.command.build_py import build_py as _build_py

from .qtinfo import QtInfo
from .utils import rmtree, detect_clang, copyfile, copydir, run_process_output, run_process
from .utils import update_env_path, init_msvc_env, filter_match
from .utils import macos_fix_rpaths_for_library
from .utils import linux_fix_rpaths_for_library
from .platforms.unix import prepare_packages_posix
from .platforms.windows_desktop import prepare_packages_win32
from .wheel_override import wheel_module_exists, get_bdist_wheel_override

from textwrap import dedent


def check_allowed_python_version():
    """
    Make sure that setup.py is run with an allowed python version.
    """

    import re
    pattern = r'Programming Language :: Python :: (\d+)\.(\d+)'
    supported = []

    for line in config.python_version_classifiers:
        found = re.search(pattern, line)
        if found:
            major = int(found.group(1))
            minor = int(found.group(2))
            supported.append((major, minor))
    this_py = sys.version_info[:2]
    if this_py not in supported:
        print("Unsupported python version detected. Only these python versions are supported: {}"
              .format(supported))
        sys.exit(1)


qt_src_dir = ''

if OPTION["QT_VERSION"] is None:
    OPTION["QT_VERSION"] = "5"
if OPTION["QMAKE"] is None:
    OPTION["QMAKE"] = find_executable("qmake-qt5")
if OPTION["QMAKE"] is None:
    OPTION["QMAKE"] = find_executable("qmake")

# make qtinfo.py independent of relative paths.
if OPTION["QMAKE"] is not None and os.path.exists(OPTION["QMAKE"]):
    OPTION["QMAKE"] = os.path.abspath(OPTION["QMAKE"])
if OPTION["CMAKE"] is not None and os.path.exists(OPTION["CMAKE"]):
    OPTION["CMAKE"] = os.path.abspath(OPTION["CMAKE"])

QMAKE_COMMAND = None
# Checking whether qmake executable exists
if OPTION["QMAKE"] is not None and os.path.exists(OPTION["QMAKE"]):
    # Looking whether qmake path is a link and whether the link exists
    if os.path.islink(OPTION["QMAKE"]) and os.path.lexists(OPTION["QMAKE"]):
        # Set -qt=X here.
        if "qtchooser" in os.readlink(OPTION["QMAKE"]):
            QMAKE_COMMAND = [OPTION["QMAKE"], "-qt={}".format(OPTION["QT_VERSION"])]
if not QMAKE_COMMAND:
    QMAKE_COMMAND = [OPTION["QMAKE"]]

if len(QMAKE_COMMAND) == 0 or QMAKE_COMMAND[0] is None:
    print("qmake could not be found.")
    sys.exit(1)
if not os.path.exists(QMAKE_COMMAND[0]):
    print("'{}' does not exist.".format(QMAKE_COMMAND[0]))
    sys.exit(1)
if OPTION["CMAKE"] is None:
    OPTION["CMAKE"] = find_executable("cmake")

if OPTION["CMAKE"] is None:
    print("cmake could not be found.")
    sys.exit(1)
if not os.path.exists(OPTION["CMAKE"]):
    print("'{}' does not exist.".format(OPTION["CMAKE"]))
    sys.exit(1)

# First element is default
available_mkspecs = ["msvc", "mingw", "ninja"] if sys.platform == "win32" else ["make", "ninja"]

if OPTION["MAKESPEC"] is None:
    OPTION["MAKESPEC"] = available_mkspecs[0]

if OPTION["MAKESPEC"] not in available_mkspecs:
    print('Invalid option --make-spec "{}". Available values are {}'.format(OPTION["MAKESPEC"],
                                                                            available_mkspecs))
    sys.exit(1)

if OPTION["JOBS"]:
    if sys.platform == 'win32' and OPTION["NO_JOM"]:
        print("Option --jobs can only be used with jom on Windows.")
        sys.exit(1)
    else:
        if not OPTION["JOBS"].startswith('-j'):
            OPTION["JOBS"] = '-j' + OPTION["JOBS"]
else:
    OPTION["JOBS"] = ''


def is_debug_python():
    return getattr(sys, "gettotalrefcount", None) is not None


# Return a prefix suitable for the _install/_build directory
def prefix():
    virtual_env_name = os.environ.get('VIRTUAL_ENV', None)
    if virtual_env_name is not None:
        name = os.path.basename(virtual_env_name)
    else:
        name = "pyside"
    name += str(sys.version_info[0])
    if OPTION["DEBUG"]:
        name += "d"
    if is_debug_python():
        name += "p"
    if OPTION["LIMITED_API"] == "yes" and sys.version_info[0] == 3:
        name += "a"
    return name


# Single global instance of QtInfo to be used later in multiple code
# paths.
qtinfo = QtInfo(QMAKE_COMMAND)


def get_qt_version():
    qt_version = qtinfo.version

    if not qt_version:
        log.error("Failed to query the Qt version with qmake {0}".format(qtinfo.qmake_command))
        sys.exit(1)

    if LooseVersion(qtinfo.version) < LooseVersion("5.7"):
        log.error("Incompatible Qt version detected: {}. A Qt version >= 5.7 is "
                  "required.".format(qt_version))
        sys.exit(1)

    return qt_version


def prepare_build():
    # Clean up temp build folder.
    for n in ["build"]:
        d = os.path.join(setup_script_dir, n)
        if os.path.isdir(d):
            log.info("Removing {}".format(d))
            try:
                rmtree(d)
            except Exception as e:
                print('***** problem removing "{}"'.format(d))
                print('ignored error: {}'.format(e))

    # locate Qt sources for the documentation
    if OPTION["QT_SRC"] is None:
        install_prefix = qtinfo.prefix_dir
        if install_prefix:
            global qt_src_dir
            # In-source, developer build
            if install_prefix.endswith("qtbase"):
                qt_src_dir = install_prefix
            else:  # SDK: Use 'Src' directory
                qt_src_dir = os.path.join(os.path.dirname(install_prefix), 'Src', 'qtbase')


class PysideInstall(_install):
    def __init__(self, *args, **kwargs):
        _install.__init__(self, *args, **kwargs)

    def initialize_options(self):
        _install.initialize_options(self)

        if sys.platform == 'darwin':
            # Because we change the plat_name to include a correct
            # deployment target on macOS distutils thinks we are
            # cross-compiling, and throws an exception when trying to
            # execute setup.py install. The check looks like this
            # if self.warn_dir and build_plat != get_platform():
            #   raise DistutilsPlatformError("Can't install when "
            #                                  "cross-compiling")
            # Obviously get_platform will return the old deployment
            # target. The fix is to disable the warn_dir flag, which
            # was created for bdist_* derived classes to override, for
            # similar cases.
            self.warn_dir = False

    def run(self):
        _install.run(self)
        print('--- Install completed ({}s)'.format(elapsed()))


class PysideDevelop(_develop):

    def __init__(self, *args, **kwargs):
        _develop.__init__(self, *args, **kwargs)

    def run(self):
        self.run_command("build")
        _develop.run(self)


class PysideBdistEgg(_bdist_egg):

    def __init__(self, *args, **kwargs):
        _bdist_egg.__init__(self, *args, **kwargs)

    def run(self):
        self.run_command("build")
        _bdist_egg.run(self)


class PysideBuildExt(_build_ext):

    def __init__(self, *args, **kwargs):
        _build_ext.__init__(self, *args, **kwargs)

    def run(self):
        pass


class PysideBuildPy(_build_py):

    def __init__(self, *args, **kwargs):
        _build_py.__init__(self, *args, **kwargs)


# _install_lib is reimplemented to preserve
# symlinks when distutils / setuptools copy files to various
# directories from the setup tools build dir to the install dir.
class PysideInstallLib(_install_lib):

    def __init__(self, *args, **kwargs):
        _install_lib.__init__(self, *args, **kwargs)

    def install(self):
        """
        Installs files from build/xxx directory into final
        site-packages/PySide2 directory.
        """

        if os.path.isdir(self.build_dir):
            # Using our own copydir makes sure to preserve symlinks.
            outfiles = copydir(os.path.abspath(self.build_dir), os.path.abspath(self.install_dir))
        else:
            self.warn("'{}' does not exist -- no Python modules to install".format(self.build_dir))
            return
        return outfiles


class PysideBuild(_build):

    def __init__(self, *args, **kwargs):
        _build.__init__(self, *args, **kwargs)

    def finalize_options(self):
        os_name_backup = os.name
        if sys.platform == 'darwin':
            self.plat_name = PysideBuild.macos_plat_name()
            # This is a hack to circumvent the dubious check in
            # distutils.commands.build -> finalize_options, which only
            # allows setting the plat_name for windows NT.
            # That is not the case for the wheel module though (which
            # does allow setting plat_name), so we circumvent by faking
            # the os name when finalizing the options, and then
            # restoring the original os name.
            os.name = "nt"

        _build.finalize_options(self)

        if sys.platform == 'darwin':
            os.name = os_name_backup

    def initialize_options(self):
        _build.initialize_options(self)
        self.make_path = None
        self.make_generator = None
        self.debug = False
        self.script_dir = None
        self.sources_dir = None
        self.build_dir = None
        self.install_dir = None
        self.py_executable = None
        self.py_include_dir = None
        self.py_library = None
        self.py_version = None
        self.py_arch = None
        self.build_type = "Release"
        self.qtinfo = None
        self.build_tests = False

    def run(self):
        prepare_build()
        platform_arch = platform.architecture()[0]
        log.info("Python architecture is {}".format(platform_arch))
        self.py_arch = platform_arch[:-3]

        build_type = "Debug" if OPTION["DEBUG"] else "Release"
        if OPTION["RELWITHDEBINFO"]:
            build_type = 'RelWithDebInfo'

        # Check env
        make_path = None
        make_generator = None
        if not OPTION["ONLYPACKAGE"]:
            if OPTION["MAKESPEC"] == "make":
                make_name = "make"
                make_generator = "Unix Makefiles"
            elif OPTION["MAKESPEC"] == "msvc":
                nmake_path = find_executable("nmake")
                if nmake_path is None or not os.path.exists(nmake_path):
                    log.info("nmake not found. Trying to initialize the MSVC env...")
                    init_msvc_env(platform_arch, build_type)
                    nmake_path = find_executable("nmake")
                    assert(nmake_path is not None and os.path.exists(nmake_path))
                jom_path = None if OPTION["NO_JOM"] else find_executable("jom")
                if jom_path is not None and os.path.exists(jom_path):
                    log.info("jom was found in {}".format(jom_path))
                    make_name = "jom"
                    make_generator = "NMake Makefiles JOM"
                else:
                    log.info("nmake was found in {}".format(nmake_path))
                    make_name = "nmake"
                    make_generator = "NMake Makefiles"
                    if OPTION["JOBS"]:
                        msg = "Option --jobs can only be used with 'jom' on Windows."
                        raise DistutilsSetupError(msg)
            elif OPTION["MAKESPEC"] == "mingw":
                make_name = "mingw32-make"
                make_generator = "MinGW Makefiles"
            elif OPTION["MAKESPEC"] == "ninja":
                make_name = "ninja"
                make_generator = "Ninja"
            else:
                raise DistutilsSetupError("Invalid option --make-spec.")
            make_path = find_executable(make_name)
            if make_path is None or not os.path.exists(make_path):
                raise DistutilsSetupError("You need the program '{}' on your system path to "
                                          "compile PySide2.".format(make_name))

            if OPTION["CMAKE"] is None or not os.path.exists(OPTION["CMAKE"]):
                raise DistutilsSetupError("Failed to find cmake."
                                          " Please specify the path to cmake with "
                                          "--cmake parameter.")

        if OPTION["QMAKE"] is None or not os.path.exists(OPTION["QMAKE"]):
            raise DistutilsSetupError("Failed to find qmake. "
                                      "Please specify the path to qmake with --qmake parameter.")

        # Prepare parameters
        py_executable = sys.executable
        py_version = "{}.{}".format(sys.version_info[0], sys.version_info[1])
        py_include_dir = get_config_var("INCLUDEPY")
        py_libdir = get_config_var("LIBDIR")
        py_prefix = get_config_var("prefix")
        if not py_prefix or not os.path.exists(py_prefix):
            py_prefix = sys.prefix
        self.py_prefix = py_prefix
        if sys.platform == "win32":
            py_scripts_dir = os.path.join(py_prefix, "Scripts")
        else:
            py_scripts_dir = os.path.join(py_prefix, "bin")
        self.py_scripts_dir = py_scripts_dir
        if py_libdir is None or not os.path.exists(py_libdir):
            if sys.platform == "win32":
                # For virtual environments on Windows, the py_prefix will contain a path pointing
                # to it, instead of the system Python installation path.
                # Since INCLUDEPY contains a path to the system location, we use the same base
                # directory to define the py_libdir variable.
                py_libdir = os.path.join(os.path.dirname(py_include_dir), "libs")
                if not os.path.isdir(py_libdir):
                    raise DistutilsSetupError("Failed to locate the 'libs' directory")
            else:
                py_libdir = os.path.join(py_prefix, "lib")
        if py_include_dir is None or not os.path.exists(py_include_dir):
            if sys.platform == "win32":
                py_include_dir = os.path.join(py_prefix, "include")
            else:
                py_include_dir = os.path.join(py_prefix, "include/python{}".format(py_version))
        dbg_postfix = ""
        if build_type == "Debug":
            dbg_postfix = "_d"
        if sys.platform == "win32":
            if OPTION["MAKESPEC"] == "mingw":
                static_lib_name = "libpython{}{}.a".format(
                    py_version.replace(".", ""), dbg_postfix)
                py_library = os.path.join(py_libdir, static_lib_name)
            else:
                python_lib_name = "python{}{}.lib".format(
                    py_version.replace(".", ""), dbg_postfix)
                py_library = os.path.join(py_libdir, python_lib_name)
        else:
            lib_exts = ['.so']
            if sys.platform == 'darwin':
                lib_exts.append('.dylib')
            if sys.version_info[0] > 2:
                lib_suff = getattr(sys, 'abiflags', None)
            else:  # Python 2
                lib_suff = ''
            lib_exts.append('.so.1')
            # Suffix for OpenSuSE 13.01
            lib_exts.append('.so.1.0')
            # static library as last gasp
            lib_exts.append('.a')

            if sys.version_info[0] == 2 and dbg_postfix:
                # For Python2 add a duplicate set of extensions
                # combined with the dbg_postfix, so we test for both the
                # debug version of the lib and the normal one.
                # This allows a debug PySide2 to be built with a
                # non-debug Python.
                lib_exts = [dbg_postfix + e for e in lib_exts] + lib_exts

            python_library_found = False
            libs_tried = []
            for lib_ext in lib_exts:
                lib_name = "libpython{}{}{}".format(py_version, lib_suff, lib_ext)
                py_library = os.path.join(py_libdir, lib_name)
                if os.path.exists(py_library):
                    python_library_found = True
                    break
                libs_tried.append(py_library)
            else:
                # At least on macOS 10.11, the system Python 2.6 does
                # not include a symlink to the framework file disguised
                # as a .dylib file, thus finding the library would
                # fail.
                # Manually check if a framework file "Python" exists in
                # the Python framework bundle.
                if sys.platform == 'darwin' and sys.version_info[:2] == (2, 6):
                    # These manipulations essentially transform
                    # /System/Library/Frameworks/Python.framework/Versions/2.6/lib
                    # to
                    # /System/Library/Frameworks/Python.framework/Versions/2.6/Python
                    possible_framework_path = os.path.realpath(os.path.join(py_libdir, '..'))
                    possible_framework_version = os.path.basename(possible_framework_path)
                    possible_framework_library = os.path.join(possible_framework_path, 'Python')

                    if (possible_framework_version == '2.6'
                            and os.path.exists(possible_framework_library)):
                        py_library = possible_framework_library
                        python_library_found = True
                    else:
                        libs_tried.append(possible_framework_library)

                # Try to find shared libraries which have a multi arch
                # suffix.
                if not python_library_found:
                    py_multiarch = get_config_var("MULTIARCH")
                    if py_multiarch and not python_library_found:
                        try_py_libdir = os.path.join(py_libdir, py_multiarch)
                        libs_tried = []
                        for lib_ext in lib_exts:
                            lib_name = "libpython{}{}{}".format(py_version, lib_suff, lib_ext)
                            py_library = os.path.join(try_py_libdir, lib_name)
                            if os.path.exists(py_library):
                                py_libdir = try_py_libdir
                                python_library_found = True
                                break
                            libs_tried.append(py_library)

            if not python_library_found:
                raise DistutilsSetupError(
                    "Failed to locate the Python library with {}".format(", ".join(libs_tried)))

            if py_library.endswith('.a'):
                # Python was compiled as a static library
                log.error("Failed to locate a dynamic Python library, using {}".format(py_library))

        self.qtinfo = qtinfo
        qt_dir = os.path.dirname(OPTION["QMAKE"])
        qt_version = get_qt_version()

        # Update the PATH environment variable
        additional_paths = [self.py_scripts_dir, qt_dir]

        # Add Clang to path for Windows.
        # Revisit once Clang is bundled with Qt.
        if (sys.platform == "win32"
                and LooseVersion(self.qtinfo.version) >= LooseVersion("5.7.0")):
            clang_dir = detect_clang()
            if clang_dir[0]:
                clangBinDir = os.path.join(clang_dir[0], 'bin')
                if clangBinDir not in os.environ.get('PATH'):
                    log.info("Adding {} as detected by {} to PATH".format(clangBinDir,
                                                                          clang_dir[1]))
                    additional_paths.append(clangBinDir)
            else:
                raise DistutilsSetupError("Failed to detect Clang when checking "
                                          "LLVM_INSTALL_DIR, CLANG_INSTALL_DIR, llvm-config")

        update_env_path(additional_paths)

        # Used for test blacklists and registry test.
        self.build_classifiers = "py{}-qt{}-{}-{}".format(py_version, qt_version,
                                                          platform.architecture()[0],
                                                          build_type.lower())
        if OPTION["SHORTER_PATHS"]:
            build_name = "p{}".format(py_version)
        else:
            build_name = self.build_classifiers

        script_dir = setup_script_dir
        sources_dir = os.path.join(script_dir, "sources")
        build_dir = os.path.join(script_dir, prefix() + "_build", "{}".format(build_name))
        install_dir = os.path.join(script_dir, prefix() + "_install", "{}".format(build_name))

        self.make_path = make_path
        self.make_generator = make_generator
        self.debug = OPTION["DEBUG"]
        self.script_dir = script_dir
        self.st_build_dir = os.path.join(self.script_dir, self.build_lib)
        self.sources_dir = sources_dir
        self.build_dir = build_dir
        self.install_dir = install_dir
        self.py_executable = py_executable
        self.py_include_dir = py_include_dir
        self.py_library = py_library
        self.py_version = py_version
        self.build_type = build_type
        self.site_packages_dir = get_python_lib(1, 0, prefix=install_dir)
        self.build_tests = OPTION["BUILDTESTS"]

        # Save the shiboken build dir path for clang deployment
        # purposes.
        self.shiboken_build_dir = os.path.join(self.build_dir, "shiboken2")

        self.log_pre_build_info()

        # Prepare folders
        if not os.path.exists(self.sources_dir):
            log.info("Creating sources folder {}...".format(self.sources_dir))
            os.makedirs(self.sources_dir)
        if not os.path.exists(self.build_dir):
            log.info("Creating build folder {}...".format(self.build_dir))
            os.makedirs(self.build_dir)
        if not os.path.exists(self.install_dir):
            log.info("Creating install folder {}...".format(self.install_dir))
            os.makedirs(self.install_dir)

        if (not OPTION["ONLYPACKAGE"]
                and not config.is_internal_shiboken_generator_build_and_part_of_top_level_all()):
            # Build extensions
            for ext in config.get_buildable_extensions():
                self.build_extension(ext)

            if OPTION["BUILDTESTS"]:
                # we record the latest successful build and note the
                # build directory for supporting the tests.
                timestamp = time.strftime('%Y-%m-%d_%H%M%S')
                build_history = os.path.join(setup_script_dir, 'build_history')
                unique_dir = os.path.join(build_history, timestamp)
                os.makedirs(unique_dir)
                fpath = os.path.join(unique_dir, 'build_dir.txt')
                with open(fpath, 'w') as f:
                    print(build_dir, file=f)
                    print(self.build_classifiers, file=f)
                log.info("Created {}".format(build_history))

        if not OPTION["SKIP_PACKAGING"]:
            # Build patchelf if needed
            self.build_patchelf()

            # Prepare packages
            self.prepare_packages()

            # Build packages
            _build.run(self)
        else:
            log.info("Skipped preparing and building packages.")
        print('--- Build completed ({}s)'.format(elapsed()))

    def log_pre_build_info(self):
        if config.is_internal_shiboken_generator_build_and_part_of_top_level_all():
            return

        setuptools_install_prefix = get_python_lib(1)
        if OPTION["FINAL_INSTALL_PREFIX"]:
            setuptools_install_prefix = OPTION["FINAL_INSTALL_PREFIX"]
        log.info("=" * 30)
        log.info("Package version: {}".format(get_package_version()))
        log.info("Build type:  {}".format(self.build_type))
        log.info("Build tests: {}".format(self.build_tests))
        log.info("-" * 3)
        log.info("Make path:      {}".format(self.make_path))
        log.info("Make generator: {}".format(self.make_generator))
        log.info("Make jobs:      {}".format(OPTION["JOBS"]))
        log.info("-" * 3)
        log.info("setup.py directory:      {}".format(self.script_dir))
        log.info("Build scripts directory: {}".format(build_scripts_dir))
        log.info("Sources directory:       {}".format(self.sources_dir))
        log.info(dedent("""
        Building {st_package_name} will create and touch directories
          in the following order:
            make build directory (py*_build/*/*) ->
            make install directory (py*_install/*/*) ->
            setuptools build directory (build/*/*) ->
            setuptools install directory
              (usually path-installed-python/lib/python*/site-packages/*)
         """).format(st_package_name=config.package_name()))
        log.info("make build directory:   {}".format(self.build_dir))
        log.info("make install directory: {}".format(self.install_dir))
        log.info("setuptools build directory:   {}".format(self.st_build_dir))
        log.info("setuptools install directory: {}".format(setuptools_install_prefix))
        log.info(dedent("""
        make-installed site-packages directory: {}
         (only relevant for copying files from 'make install directory'
                                          to   'setuptools build directory'
         """).format(
            self.site_packages_dir))
        log.info("-" * 3)
        log.info("Python executable: {}".format(self.py_executable))
        log.info("Python includes:   {}".format(self.py_include_dir))
        log.info("Python library:    {}".format(self.py_library))
        log.info("Python prefix:     {}".format(self.py_prefix))
        log.info("Python scripts:    {}".format(self.py_scripts_dir))
        log.info("-" * 3)
        log.info("Qt qmake:   {}".format(self.qtinfo.qmake_command))
        log.info("Qt version: {}".format(self.qtinfo.version))
        log.info("Qt bins:    {}".format(self.qtinfo.bins_dir))
        log.info("Qt docs:    {}".format(self.qtinfo.docs_dir))
        log.info("Qt plugins: {}".format(self.qtinfo.plugins_dir))
        log.info("-" * 3)
        if sys.platform == 'win32':
            log.info("OpenSSL dll directory: {}".format(OPTION["OPENSSL"]))
        if sys.platform == 'darwin':
            pyside_macos_deployment_target = (
                PysideBuild.macos_pyside_min_deployment_target()
            )
            log.info("MACOSX_DEPLOYMENT_TARGET set to: {}".format(
                pyside_macos_deployment_target))
        log.info("=" * 30)

    @staticmethod
    def macos_qt_min_deployment_target():
        target = qtinfo.macos_min_deployment_target

        if not target:
            raise DistutilsSetupError("Failed to query for Qt's QMAKE_MACOSX_DEPLOYMENT_TARGET.")
        return target

    @staticmethod
    @memoize
    def macos_pyside_min_deployment_target():
        """
        Compute and validate PySide2 MACOSX_DEPLOYMENT_TARGET value.
        Candidate sources that are considered:
            - setup.py provided value
            - maximum value between minimum deployment target of the
              Python interpreter and the minimum deployment target of
              the Qt libraries.
        If setup.py value is provided, that takes precedence.
        Otherwise use the maximum of the above mentioned two values.
        """
        python_target = get_config_var('MACOSX_DEPLOYMENT_TARGET') or None
        qt_target = PysideBuild.macos_qt_min_deployment_target()
        setup_target = OPTION["MACOS_DEPLOYMENT_TARGET"]

        qt_target_split = [int(x) for x in qt_target.split('.')]
        if python_target:
            python_target_split = [int(x) for x in python_target.split('.')]
        if setup_target:
            setup_target_split = [int(x) for x in setup_target.split('.')]

        message = ("Can't set MACOSX_DEPLOYMENT_TARGET value to {} because "
                   "{} was built with minimum deployment target set to {}.")
        # setup.py provided OPTION["MACOS_DEPLOYMENT_TARGET"] value takes
        # precedence.
        if setup_target:
            if python_target and setup_target_split < python_target_split:
                raise DistutilsSetupError(message.format(setup_target, "Python",
                                                         python_target))
            if setup_target_split < qt_target_split:
                raise DistutilsSetupError(message.format(setup_target, "Qt",
                                                         qt_target))
            # All checks clear, use setup.py provided value.
            return setup_target

        # Setup.py value not provided,
        # use same value as provided by Qt.
        if python_target:
            maximum_target = '.'.join([str(e) for e in max(python_target_split, qt_target_split)])
        else:
            maximum_target = qt_target
        return maximum_target

    @staticmethod
    @memoize
    def macos_plat_name():
        deployment_target = PysideBuild.macos_pyside_min_deployment_target()
        # Example triple "macosx-10.12-x86_64".
        plat = get_platform().split("-")
        plat_name = "{}-{}-{}".format(plat[0], deployment_target, plat[2])
        return plat_name

    def build_patchelf(self):
        if not sys.platform.startswith('linux'):
            return
        self._patchelf_path = find_executable('patchelf')
        if self._patchelf_path:
            if not os.path.isabs(self._patchelf_path):
                self._patchelf_path = os.path.join(os.getcwd(), self._patchelf_path)
            log.info("Using {} ...".format(self._patchelf_path))
            return
        log.info("Building patchelf...")
        module_src_dir = os.path.join(self.sources_dir, "patchelf")
        build_cmd = ["g++", "{}/patchelf.cc".format(module_src_dir), "-o", "patchelf"]
        if run_process(build_cmd) != 0:
            raise DistutilsSetupError("Error building patchelf")
        self._patchelf_path = os.path.join(self.script_dir, "patchelf")

    def build_extension(self, extension):
        # calculate the subrepos folder name

        log.info("Building module {}...".format(extension))

        # Prepare folders
        os.chdir(self.build_dir)
        module_build_dir = os.path.join(self.build_dir, extension)
        skipflag_file = "{} -skip".format(module_build_dir)
        if os.path.exists(skipflag_file):
            log.info("Skipping {} because {} exists".format(extension, skipflag_file))
            return

        module_build_exists = os.path.exists(module_build_dir)
        if module_build_exists:
            if not OPTION["REUSE_BUILD"]:
                log.info("Deleting module build folder {}...".format(module_build_dir))
                try:
                    rmtree(module_build_dir)
                except Exception as e:
                    print('***** problem removing "{}"'.format(module_build_dir))
                    print('ignored error: {}'.format(e))
            else:
                log.info("Reusing module build folder {}...".format(module_build_dir))
        if not os.path.exists(module_build_dir):
            log.info("Creating module build folder {}...".format(module_build_dir))
            os.makedirs(module_build_dir)
        os.chdir(module_build_dir)

        module_src_dir = os.path.join(self.sources_dir, extension)

        # Build module
        cmake_cmd = [OPTION["CMAKE"]]
        if OPTION["QUIET"]:
            # Pass a special custom option, to allow printing a lot less information when doing
            # a quiet build.
            cmake_cmd.append('-DQUIET_BUILD=1')
            if self.make_generator == "Unix Makefiles":
                # Hide progress messages for each built source file.
                # Doesn't seem to work if set within the cmake files themselves.
                cmake_cmd.append('-DCMAKE_RULE_MESSAGES=0')

        cmake_cmd += [
            "-G", self.make_generator,
            "-DBUILD_TESTS={}".format(self.build_tests),
            "-DQt5Help_DIR={}".format(self.qtinfo.docs_dir),
            "-DCMAKE_BUILD_TYPE={}".format(self.build_type),
            "-DCMAKE_INSTALL_PREFIX={}".format(self.install_dir),
            module_src_dir
        ]
        cmake_cmd.append("-DPYTHON_EXECUTABLE={}".format(self.py_executable))
        cmake_cmd.append("-DPYTHON_INCLUDE_DIR={}".format(self.py_include_dir))
        cmake_cmd.append("-DPYTHON_LIBRARY={}".format(self.py_library))

        # If a custom shiboken cmake config directory path was provided, pass it to CMake.
        if OPTION["SHIBOKEN_CONFIG_DIR"] and config.is_internal_pyside_build():
            if os.path.exists(OPTION["SHIBOKEN_CONFIG_DIR"]):
                log.info("Using custom provided shiboken2 installation: {}"
                         .format(OPTION["SHIBOKEN_CONFIG_DIR"]))
                cmake_cmd.append("-DShiboken2_DIR={}".format(OPTION["SHIBOKEN_CONFIG_DIR"]))
            else:
                log.info("Custom provided shiboken2 installation not found. Path given: {}"
                         .format(OPTION["SHIBOKEN_CONFIG_DIR"]))

        if OPTION["MODULE_SUBSET"]:
            module_sub_set = ''
            for m in OPTION["MODULE_SUBSET"].split(','):
                if m.startswith('Qt'):
                    m = m[2:]
                if module_sub_set:
                    module_sub_set += ';'
                module_sub_set += m
            cmake_cmd.append("-DMODULES={}".format(module_sub_set))
        if OPTION["SKIP_MODULES"]:
            skip_modules = ''
            for m in OPTION["SKIP_MODULES"].split(','):
                if m.startswith('Qt'):
                    m = m[2:]
                if skip_modules:
                    skip_modules += ';'
                skip_modules += m
            cmake_cmd.append("-DSKIP_MODULES={}".format(skip_modules))
        # Add source location for generating documentation
        cmake_src_dir = OPTION["QT_SRC"] if OPTION["QT_SRC"] else qt_src_dir
        cmake_cmd.append("-DQT_SRC_DIR={}".format(cmake_src_dir))
        log.info("Qt Source dir: {}".format(cmake_src_dir))

        if self.build_type.lower() == 'debug':
            cmake_cmd.append("-DPYTHON_DEBUG_LIBRARY={}".format(
                self.py_library))

        if OPTION["LIMITED_API"] == "yes":
            cmake_cmd.append("-DFORCE_LIMITED_API=yes")
        elif OPTION["LIMITED_API"] == "no":
            cmake_cmd.append("-DFORCE_LIMITED_API=no")
        elif not OPTION["LIMITED_API"]:
            pass
        else:
            raise DistutilsSetupError("option limited-api must be 'yes' or 'no' "
                                      "(default yes if applicable, i.e. python version >= 3.5)")

        if OPTION["VERBOSE_BUILD"]:
            cmake_cmd.append("-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON")

        if OPTION["SANITIZE_ADDRESS"]:
            # Some simple sanity checking. Only use at your own risk.
            if (sys.platform.startswith('linux')
                    or sys.platform.startswith('darwin')):
                cmake_cmd.append("-DSANITIZE_ADDRESS=ON")
            else:
                raise DistutilsSetupError("Address sanitizer can only be used on Linux and macOS.")

        if extension.lower() == "pyside2":
            pyside_qt_conf_prefix = ''
            if OPTION["QT_CONF_PREFIX"]:
                pyside_qt_conf_prefix = OPTION["QT_CONF_PREFIX"]
            else:
                if OPTION["STANDALONE"]:
                    pyside_qt_conf_prefix = '"Qt"'
                if sys.platform == 'win32':
                    pyside_qt_conf_prefix = '"."'
            cmake_cmd.append("-DPYSIDE_QT_CONF_PREFIX={}".format(
                pyside_qt_conf_prefix))

        # Pass package version to CMake, so this string can be
        # embedded into _config.py file.
        package_version = get_package_version()
        cmake_cmd.append("-DPACKAGE_SETUP_PY_PACKAGE_VERSION={}".format(package_version))

        # In case if this is a snapshot build, also pass the
        # timestamp as a separate value, because it is the only
        # version component that is actually generated by setup.py.
        timestamp = ''
        if OPTION["SNAPSHOT_BUILD"]:
            timestamp = get_package_timestamp()
        cmake_cmd.append("-DPACKAGE_SETUP_PY_PACKAGE_TIMESTAMP={}".format(timestamp))

        if extension.lower() in ["shiboken2"]:
            cmake_cmd.append("-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=yes")
            if sys.version_info[0] > 2:
                cmake_cmd.append("-DUSE_PYTHON_VERSION=3.3")

        if sys.platform == 'darwin':
            if OPTION["MACOS_ARCH"]:
                # also tell cmake which architecture to use
                cmake_cmd.append("-DCMAKE_OSX_ARCHITECTURES:STRING={}".format(OPTION["MACOS_ARCH"]))

            if OPTION["MACOS_USE_LIBCPP"]:
                # Explicitly link the libc++ standard library (useful
                # for macOS deployment targets lower than 10.9).
                # This is not on by default, because most libraries and
                # executables on macOS <= 10.8 are linked to libstdc++,
                # and mixing standard libraries can lead to crashes.
                # On macOS >= 10.9 with a similar minimum deployment
                # target, libc++ is linked in implicitly, thus the
                # option is a no-op in those cases.
                cmake_cmd.append("-DOSX_USE_LIBCPP=ON")

            if OPTION["MACOS_SYSROOT"]:
                cmake_cmd.append("-DCMAKE_OSX_SYSROOT={}".format(
                                 OPTION["MACOS_SYSROOT"]))
            else:
                latest_sdk_path = run_process_output(['xcrun', '--sdk', 'macosx',
                                                      '--show-sdk-path'])
                if latest_sdk_path:
                    latest_sdk_path = latest_sdk_path[0]
                    cmake_cmd.append("-DCMAKE_OSX_SYSROOT={}".format(
                        latest_sdk_path))

            # Set macOS minimum deployment target (version).
            # This is required so that calling
            #   run_process -> distutils.spawn()
            # does not set its own minimum deployment target
            # environment variable which is based on the python
            # interpreter sysconfig value.
            # Doing so could break the detected clang include paths
            # for example.
            deployment_target = PysideBuild.macos_pyside_min_deployment_target()
            cmake_cmd.append("-DCMAKE_OSX_DEPLOYMENT_TARGET={}".format(deployment_target))
            os.environ['MACOSX_DEPLOYMENT_TARGET'] = deployment_target

        if OPTION["DOC_BUILD_ONLINE"]:
            log.info("Output format will be HTML")
            cmake_cmd.append("-DDOC_OUTPUT_FORMAT=html")
        else:
            log.info("Output format will be qthelp")
            cmake_cmd.append("-DDOC_OUTPUT_FORMAT=qthelp")

        if not OPTION["SKIP_CMAKE"]:
            log.info("Configuring module {} ({})...".format(extension, module_src_dir))
            if run_process(cmake_cmd) != 0:
                raise DistutilsSetupError("Error configuring {}".format(extension))
        else:
            log.info("Reusing old configuration for module {} ({})...".format(
                extension, module_src_dir))

        log.info("-- Compiling module {}...".format(extension))
        cmd_make = [self.make_path]
        if OPTION["JOBS"]:
            cmd_make.append(OPTION["JOBS"])
        if run_process(cmd_make) != 0:
            raise DistutilsSetupError("Error compiling {}".format(extension))

        if not OPTION["SKIP_DOCS"]:
            if extension.lower() == "shiboken2":
                try:
                    # Check if sphinx is installed to proceed.
                    import sphinx

                    log.info("Generating Shiboken documentation")
                    if run_process([self.make_path, "doc"]) != 0:
                        raise DistutilsSetupError("Error generating documentation "
                                                  "for {}".format(extension))
                except ImportError:
                    log.info("Sphinx not found, skipping documentation build")
        else:
            log.info("Skipped documentation generation")

        if not OPTION["SKIP_MAKE_INSTALL"]:
            log.info("Installing module {}...".format(extension))
            # Need to wait a second, so installed file timestamps are
            # older than build file timestamps.
            # See https://gitlab.kitware.com/cmake/cmake/issues/16155
            # for issue details.
            if sys.platform == 'darwin':
                log.info("Waiting 1 second, to ensure installation is successful...")
                time.sleep(1)
            # ninja: error: unknown target 'install/fast'
            target = 'install/fast' if self.make_generator != 'Ninja' else 'install'
            if run_process([self.make_path, target]) != 0:
                raise DistutilsSetupError("Error pseudo installing {}".format(
                    extension))
        else:
            log.info("Skipped installing module {}".format(extension))

        os.chdir(self.script_dir)

    def prepare_packages(self):
        """
        This will copy all relevant files from the various locations in the "cmake install dir",
        to the setup tools build dir (which is read from self.build_lib provided by distutils).

        After that setuptools.command.build_py is smart enough to copy everything
        from the build dir to the install dir (the virtualenv site-packages for example).
        """
        try:
            log.info("\nPreparing setup tools build directory.\n")
            vars = {
                "site_packages_dir": self.site_packages_dir,
                "sources_dir": self.sources_dir,
                "install_dir": self.install_dir,
                "build_dir": self.build_dir,
                "script_dir": self.script_dir,
                "st_build_dir": self.st_build_dir,
                "cmake_package_name": config.package_name(),
                "st_package_name": config.package_name(),
                "ssl_libs_dir": OPTION["OPENSSL"],
                "py_version": self.py_version,
                "qt_version": self.qtinfo.version,
                "qt_bin_dir": self.qtinfo.bins_dir,
                "qt_doc_dir": self.qtinfo.docs_dir,
                "qt_lib_dir": self.qtinfo.libs_dir,
                "qt_lib_execs_dir": self.qtinfo.lib_execs_dir,
                "qt_plugins_dir": self.qtinfo.plugins_dir,
                "qt_prefix_dir": self.qtinfo.prefix_dir,
                "qt_translations_dir": self.qtinfo.translations_dir,
                "qt_qml_dir": self.qtinfo.qml_dir,
                "target_arch": self.py_arch,
            }

            # Needed for correct file installation in generator build
            # case.
            if config.is_internal_shiboken_generator_build():
                vars['cmake_package_name'] = config.shiboken_module_option_name

            os.chdir(self.script_dir)

            if sys.platform == "win32":
                vars['dbg_postfix'] = OPTION["DEBUG"] and "_d" or ""
                return prepare_packages_win32(self, vars)
            else:
                return prepare_packages_posix(self, vars)
        except IOError as e:
            print('setup.py/prepare_packages: ', e)
            raise

    def qt_is_framework_build(self):
        if os.path.isdir(self.qtinfo.headers_dir + "/../lib/QtCore.framework"):
            return True
        return False

    def get_built_pyside_config(self, vars):
        # Get config that contains list of built modules, and
        # SOVERSIONs of the built libraries.
        st_build_dir = vars['st_build_dir']
        config_path = os.path.join(st_build_dir, config.package_name(), "_config.py")
        temp_config = get_python_dict(config_path)
        if 'built_modules' not in temp_config:
            temp_config['built_modules'] = []
        return temp_config

    def is_webengine_built(self, built_modules):
        return ('WebEngineWidgets' in built_modules
                or 'WebEngineCore' in built_modules
                or 'WebEngine' in built_modules)

    def prepare_standalone_clang(self, is_win=False):
        """
        Copies the libclang library to the shiboken2-generator
        package so that the shiboken executable works.
        """
        log.info('Finding path to the libclang shared library.')
        cmake_cmd = [
            OPTION["CMAKE"],
            "-L",         # Lists variables
            "-N",         # Just inspects the cache (faster)
            "--build",    # Specifies the build dir
            self.shiboken_build_dir
        ]
        out = run_process_output(cmake_cmd)
        lines = [s.strip() for s in out]
        pattern = re.compile(r"CLANG_LIBRARY:FILEPATH=(.+)$")

        clang_lib_path = None
        for line in lines:
            match = pattern.search(line)
            if match:
                clang_lib_path = match.group(1)
                break

        if not clang_lib_path:
            raise RuntimeError("Could not find the location of the libclang "
                               "library inside the CMake cache file.")

        if is_win:
            # clang_lib_path points to the static import library
            # (lib/libclang.lib), whereas we want to copy the shared
            # library (bin/libclang.dll).
            clang_lib_path = re.sub(r'lib/libclang.lib$',
                                    'bin/libclang.dll',
                                    clang_lib_path)
        else:
            # shiboken2 links against libclang.so.6 or a similarly
            # named library.
            # If the linked against library is a symlink, resolve
            # the symlink once (but not all the way to the real
            # file) on Linux and macOS,
            # so that we get the path to the "SO version" symlink
            # (the one used as the install name in the shared library
            # dependency section).
            # E.g. On Linux libclang.so -> libclang.so.6 ->
            # libclang.so.6.0.
            # "libclang.so.6" is the name we want for the copied file.
            if os.path.islink(clang_lib_path):
                link_target = os.readlink(clang_lib_path)
                if os.path.isabs(link_target):
                    clang_lib_path = link_target
                else:
                    # link_target is relative, transform to absolute.
                    clang_lib_path = os.path.join(os.path.dirname(clang_lib_path), link_target)
            clang_lib_path = os.path.abspath(clang_lib_path)

        # The destination will be the shiboken package folder.
        vars = {}
        vars['st_build_dir'] = self.st_build_dir
        vars['st_package_name'] = config.package_name()
        destination_dir = "{st_build_dir}/{st_package_name}".format(**vars)

        if os.path.exists(clang_lib_path):
            basename = os.path.basename(clang_lib_path)
            log.info('Copying libclang shared library {} to the package folder as {}.'.format(
                     clang_lib_path, basename))
            destination_path = os.path.join(destination_dir, basename)

            # Need to modify permissions in case file is not writable
            # (a reinstall would cause a permission denied error).
            copyfile(clang_lib_path,
                     destination_path,
                     force_copy_symlink=True,
                     make_writable_by_owner=True)
        else:
            raise RuntimeError("Error copying libclang library "
                               "from {} to {}. ".format(clang_lib_path, destination_dir))

    def update_rpath(self, package_path, executables):
        if sys.platform.startswith('linux'):
            pyside_libs = [lib for lib in os.listdir(
                package_path) if filter_match(lib, ["*.so", "*.so.*"])]

            def rpath_cmd(srcpath):
                final_rpath = ''
                # Command line rpath option takes precedence over
                # automatically added one.
                if OPTION["RPATH_VALUES"]:
                    final_rpath = OPTION["RPATH_VALUES"]
                else:
                    # Add rpath values pointing to $ORIGIN and the
                    # installed qt lib directory.
                    final_rpath = self.qtinfo.libs_dir
                    if OPTION["STANDALONE"]:
                        final_rpath = "$ORIGIN/Qt/lib"
                override = OPTION["STANDALONE"]
                linux_fix_rpaths_for_library(self._patchelf_path, srcpath, final_rpath,
                                             override=override)

        elif sys.platform == 'darwin':
            pyside_libs = [lib for lib in os.listdir(
                package_path) if filter_match(lib, ["*.so", "*.dylib"])]

            def rpath_cmd(srcpath):
                final_rpath = ''
                # Command line rpath option takes precedence over
                # automatically added one.
                if OPTION["RPATH_VALUES"]:
                    final_rpath = OPTION["RPATH_VALUES"]
                else:
                    if OPTION["STANDALONE"]:
                        final_rpath = "@loader_path/Qt/lib"
                    else:
                        final_rpath = self.qtinfo.libs_dir
                macos_fix_rpaths_for_library(srcpath, final_rpath)

        else:
            raise RuntimeError('Not configured for platform {}'.format(sys.platform))

        pyside_libs.extend(executables)

        # Update rpath in PySide2 libs
        for srcname in pyside_libs:
            srcpath = os.path.join(package_path, srcname)
            if os.path.isdir(srcpath) or os.path.islink(srcpath):
                continue
            if not os.path.exists(srcpath):
                continue
            rpath_cmd(srcpath)
            log.info("Patched rpath to '$ORIGIN/' (Linux) or "
                     "updated rpath (OS/X) in {}.".format(srcpath))


cmd_class_dict = {
    'build': PysideBuild,
    'build_py': PysideBuildPy,
    'build_ext': PysideBuildExt,
    'bdist_egg': PysideBdistEgg,
    'develop': PysideDevelop,
    'install': PysideInstall,
    'install_lib': PysideInstallLib
}
if wheel_module_exists:
    params = {}
    params['qt_version'] = get_qt_version()
    params['package_version'] = get_package_version()
    if sys.platform == 'darwin':
        params['macos_plat_name'] = PysideBuild.macos_plat_name()
    pyside_bdist_wheel = get_bdist_wheel_override(params)
    if pyside_bdist_wheel:
        cmd_class_dict['bdist_wheel'] = pyside_bdist_wheel
