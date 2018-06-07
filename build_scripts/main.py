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
from .utils import memoize, get_python_dict
from .options import *

setup_script_dir = os.getcwd()
build_scripts_dir = os.path.join(setup_script_dir, 'build_scripts')
setup_py_path = os.path.join(setup_script_dir, "setup.py")

@memoize
def get_package_timestamp():
    return int(time.time())

@memoize
def get_package_version():
    """ Returns the version string for the PySide2 package. """
    pyside_version_py = os.path.join(
        setup_script_dir, "sources", "pyside2", "pyside_version.py")
    d = get_python_dict(pyside_version_py)

    final_version = "{}.{}.{}".format(
        d['major_version'], d['minor_version'], d['patch_version'])
    pre_release_version_type = d['pre_release_version_type']
    pre_release_version = d['pre_release_version']
    if pre_release_version and pre_release_version_type:
        final_version += pre_release_version_type + pre_release_version

    # Add the current timestamp to the version number, to suggest it
    # is a development snapshot build.
    if OPTION_SNAPSHOT_BUILD:
        final_version += ".dev{}".format(get_package_timestamp())
    return final_version

def get_setuptools_extension_modules():
    # Setting py_limited_api on the extension is the "correct" thing
    # to do, but it doesn't actually do anything, because we
    # override build_ext. So this is just foolproofing for the
    # future.
    extension_args = ('QtCore', [])
    extension_kwargs = {}
    if OPTION_LIMITED_API:
        extension_kwargs['py_limited_api'] = True
    extension_modules = [Extension(*extension_args, **extension_kwargs)]
    return extension_modules

# Buildable extensions.
contained_modules = ['shiboken2', 'pyside2', 'pyside2-tools']

# Git submodules: ["submodule_name",
#   "location_relative_to_sources_folder"]
submodules = [["pyside2-tools"]]

pyside_package_dir_name = "pyside_package"

try:
    import setuptools
except ImportError:
    from ez_setup import use_setuptools
    use_setuptools()

import sys
import platform
import re
import fnmatch

import difflib # for a close match of dirname and module
import functools

from distutils import log
from distutils.errors import DistutilsOptionError
from distutils.errors import DistutilsSetupError
from distutils.sysconfig import get_config_var
from distutils.sysconfig import get_python_lib
from distutils.spawn import find_executable
from distutils.command.build import build as _build
from distutils.command.build_ext import build_ext as _build_ext
from distutils.util import get_platform

from setuptools import setup, Extension
from setuptools.command.install import install as _install
from setuptools.command.install_lib import install_lib as _install_lib
from setuptools.command.bdist_egg import bdist_egg as _bdist_egg
from setuptools.command.develop import develop as _develop
from setuptools.command.build_py import build_py as _build_py

from .qtinfo import QtInfo
from .utils import rmtree, detect_clang, copyfile, copydir, run_process_output, run_process
from .utils import update_env_path, init_msvc_env, filter_match, macos_fix_rpaths_for_library
from .platforms.unix import prepare_packages_posix
from .platforms.windows_desktop import prepare_packages_win32
from .wheel_override import wheel_module_exists, get_bdist_wheel_override

from textwrap import dedent

# make sure that setup.py is run with an allowed python version
def check_allowed_python_version():
    import re
    pattern = "'Programming Language :: Python :: (\d+)\.(\d+)'"
    supported = []
    with open(setup_py_path) as setup:
        for line in setup.readlines():
            found = re.search(pattern, line)
            if found:
                major = int(found.group(1))
                minor = int(found.group(2))
                supported.append( (major, minor) )
    this_py = sys.version_info[:2]
    if this_py not in supported:
        print("only these python versions are supported:", supported)
        sys.exit(1)

check_allowed_python_version()

qt_src_dir = ''

# This is used automatically by distutils.command.install object, to
# specify final installation location.
OPTION_FINAL_INSTALL_PREFIX = option_value("prefix")


if OPTION_QT_VERSION is None:
    OPTION_QT_VERSION = "5"
if OPTION_QMAKE is None:
    OPTION_QMAKE = find_executable("qmake-qt5")
if OPTION_QMAKE is None:
    OPTION_QMAKE = find_executable("qmake")

# make qtinfo.py independent of relative paths.
if OPTION_QMAKE is not None and os.path.exists(OPTION_QMAKE):
    OPTION_QMAKE = os.path.abspath(OPTION_QMAKE)
if OPTION_CMAKE is not None and os.path.exists(OPTION_CMAKE):
    OPTION_CMAKE = os.path.abspath(OPTION_CMAKE)

QMAKE_COMMAND = None
# Checking whether qmake executable exists
if OPTION_QMAKE is not None and os.path.exists(OPTION_QMAKE):
    # Looking whether qmake path is a link and whether the link exists
    if os.path.islink(OPTION_QMAKE) and os.path.lexists(OPTION_QMAKE):
        # Set -qt=X here.
        if "qtchooser" in os.readlink(OPTION_QMAKE):
            QMAKE_COMMAND = [OPTION_QMAKE, "-qt={}".format(OPTION_QT_VERSION)]
if not QMAKE_COMMAND:
    QMAKE_COMMAND = [OPTION_QMAKE]

if len(QMAKE_COMMAND) == 0 or QMAKE_COMMAND[0] is None:
    print("qmake could not be found.")
    sys.exit(1)
if not os.path.exists(QMAKE_COMMAND[0]):
    print("'{}' does not exist.".format(QMAKE_COMMAND[0]))
    sys.exit(1)
if OPTION_CMAKE is None:
    OPTION_CMAKE = find_executable("cmake")

if OPTION_CMAKE is None:
    print("cmake could not be found.")
    sys.exit(1)
if not os.path.exists(OPTION_CMAKE):
    print("'{}' does not exist.".format(OPTION_CMAKE))
    sys.exit(1)

if sys.platform == "win32":
    if OPTION_MAKESPEC is None:
        OPTION_MAKESPEC = "msvc"
    if not OPTION_MAKESPEC in ["msvc", "mingw"]:
        print("Invalid option --make-spec. Available values are {}".format(
            ["msvc", "mingw"]))
        sys.exit(1)
else:
    if OPTION_MAKESPEC is None:
        OPTION_MAKESPEC = "make"
    if not OPTION_MAKESPEC in ["make"]:
        print("Invalid option --make-spec. Available values are {}".format(
            ["make"]))
        sys.exit(1)

if OPTION_JOBS:
    if sys.platform == 'win32' and OPTION_NO_JOM:
        print("Option --jobs can only be used with jom on Windows.")
        sys.exit(1)
    else:
        if not OPTION_JOBS.startswith('-j'):
            OPTION_JOBS = '-j' + OPTION_JOBS
else:
    OPTION_JOBS = ''

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
    if OPTION_DEBUG:
        name += 'd'
    if is_debug_python():
        name += 'p'
    return name

# Initialize, pull and checkout submodules
def prepare_sub_modules():
    print("Initializing submodules for PySide2 version: {}".format(
        get_package_version()))
    submodules_dir = os.path.join(setup_script_dir, "sources")

    # Create list of [name, desired branch, absolute path, desired
    # branch] and determine whether all submodules are present
    need_init_sub_modules = False

    for m in submodules:
        module_name = m[0]
        module_dir = m[1] if len(m) > 1 else ''
        module_dir = os.path.join(submodules_dir, module_dir, module_name)
        # Check for non-empty directory (repository checked out)
        if not os.listdir(module_dir):
            need_init_sub_modules = True
            break

    if need_init_sub_modules:
        git_update_cmd = ["git", "submodule", "update", "--init"]
        if run_process(git_update_cmd) != 0:
            m = ("Failed to initialize the git submodules: "
                "update --init failed")
            raise DistutilsSetupError(m)
        git_pull_cmd = ["git", "submodule", "foreach", "git", "fetch", "--all"]
        if run_process(git_pull_cmd) != 0:
            m = ("Failed to initialize the git submodules: "
                "git fetch --all failed")
            raise DistutilsSetupError(m)
    else:
        print("All submodules present.")

    git_update_cmd = ["git", "submodule", "update"]
    if run_process(git_update_cmd) != 0:
        m = "Failed to checkout the correct git submodules SHA1s."
        raise DistutilsSetupError(m)

# Single global instance of QtInfo to be used later in multiple code
# paths.
qtinfo = QtInfo(QMAKE_COMMAND)

def get_qt_version():
    qt_version = qtinfo.version

    if not qt_version:
        log.error("Failed to query the Qt version with qmake {0}".format(
            self.qtinfo.qmake_command))
        sys.exit(1)

    if LooseVersion(qtinfo.version) < LooseVersion("5.7"):
        log.error("Incompatible Qt version detected: {}. "
            "A Qt version >= 5.7 is required.".format(qt_version))
        sys.exit(1)

    return qt_version

def prepare_build():
    if (os.path.isdir(".git") and not OPTION_IGNOREGIT and
            not OPTION_ONLYPACKAGE and not OPTION_REUSE_BUILD):
        prepare_sub_modules()
    # Clean up temp and package folders
    for n in [pyside_package_dir_name, "build"]:
        d = os.path.join(setup_script_dir, n)
        if os.path.isdir(d):
            print("Removing {}".format(d))
            try:
                rmtree(d)
            except Exception as e:
                print('***** problem removing "{}"'.format(d))
                print('ignored error: {}'.format(e))
    # Prepare package folders
    ppdn = pyside_package_dir_name
    absolute_paths = [os.path.join(ppdn, "PySide2"),
        os.path.join(ppdn, "pyside2uic")]
    for pkg in absolute_paths:
        pkg_dir = os.path.join(setup_script_dir, pkg)
        os.makedirs(pkg_dir)
    # locate Qt sources for the documentation
    if OPTION_QT_SRC is None:
        install_prefix = qtinfo.prefix_dir
        if install_prefix:
            global qt_src_dir
            # In-source, developer build
            if install_prefix.endswith("qtbase"):
                qt_src_dir = install_prefix
            else: # SDK: Use 'Src' directory
                qt_src_dir = os.path.join(os.path.dirname(install_prefix),
                    'Src', 'qtbase')

class PysideInstall(_install):
    def __init__(self, *args, **kwargs):
        _install.__init__(self, *args, **kwargs)

    def initialize_options (self):
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
        log.info('*** Install completed')

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



# pyside_build_py and pyside_install_lib are reimplemented to preserve
# symlinks when distutils / setuptools copy files to various
# directories through the different build stages.
class PysideBuildPy(_build_py):

    def __init__(self, *args, **kwargs):
        _build_py.__init__(self, *args, **kwargs)

    def build_package_data(self):
        """Copies files from pyside_package into build/xxx directory"""

        for package, src_dir, build_dir, filenames in self.data_files:
            for filename in filenames:
                target = os.path.join(build_dir, filename)
                self.mkpath(os.path.dirname(target))
                srcfile = os.path.abspath(os.path.join(src_dir, filename))
                # Using our own copyfile makes sure to preserve symlinks.
                copyfile(srcfile, target)

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
            outfiles = copydir(os.path.abspath(self.build_dir),
                os.path.abspath(self.install_dir))
        else:
            self.warn("'{}' does not exist -- "
                "no Python modules to install".format(self.build_dir))
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
        self.build_type = "Release"
        self.qtinfo = None
        self.build_tests = False

    def run(self):
        prepare_build()
        platform_arch = platform.architecture()[0]
        log.info("Python architecture is {}".format(platform_arch))

        build_type = "Debug" if OPTION_DEBUG else "Release"
        if OPTION_RELWITHDEBINFO:
            build_type = 'RelWithDebInfo'

        # Check env
        make_path = None
        make_generator = None
        if not OPTION_ONLYPACKAGE:
            if OPTION_MAKESPEC == "make":
                make_name = "make"
                make_generator = "Unix Makefiles"
            elif OPTION_MAKESPEC == "msvc":
                nmake_path = find_executable("nmake")
                if nmake_path is None or not os.path.exists(nmake_path):
                    log.info("nmake not found. "
                        "Trying to initialize the MSVC env...")
                    init_msvc_env(platform_arch, build_type)
                    nmake_path = find_executable("nmake")
                    assert(nmake_path is not None and
                        os.path.exists(nmake_path))
                jom_path = None if OPTION_NO_JOM else find_executable("jom")
                if jom_path is not None and os.path.exists(jom_path):
                    log.info("jom was found in {}".format(jom_path))
                    make_name = "jom"
                    make_generator = "NMake Makefiles JOM"
                else:
                    log.info("nmake was found in {}".format(nmake_path))
                    make_name = "nmake"
                    make_generator = "NMake Makefiles"
                    if OPTION_JOBS:
                        msg = "Option --jobs can only be used with 'jom' on Windows."
                        raise DistutilsSetupError(msg)
            elif OPTION_MAKESPEC == "mingw":
                make_name = "mingw32-make"
                make_generator = "MinGW Makefiles"
            else:
                raise DistutilsSetupError(
                    "Invalid option --make-spec.")
            make_path = find_executable(make_name)
            if make_path is None or not os.path.exists(make_path):
                raise DistutilsSetupError("You need the program '{}' on your "
                    "system path to compile PySide2.".format(make_name))

            if OPTION_CMAKE is None or not os.path.exists(OPTION_CMAKE):
                raise DistutilsSetupError(
                    "Failed to find cmake."
                    " Please specify the path to cmake with "
                    "--cmake parameter.")

        if OPTION_QMAKE is None or not os.path.exists(OPTION_QMAKE):
            raise DistutilsSetupError(
                "Failed to find qmake."
                " Please specify the path to qmake with --qmake parameter.")

        # Prepare parameters
        py_executable = sys.executable
        py_version = "{}.{}".format(sys.version_info[0], sys.version_info[1])
        py_include_dir = get_config_var("INCLUDEPY")
        py_libdir = get_config_var("LIBDIR")
        py_prefix = get_config_var("prefix")
        if not py_prefix or not os.path.exists(py_prefix):
            py_prefix = sys.prefix
        if sys.platform == "win32":
            py_scripts_dir = os.path.join(py_prefix, "Scripts")
        else:
            py_scripts_dir = os.path.join(py_prefix, "bin")
        if py_libdir is None or not os.path.exists(py_libdir):
            if sys.platform == "win32":
                py_libdir = os.path.join(py_prefix, "libs")
            else:
                py_libdir = os.path.join(py_prefix, "lib")
        if py_include_dir is None or not os.path.exists(py_include_dir):
            if sys.platform == "win32":
                py_include_dir = os.path.join(py_prefix, "include")
            else:
                py_include_dir = os.path.join(py_prefix,
                    "include/python{}".format(py_version))
        dbg_postfix = ""
        if build_type == "Debug":
            dbg_postfix = "_d"
        if sys.platform == "win32":
            if OPTION_MAKESPEC == "mingw":
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
            else: # Python 2
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
                lib_name = "libpython{}{}{}".format(py_version, lib_suff,
                    lib_ext)
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
                    possible_framework_path = os.path.realpath(
                        os.path.join(py_libdir, '..'))
                    possible_framework_version = os.path.basename(
                        possible_framework_path)
                    possible_framework_library = os.path.join(
                        possible_framework_path, 'Python')

                    if (possible_framework_version == '2.6' and
                            os.path.exists(possible_framework_library)):
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
                            lib_name = "libpython{}{}{}".format(
                                py_version, lib_suff, lib_ext)
                            py_library = os.path.join(try_py_libdir, lib_name)
                            if os.path.exists(py_library):
                                py_libdir = try_py_libdir
                                python_library_found = True
                                break
                            libs_tried.append(py_library)

            if not python_library_found:
                raise DistutilsSetupError(
                    "Failed to locate the Python library with {}".format(
                        ", ".join(libs_tried)))

            if py_library.endswith('.a'):
                # Python was compiled as a static library
                log.error("Failed to locate a dynamic Python library, "
                    "using {}".format(py_library))

        self.qtinfo = qtinfo
        qt_dir = os.path.dirname(OPTION_QMAKE)
        qt_version = get_qt_version()

        # Update the PATH environment variable
        additional_paths = [py_scripts_dir, qt_dir]

        # Add Clang to path for Windows.
        # Revisit once Clang is bundled with Qt.
        if (sys.platform == "win32" and
                LooseVersion(self.qtinfo.version) >= LooseVersion("5.7.0")):
            clang_dir = detect_clang()
            if clang_dir[0]:
                clangBinDir = os.path.join(clang_dir[0], 'bin')
                if not clangBinDir in os.environ.get('PATH'):
                    log.info("Adding {} as detected by {} to PATH".format(
                        clangBinDir, clang_dir[1]))
                    additional_paths.append(clangBinDir)
            else:
                raise DistutilsSetupError("Failed to detect Clang when checking "
                                          "LLVM_INSTALL_DIR, CLANG_INSTALL_DIR, llvm-config")

        update_env_path(additional_paths)

        build_name = "py{}-qt{}-{}-{}".format(py_version, qt_version,
            platform.architecture()[0], build_type.lower())

        script_dir = setup_script_dir
        sources_dir = os.path.join(script_dir, "sources")
        build_dir = os.path.join(script_dir, prefix() + "_build",
            "{}".format(build_name))
        install_dir = os.path.join(script_dir, prefix() + "_install",
            "{}".format(build_name))

        # Try to ensure that tools built by this script (such as shiboken2)
        # are found before any that may already be installed on the system.
        update_env_path([os.path.join(install_dir, 'bin')])

        # Tell cmake to look here for *.cmake files
        os.environ['CMAKE_PREFIX_PATH'] = install_dir

        self.make_path = make_path
        self.make_generator = make_generator
        self.debug = OPTION_DEBUG
        self.script_dir = script_dir
        self.pyside_package_dir = os.path.join(self.script_dir,
            pyside_package_dir_name)
        self.sources_dir = sources_dir
        self.build_dir = build_dir
        self.install_dir = install_dir
        self.py_executable = py_executable
        self.py_include_dir = py_include_dir
        self.py_library = py_library
        self.py_version = py_version
        self.build_type = build_type
        self.site_packages_dir = get_python_lib(1, 0, prefix=install_dir)
        self.build_tests = OPTION_BUILDTESTS

        setuptools_install_prefix = get_python_lib(1)
        if OPTION_FINAL_INSTALL_PREFIX:
            setuptools_install_prefix = OPTION_FINAL_INSTALL_PREFIX

        # Save the shiboken build dir path for clang deployment
        # purposes.
        self.shiboken_build_dir = os.path.join(self.build_dir, "shiboken2")

        log.info("=" * 30)
        log.info("Package version: {}".format(get_package_version()))
        log.info("Build type: {}".format(self.build_type))
        log.info("Build tests: {}".format(self.build_tests))
        log.info("-" * 3)
        log.info("Make path: {}".format(self.make_path))
        log.info("Make generator: {}".format(self.make_generator))
        log.info("Make jobs: {}".format(OPTION_JOBS))
        log.info("-" * 3)

        log.info("setup.py directory: {}".format(self.script_dir))
        log.info("Build scripts directory: {}".format(build_scripts_dir))
        log.info("Sources directory: {}".format(self.sources_dir))

        log.info(dedent("""
        Building PySide2 will create and touch directories
          in the following order:
            make build directory (py*_build/*/*) ->
            make install directory (py*_install/*/*) ->
            {} directory (pyside_package/*) ->
            setuptools build directory (build/*/*) ->
            setuptools install directory
              (usually path-installed-python/lib/python*/site-packages/*)
         """).format(pyside_package_dir_name))

        log.info("make build directory: {}".format(self.build_dir))
        log.info("make install directory: {}".format(self.install_dir))
        log.info("{} directory: {}".format(pyside_package_dir_name,
            self.pyside_package_dir))
        log.info("setuptools build directory: {}".format(
            os.path.join(self.script_dir, "build")))
        log.info("setuptools install directory: {}".format(
            setuptools_install_prefix))
        log.info("make-installed site-packages directory: {} \n"
                 "  (only relevant for copying files from "
                 "'make install directory' to '{} directory'".format(
                    self.site_packages_dir, pyside_package_dir_name))
        log.info("-" * 3)
        log.info("Python executable: {}".format(self.py_executable))
        log.info("Python includes: {}".format(self.py_include_dir))
        log.info("Python library: {}".format(self.py_library))
        log.info("Python prefix: {}".format(py_prefix))
        log.info("Python scripts: {}".format(py_scripts_dir))
        log.info("-" * 3)
        log.info("Qt qmake: {}".format(self.qtinfo.qmake_command))
        log.info("Qt version: {}".format(self.qtinfo.version))
        log.info("Qt bins: {}".format(self.qtinfo.bins_dir))
        log.info("Qt docs: {}".format(self.qtinfo.docs_dir))
        log.info("Qt plugins: {}".format(self.qtinfo.plugins_dir))
        log.info("-" * 3)
        if sys.platform == 'win32':
            log.info("OpenSSL dll directory: {}".format(OPTION_OPENSSL))
        if sys.platform == 'darwin':
            pyside_macos_deployment_target = (
                PysideBuild.macos_pyside_min_deployment_target()
                )
            log.info("MACOSX_DEPLOYMENT_TARGET set to: {}".format(
                pyside_macos_deployment_target))
        log.info("=" * 30)

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

        if not OPTION_ONLYPACKAGE:
            # Build extensions
            for ext in contained_modules:
                self.build_extension(ext)

            if OPTION_BUILDTESTS:
                # we record the latest successful build and note the
                # build directory for supporting the tests.
                timestamp = time.strftime('%Y-%m-%d_%H%M%S')
                build_history = os.path.join(setup_script_dir, 'build_history')
                unique_dir = os.path.join(build_history, timestamp)
                os.makedirs(unique_dir)
                fpath = os.path.join(unique_dir, 'build_dir.txt')
                with open(fpath, 'w') as f:
                    print(build_dir, file=f)
                log.info("Created {}".format(build_history))

        if not OPTION_SKIP_PACKAGING:
            # Build patchelf if needed
            self.build_patchelf()

            # Prepare packages
            self.prepare_packages()

            # Build packages
            _build.run(self)
        else:
            log.info("Skipped preparing and building packages.")
        log.info('*** Build completed')

    @staticmethod
    def macos_qt_min_deployment_target():
        target = qtinfo.macos_min_deployment_target

        if not target:
            raise DistutilsSetupError("Failed to query for Qt's "
                "QMAKE_MACOSX_DEPLOYMENT_TARGET.")
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
        setup_target = OPTION_MACOS_DEPLOYMENT_TARGET

        qt_target_split = [int(x) for x in qt_target.split('.')]
        if python_target:
            python_target_split = [int(x) for x in python_target.split('.')]
        if setup_target:
            setup_target_split = [int(x) for x in setup_target.split('.')]

        message = ("Can't set MACOSX_DEPLOYMENT_TARGET value to {} because "
                  "{} was built with minimum deployment target set to {}.")
        # setup.py provided OPTION_MACOS_DEPLOYMENT_TARGET value takes
        # precedence.
        if setup_target:
            if python_target and setup_target_split < python_target_split:
                raise DistutilsSetupError(message.format(setup_target,
                    "Python", python_target))
            if setup_target_split < qt_target_split:
                raise DistutilsSetupError(message.format(setup_target,
                    "Qt", qt_target))
            # All checks clear, use setup.py provided value.
            return setup_target

        # Setup.py value not provided,
        # use same value as provided by Qt.
        if python_target:
            maximum_target = '.'.join([str(e) for e in max(python_target_split,
                qt_target_split)])
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
        log.info("Building patchelf...")
        module_src_dir = os.path.join(self.sources_dir, "patchelf")
        build_cmd = [
            "g++",
            "{}/patchelf.cc".format(module_src_dir),
            "-o",
            "patchelf",
        ]
        if run_process(build_cmd) != 0:
            raise DistutilsSetupError("Error building patchelf")

    def build_extension(self, extension):
        # calculate the subrepos folder name

        log.info("Building module {}...".format(extension))

        # Prepare folders
        os.chdir(self.build_dir)
        module_build_dir = os.path.join(self.build_dir,  extension)
        skipflag_file = module_build_dir + '-skip'
        if os.path.exists(skipflag_file):
            log.info("Skipping {} because {} exists".format(extension,
                skipflag_file))
            return

        module_build_exists = os.path.exists(module_build_dir)
        if module_build_exists:
            if not OPTION_REUSE_BUILD:
                log.info("Deleting module build folder {}...".format(
                    module_build_dir))
                try:
                    rmtree(module_build_dir)
                except Exception as e:
                    print('***** problem removing "{}"'.format(
                        module_build_dir))
                    print('ignored error: {}'.format(e))
            else:
                log.info("Reusing module build folder {}...".format(
                    module_build_dir))
        if not os.path.exists(module_build_dir):
            log.info("Creating module build folder {}...".format(
                module_build_dir))
            os.makedirs(module_build_dir)
        os.chdir(module_build_dir)

        module_src_dir = os.path.join(self.sources_dir, extension)

        # Build module
        cmake_cmd = [
            OPTION_CMAKE,
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
        if OPTION_MODULE_SUBSET:
            module_sub_set = ''
            for m in OPTION_MODULE_SUBSET.split(','):
                if m.startswith('Qt'):
                    m = m[2:]
                if module_sub_set:
                    module_sub_set += ';'
                module_sub_set += m
            cmake_cmd.append("-DMODULES={}".format(module_sub_set))
        if OPTION_SKIP_MODULES:
            skip_modules = ''
            for m in OPTION_SKIP_MODULES.split(','):
                if m.startswith('Qt'):
                    m = m[2:]
                if skip_modules:
                    skip_modules += ';'
                skip_modules += m
            cmake_cmd.append("-DSKIP_MODULES={}".format(skip_modules))
        # Add source location for generating documentation
        cmake_src_dir = OPTION_QT_SRC if OPTION_QT_SRC else qt_src_dir
        cmake_cmd.append("-DQT_SRC_DIR={}".format(cmake_src_dir))
        log.info("Qt Source dir: {}".format(cmake_src_dir))

        if self.build_type.lower() == 'debug':
            cmake_cmd.append("-DPYTHON_DEBUG_LIBRARY={}".format(
                self.py_library))

        if OPTION_LIMITED_API == "yes":
            cmake_cmd.append("-DFORCE_LIMITED_API=yes")
        elif OPTION_LIMITED_API == "no":
            cmake_cmd.append("-DFORCE_LIMITED_API=no")
        elif not OPTION_LIMITED_API:
            pass
        else:
            raise DistutilsSetupError("option limited-api must be 'yes' or 'no' "
                                      "(default yes if applicable, i.e. python version >= 3.5)")

        if OPTION_VERBOSE_BUILD:
            cmake_cmd.append("-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON")

        if OPTION_SANITIZE_ADDRESS:
            # Some simple sanity checking. Only use at your own risk.
            if (sys.platform.startswith('linux') or
                    sys.platform.startswith('darwin')):
                cmake_cmd.append("-DSANITIZE_ADDRESS=ON")
            else:
                raise DistutilsSetupError("Address sanitizer can only be used "
                    "on Linux and macOS.")

        if extension.lower() == "pyside2":
            pyside_qt_conf_prefix = ''
            if OPTION_QT_CONF_PREFIX:
                pyside_qt_conf_prefix = OPTION_QT_CONF_PREFIX
            else:
                if OPTION_STANDALONE:
                    pyside_qt_conf_prefix = '"Qt"'
                if sys.platform == 'win32':
                    pyside_qt_conf_prefix = '"."'
            cmake_cmd.append("-DPYSIDE_QT_CONF_PREFIX={}".format(
                pyside_qt_conf_prefix))

            # Pass package version to CMake, so this string can be
            # embedded into _config.py file.
            package_version = get_package_version()
            cmake_cmd.append("-DPYSIDE_SETUP_PY_PACKAGE_VERSION={}".format(
                package_version))

            # In case if this is a snapshot build, also pass the
            # timestamp as a separate value, because it the only
            # version component that is actually generated by setup.py.
            timestamp = ''
            if OPTION_SNAPSHOT_BUILD:
                timestamp = get_package_timestamp()
            cmake_cmd.append("-DPYSIDE_SETUP_PY_PACKAGE_TIMESTAMP={}".format(
                timestamp))

        if extension.lower() == "shiboken2":
            cmake_cmd.append("-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=yes")
            if sys.version_info[0] > 2:
                cmake_cmd.append("-DUSE_PYTHON_VERSION=3.3")

        if sys.platform == 'darwin':
            if OPTION_MACOS_ARCH:
                # also tell cmake which architecture to use
                cmake_cmd.append("-DCMAKE_OSX_ARCHITECTURES:STRING={}".format(
                    OPTION_MACOS_ARCH))

            if OPTION_MACOS_USE_LIBCPP:
                # Explicitly link the libc++ standard library (useful
                # for macOS deployment targets lower than 10.9).
                # This is not on by default, because most libraries and
                # executables on macOS <= 10.8 are linked to libstdc++,
                # and mixing standard libraries can lead to crashes.
                # On macOS >= 10.9 with a similar minimum deployment
                # target, libc++ is linked in implicitly, thus the
                # option is a no-op in those cases.
                cmake_cmd.append("-DOSX_USE_LIBCPP=ON")

            if OPTION_MACOS_SYSROOT:
                cmake_cmd.append("-DCMAKE_OSX_SYSROOT={}".format(
                    OPTION_MACOS_SYSROOT))
            else:
                latest_sdk_path = run_process_output(['xcrun',
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
            deployment_target = \
                PysideBuild.macos_pyside_min_deployment_target()
            cmake_cmd.append("-DCMAKE_OSX_DEPLOYMENT_TARGET={}".format(
                deployment_target))
            os.environ['MACOSX_DEPLOYMENT_TARGET'] = deployment_target

        if not OPTION_SKIP_CMAKE:
            log.info("Configuring module {} ({})...".format(extension,
                module_src_dir))
            if run_process(cmake_cmd) != 0:
                raise DistutilsSetupError("Error configuring {}".format(
                    extension))
        else:
            log.info("Reusing old configuration for module {} ({})...".format(
                extension, module_src_dir))

        log.info("Compiling module {}...".format(extension))
        cmd_make = [self.make_path]
        if OPTION_JOBS:
            cmd_make.append(OPTION_JOBS)
        if run_process(cmd_make) != 0:
            raise DistutilsSetupError("Error compiling {}".format(extension))

        if not OPTION_SKIP_DOCS:
            if extension.lower() == "shiboken2":
                try:
                    # Check if sphinx is installed to proceed.
                    import sphinx

                    log.info("Generating Shiboken documentation")
                    if run_process([self.make_path, "doc"]) != 0:
                        raise DistutilsSetupError(
                            "Error generating documentation for {}".format(
                                extension))
                except ImportError:
                    log.info("Sphinx not found, skipping documentation build")
        else:
            log.info("Skipped documentation generation")


        if not OPTION_SKIP_MAKE_INSTALL:
            log.info("Installing module {}...".format(extension))
            # Need to wait a second, so installed file timestamps are
            # older than build file timestamps.
            # See https://gitlab.kitware.com/cmake/cmake/issues/16155
            # for issue details.
            if sys.platform == 'darwin':
                log.info("Waiting 1 second, to ensure installation is "
                    "successful...")
                time.sleep(1)
            if run_process([self.make_path, "install/fast"]) != 0:
                raise DistutilsSetupError("Error pseudo installing {}".format(
                    extension))
        else:
            log.info("Skipped installing module {}".format(extension))

        os.chdir(self.script_dir)

    def prepare_packages(self):
        try:
            log.info("Preparing packages...")
            vars = {
                "site_packages_dir": self.site_packages_dir,
                "sources_dir": self.sources_dir,
                "install_dir": self.install_dir,
                "build_dir": self.build_dir,
                "script_dir": self.script_dir,
                "pyside_package_dir": self.pyside_package_dir,
                "ssl_libs_dir": OPTION_OPENSSL,
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
            }
            os.chdir(self.script_dir)

            if sys.platform == "win32":
                vars['dbg_postfix'] = OPTION_DEBUG and "_d" or ""
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
        pyside_package_dir = vars['pyside_package_dir']
        config_path = os.path.join(pyside_package_dir, "PySide2", "_config.py")
        config = get_python_dict(config_path)
        return config

    def is_webengine_built(self, built_modules):
        return 'WebEngineWidgets' in built_modules or 'WebEngineCore' in built_modules

    def prepare_standalone_clang(self, is_win = False):
        """
        Copies the libclang library to the pyside package so that
        shiboken executable works.
        """
        log.info('Finding path to the libclang shared library.')
        cmake_cmd = [
            OPTION_CMAKE,
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
            clang_lib_path = re.sub(r'lib/libclang.lib$', 'bin/libclang.dll',
                clang_lib_path)
        else:
            # We want to resolve any symlink on Linux and macOS, and
            # copy the actual file.
            clang_lib_path = os.path.realpath(clang_lib_path)

        # Path to directory containing libclang.
        clang_lib_dir = os.path.dirname(clang_lib_path)

        # The destination will be the package folder near the other
        # extension modules.
        destination_dir = "{}/PySide2".format(os.path.join(self.script_dir,
            'pyside_package'))
        if os.path.exists(clang_lib_path):
            log.info('Copying libclang shared library to the package folder.')
            basename = os.path.basename(clang_lib_path)
            destination_path = os.path.join(destination_dir, basename)

            # Need to modify permissions in case file is not writable
            # (a reinstall would cause a permission denied error).
            copyfile(clang_lib_path, destination_path, make_writable_by_owner=True)
        else:
            raise RuntimeError("Error copying libclang library "
                               "from {} to {}. ".format(
                                    clang_lib_path, destination_dir))

    def update_rpath(self, package_path, executables):
        if sys.platform.startswith('linux'):
            pyside_libs = [lib for lib in os.listdir(
                package_path) if filter_match(lib, ["*.so", "*.so.*"])]

            patchelf_path = os.path.join(self.script_dir, "patchelf")

            def rpath_cmd(srcpath):
                final_rpath = ''
                # Command line rpath option takes precedence over
                # automatically added one.
                if OPTION_RPATH_VALUES:
                    final_rpath = OPTION_RPATH_VALUES
                else:
                    # Add rpath values pointing to $ORIGIN and the
                    # installed qt lib directory.
                    local_rpath = '$ORIGIN/'
                    qt_lib_dir = self.qtinfo.libs_dir
                    if OPTION_STANDALONE:
                        qt_lib_dir = "$ORIGIN/Qt/lib"
                    final_rpath = local_rpath + ':' + qt_lib_dir
                cmd = [patchelf_path, '--set-rpath', final_rpath, srcpath]
                if run_process(cmd) != 0:
                    raise RuntimeError("Error patching rpath in " + srcpath)

        elif sys.platform == 'darwin':
            pyside_libs = [lib for lib in os.listdir(
                package_path) if filter_match(lib, ["*.so", "*.dylib"])]
            def rpath_cmd(srcpath):
                final_rpath = ''
                # Command line rpath option takes precedence over
                # automatically added one.
                if OPTION_RPATH_VALUES:
                    final_rpath = OPTION_RPATH_VALUES
                else:
                    if OPTION_STANDALONE:
                        final_rpath = "@loader_path/Qt/lib"
                    else:
                        final_rpath = self.qtinfo.libs_dir
                macos_fix_rpaths_for_library(srcpath, final_rpath)

        else:
            raise RuntimeError('Not configured for platform ' +
                               sys.platform)

        pyside_libs.extend(executables)

        # Update rpath in PySide2 libs
        for srcname in pyside_libs:
            srcpath = os.path.join(package_path, srcname)
            if os.path.isdir(srcpath) or os.path.islink(srcpath):
                continue
            if not os.path.exists(srcpath):
                continue
            rpath_cmd(srcpath)
            print("Patched rpath to '$ORIGIN/' (Linux) or "
                "updated rpath (OS/X) in {}.".format(srcpath))


try:
    with open(os.path.join(setup_script_dir, 'README.rst')) as f:
        README = f.read()
    with open(os.path.join(setup_script_dir, 'CHANGES.rst')) as f:
        CHANGES = f.read()
except IOError:
    README = CHANGES = ''


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
