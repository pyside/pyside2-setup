#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of PySide2.
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

"""This is a distutils setup-script for the PySide2 project

To build the PySide2, simply execute:
  python setup.py build --qmake=</path/to/qt/bin/qmake> [--cmake=</path/to/cmake>] [--openssl=</path/to/openssl/bin>]
or
  python setup.py install --qmake=</path/to/qt/bin/qmake> [--cmake=</path/to/cmake>] [--openssl=</path/to/openssl/bin>]
to build and install into your current Python installation.

On Linux you can use option --standalone, to embed Qt libraries to PySide2 distribution

You can use option --only-package, if you want to create more binary packages (bdist_wheel, bdist_egg, ...)
without rebuilding entire PySide2 every time:
  # First time we create bdist_wheel with full PySide2 build
  python setup.py bdist_wheel --qmake=c:\Qt\4.8.5\bin\qmake.exe --cmake=c:\tools\cmake\bin\cmake.exe --openssl=c:\libs\OpenSSL32bit\bin

  # Then we create bdist_egg reusing PySide2 build with option --only-package
  python setup.py bdist_egg --only-package --qmake=c:\Qt\4.8.5\bin\qmake.exe --cmake=c:\tools\cmake\bin\cmake.exe --opnessl=c:\libs\OpenSSL32bit\bin

For development purposes the following options might be of use, when using "setup.py build":
    --reuse-build option allows recompiling only the modified sources and not the whole world,
      shortening development iteration time,
    --skip-cmake will reuse the already generated Makefiles (or equivalents), instead of invoking,
      CMake to update the Makefiles (note, CMake should be ran at least once to generate the files),
    --skip-make-install will not run make install (or equivalent) for each built module,
    --skip-packaging will skip creation of the python package,
    --ignore-git will skip the fetching and checkout steps for supermodule and all submodules.

REQUIREMENTS:
- Python: 2.6, 2.7, 3.3, 3.4, 3.5 and 3.6 are supported
- Cmake: Specify the path to cmake with --cmake option or add cmake to the system path.
- Qt: 5.5 and 5.6 are supported. Specify the path to qmake with --qmake option or add qmake to the system path.

OPTIONAL:
OpenSSL: You can specify the location of OpenSSL DLLs with option --opnessl=</path/to/openssl/bin>.
    You can download OpenSSL for windows here: http://slproweb.com/products/Win32OpenSSL.html

OS X SDK: You can specify which OS X SDK should be used for compilation with the option --osx-sysroot=</path/to/sdk>.
          For e.g. "--osx-sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/".
"""

__version__ = "5.9"

submodules = {
    '2.0.0.dev0': [
        ["shiboken2", "dev"],
        ["pyside2", "dev"],
        ["pyside2-tools", "dev"],
        ["pyside2-examples", "dev"],
        ["wiki", "master", ".."],
    ],
    '5.9': [
        ["shiboken2", "5.9"],
        ["pyside2", "5.9"],
        ["pyside2-tools", "5.9"],
        ["pyside2-examples", "5.9"],
        ["wiki", "master", ".."]
    ],
    '5.6': [
        ["shiboken2", "5.6"],
        ["pyside2", "5.6"],
        ["pyside2-tools", "5.6"],
        ["pyside2-examples", "5.6"],
        ["wiki", "master", ".."]
    ],
}
old_submodules = {
    # these are just kept a while for reference but not maintained.
    # if you need an old version, please use the pyside/pyside-setup version.
    '1.3.0dev': [
        ["shiboken", "master"],
        ["pyside", "master"],
        ["pyside-tools", "master"],
        ["pyside-examples", "master"],
    ],
    '1.2.2': [
        ["shiboken", "1.2.2"],
        ["pyside", "1.2.2"],
        ["pyside-tools", "0.2.15"],
        ["pyside-examples", "master"],
    ],
    '1.2.1': [
        ["shiboken", "1.2.1"],
        ["pyside", "1.2.1"],
        ["pyside-tools", "0.2.15"],
        ["pyside-examples", "master"],
    ],
    '1.2.0': [
        ["shiboken", "1.2.0"],
        ["pyside", "1.2.0"],
        ["pyside-tools", "0.2.14"],
        ["pyside-examples", "master"],
    ],
    '1.1.2': [
        ["shiboken", "1.1.2"],
        ["pyside", "1.1.2"],
        ["pyside-tools", "0.2.14"],
        ["pyside-examples", "master"],
    ],
    '1.1.1': [
        ["shiboken", "1.1.1"],
        ["pyside", "1.1.1"],
        ["pyside-tools", "0.2.14"],
        ["pyside-examples", "master"],
    ],
}

try:
    import setuptools
except ImportError:
    from ez_setup import use_setuptools
    use_setuptools()

import os
import sys
import platform
import time

import difflib # for a close match of dirname and module

from distutils import log
from distutils.errors import DistutilsOptionError
from distutils.errors import DistutilsSetupError
from distutils.sysconfig import get_config_var
from distutils.sysconfig import get_python_lib
from distutils.spawn import find_executable
from distutils.command.build import build as _build
from distutils.command.build_ext import build_ext as _build_ext

from setuptools import setup, Extension
from setuptools.command.install import install as _install
from setuptools.command.bdist_egg import bdist_egg as _bdist_egg
from setuptools.command.develop import develop as _develop

from qtinfo import QtInfo
from utils import rmtree
from utils import makefile
from utils import copyfile
from utils import copydir
from utils import run_process_output, run_process
from utils import has_option
from utils import option_value
from utils import update_env_path
from utils import init_msvc_env
from utils import regenerate_qt_resources
from utils import filter_match
from utils import osx_localize_libpaths

# guess a close folder name for extensions
def get_extension_folder(ext):
    maybe = list(map(lambda x:x[0], submodules[__version__]))
    folder = difflib.get_close_matches(ext, maybe)[0]
    return folder

# make sure that setup.py is run with an allowed python version
def check_allowed_python_version():
    import re
    pattern = "'Programming Language :: Python :: (\d+)\.(\d+)'"
    supported = []
    with open(__file__) as setup:
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

qtSrcDir = ''

# Declare options
OPTION_DEBUG = has_option("debug")
OPTION_RELWITHDEBINFO = has_option('relwithdebinfo')
OPTION_QMAKE = option_value("qmake")
OPTION_QT_VERSION = option_value("qt")
OPTION_CMAKE = option_value("cmake")
OPTION_OPENSSL = option_value("openssl")
OPTION_ONLYPACKAGE = has_option("only-package")
OPTION_STANDALONE = has_option("standalone")
OPTION_VERSION = option_value("version")
OPTION_LISTVERSIONS = has_option("list-versions")
OPTION_MAKESPEC = option_value("make-spec")
OPTION_IGNOREGIT = has_option("ignore-git")
OPTION_NOEXAMPLES = has_option("no-examples")     # don't include pyside2-examples
OPTION_JOBS = option_value('jobs')                # number of parallel build jobs
OPTION_JOM = has_option('jom')                    # Legacy, not used any more.
OPTION_NO_JOM = has_option('no-jom')              # Do not use jom instead of nmake with msvc
OPTION_BUILDTESTS = has_option("build-tests")
OPTION_OSXARCH = option_value("osx-arch")
OPTION_OSX_USE_LIBCPP = has_option("osx-use-libc++")
OPTION_OSX_SYSROOT = option_value("osx-sysroot")
OPTION_XVFB = has_option("use-xvfb")
OPTION_REUSE_BUILD = has_option("reuse-build")
OPTION_SKIP_CMAKE = has_option("skip-cmake")
OPTION_SKIP_MAKE_INSTALL = has_option("skip-make-install")
OPTION_SKIP_PACKAGING = has_option("skip-packaging")

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
if OPTION_QMAKE is not None and os.path.exists(OPTION_QMAKE): # Checking whether qmake executable exists
    if os.path.islink(OPTION_QMAKE) and os.path.lexists(OPTION_QMAKE): # Looking whether qmake path is a link and whether the link exists
        if "qtchooser" in os.readlink(OPTION_QMAKE): # Set -qt=X here.
            QMAKE_COMMAND = [OPTION_QMAKE, "-qt=%s" %(OPTION_QT_VERSION)]
if not QMAKE_COMMAND:
    QMAKE_COMMAND = [OPTION_QMAKE]

if len(QMAKE_COMMAND) == 0 or QMAKE_COMMAND[0] is None:
    print("qmake could not be found.")
    sys.exit(1)
if not os.path.exists(QMAKE_COMMAND[0]):
    print("'%s' does not exist." % QMAKE_COMMAND[0])
    sys.exit(1)
if OPTION_CMAKE is None:
    OPTION_CMAKE = find_executable("cmake")

if OPTION_CMAKE is None:
    print("cmake could not be found.")
    sys.exit(1)
if not os.path.exists(OPTION_CMAKE):
    print("'%s' does not exist." % OPTION_CMAKE)
    sys.exit(1)

if sys.platform == "win32":
    if OPTION_MAKESPEC is None:
        OPTION_MAKESPEC = "msvc"
    if not OPTION_MAKESPEC in ["msvc", "mingw"]:
        print("Invalid option --make-spec. Available values are %s" % (["msvc", "mingw"]))
        sys.exit(1)
else:
    if OPTION_MAKESPEC is None:
        OPTION_MAKESPEC = "make"
    if not OPTION_MAKESPEC in ["make"]:
        print("Invalid option --make-spec. Available values are %s" % (["make"]))
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

if sys.platform == 'darwin' and OPTION_STANDALONE:
    print("--standalone option does not yet work on OSX")


# Show available versions
if OPTION_LISTVERSIONS:
    for v in submodules:
        print("%s" % (v))
        for m in submodules[v]:
            print("  %s %s" % (m[0], m[1]))
    sys.exit(1)

# Change the cwd to our source dir
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))
script_dir = os.getcwd()

# Change package version
if OPTION_VERSION:
    if OPTION_IGNOREGIT:
        print("Option --version can not be used together with option --ignore-git")
        sys.exit(1)
    if not os.path.isdir(".git"):
        print("Option --version is available only when pyside2-setup was cloned from git repository")
        sys.exit(1)
    if not OPTION_VERSION in submodules:
        print("""Invalid version specified %s
Use --list-versions option to get list of available versions""" % OPTION_VERSION)
        sys.exit(1)
    __version__ = OPTION_VERSION

if OPTION_NOEXAMPLES:
    # remove pyside2-exampes from submodules so they will not be included
    for idx, item in enumerate(submodules[__version__]):
        if item[0].startswith('pyside2-examples'):
            del submodules[__version__][idx]

# Return a prefix suitable for the _install/_build directory
def prefix():
    virtualEnvName = os.environ.get('VIRTUAL_ENV', None)
    name = os.path.basename(virtualEnvName) if virtualEnvName is not None else 'pyside'
    name += str(sys.version_info[0])
    if OPTION_DEBUG:
        name += 'd'
    return name

# Initialize, pull and checkout submodules
def prepareSubModules():
    print("Initializing submodules for PySide2 version %s" % __version__)
    submodules_dir = os.path.join(script_dir, "sources")
    # Create list of [name, desired branch, absolute path, desired branch]
    # and determine whether all submodules are present
    needInitSubModules = False
    modulesList = []
    for m in submodules[__version__]:
        module_name = m[0]
        module_version = m[1]
        module_dir = m[2] if len(m) > 2 else ''
        module_dir = os.path.join(submodules_dir, module_dir, module_name)
        # Check for non-empty directory (repository checked out)
        if not os.listdir(module_dir):
            needInitSubModules = True
        modulesList.append([module_name, module_version, module_dir])
    if needInitSubModules:
        git_update_cmd = ["git", "submodule", "update", "--init"]
        if run_process(git_update_cmd) != 0:
            raise DistutilsSetupError("Failed to initialize the git submodules")
        git_pull_cmd = ["git", "submodule", "foreach", "git", "fetch", "--all"]
        if run_process(git_pull_cmd) != 0:
            raise DistutilsSetupError("Failed to initialize the git submodules")
    else:
        print("All submodules present...")
    # Ensure all submodules have the correct branch checked out
    for m in modulesList:
        module_name = m[0]
        module_version = m[1]
        module_dir = m[2]
        os.chdir(module_dir)
        currentBranch = ''
        branches = set()
        for line in run_process_output(['git', 'branch']):
            if line.startswith('* '):
                currentBranch = line[2:len(line)]
            else:
                branches.add(line.strip())
        if currentBranch != module_version:
            if not module_version in branches:
                print("Creating tracking branch %s for submodule %s" % \
                      (module_version, module_name))
                git_create_branch_cmd = ["git", "branch", "--track", module_version,
                                         "origin/" + module_version]
                if run_process(git_create_branch_cmd) != 0:
                    raise DistutilsSetupError("Failed to create a tracking branch %s for %s" % \
                                              (module_version, module_name))
            print("Checking out submodule %s to branch %s (from %s)" % (module_name, module_version, currentBranch))
            git_checkout_cmd = ["git", "checkout", module_version]
            if run_process(git_checkout_cmd) != 0:
                raise DistutilsSetupError("Failed to initialize the git submodule %s" % module_name)
        else:
            print("Submodule %s has branch %s checked out" % (module_name, module_version))
        os.chdir(script_dir)

def prepareBuild():
    if os.path.isdir(".git") and not OPTION_IGNOREGIT and not OPTION_ONLYPACKAGE and not OPTION_REUSE_BUILD:
        prepareSubModules()
    # Clean up temp and package folders
    for n in ["pyside_package", "build", "PySide2-%s" % __version__]:
        d = os.path.join(script_dir, n)
        if os.path.isdir(d):
            print("Removing %s" % d)
            try:
                rmtree(d)
            except Exception as e:
                print('***** problem removing "{}"'.format(d))
                print('ignored error: {}'.format(e))
    # Prepare package folders
    for pkg in ["pyside_package/PySide2", "pyside_package/pyside2uic"]:
        pkg_dir = os.path.join(script_dir, pkg)
        os.makedirs(pkg_dir)
    # locate Qt sources for the documentation
    qmakeOutput = run_process_output([OPTION_QMAKE, '-query', 'QT_INSTALL_PREFIX/src'])
    if qmakeOutput:
        global qtSrcDir
        qtSrcDir = qmakeOutput[0].rstrip()

class pyside_install(_install):
    def _init(self, *args, **kwargs):
        _install.__init__(self, *args, **kwargs)

    def run(self):
        _install.run(self)
        log.info('*** Install completed')

class pyside_develop(_develop):

    def __init__(self, *args, **kwargs):
        _develop.__init__(self, *args, **kwargs)

    def run(self):
        self.run_command("build")
        _develop.run(self)

class pyside_bdist_egg(_bdist_egg):

    def __init__(self, *args, **kwargs):
        _bdist_egg.__init__(self, *args, **kwargs)

    def run(self):
        self.run_command("build")
        _bdist_egg.run(self)

class pyside_build_ext(_build_ext):

    def __init__(self, *args, **kwargs):
        _build_ext.__init__(self, *args, **kwargs)

    def run(self):
        pass

class pyside_build(_build):

    def __init__(self, *args, **kwargs):
        _build.__init__(self, *args, **kwargs)

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
        prepareBuild()
        platform_arch = platform.architecture()[0]
        log.info("Python architecture is %s" % platform_arch)

        build_type = OPTION_DEBUG and "Debug" or "Release"
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
                    log.info("nmake not found. Trying to initialize the MSVC env...")
                    init_msvc_env(platform_arch, build_type)
                    nmake_path = find_executable("nmake")
                    assert(nmake_path is not None and os.path.exists(nmake_path))
                jom_path = None if OPTION_NO_JOM else find_executable("jom")
                if jom_path is not None and os.path.exists(jom_path):
                    log.info("jom was found in %s" % jom_path)
                    make_name = "jom"
                    make_generator = "NMake Makefiles JOM"
                else:
                    log.info("nmake was found in %s" % nmake_path)
                    make_name = "nmake"
                    make_generator = "NMake Makefiles"
            elif OPTION_MAKESPEC == "mingw":
                make_name = "mingw32-make"
                make_generator = "MinGW Makefiles"
            else:
                raise DistutilsSetupError(
                    "Invalid option --make-spec.")
            make_path = find_executable(make_name)
            if make_path is None or not os.path.exists(make_path):
                raise DistutilsSetupError(
                    "You need the program \"%s\" on your system path to compile PySide2." \
                    % make_name)

            if OPTION_CMAKE is None or not os.path.exists(OPTION_CMAKE):
                raise DistutilsSetupError(
                    "Failed to find cmake."
                    " Please specify the path to cmake with --cmake parameter.")

        if OPTION_QMAKE is None or not os.path.exists(OPTION_QMAKE):
            raise DistutilsSetupError(
                "Failed to find qmake."
                " Please specify the path to qmake with --qmake parameter.")

        # Prepare parameters
        py_executable = sys.executable
        py_version = "%s.%s" % (sys.version_info[0], sys.version_info[1])
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
                py_include_dir = os.path.join(py_prefix, "include/python%s" % py_version)
        dbgPostfix = ""
        if build_type == "Debug":
            dbgPostfix = "_d"
        if sys.platform == "win32":
            if OPTION_MAKESPEC == "mingw":
                py_library = os.path.join(py_libdir, "libpython%s%s.a" % \
                    (py_version.replace(".", ""), dbgPostfix))
            else:
                py_library = os.path.join(py_libdir, "python%s%s.lib" % \
                    (py_version.replace(".", ""), dbgPostfix))
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
            lib_exts.append('.a') # static library as last gasp

            if sys.version_info[0] == 2 and dbgPostfix:
                # For Python2 add a duplicate set of extensions combined with
                # the dbgPostfix, so we test for both the debug version of
                # the lib and the normal one. This allows a debug PySide2 to
                # be built with a non-debug Python.
                lib_exts = [dbgPostfix + e for e in lib_exts] + lib_exts

            python_library_found = False
            libs_tried = []
            for lib_ext in lib_exts:
                lib_name = "libpython%s%s%s" % (py_version, lib_suff, lib_ext)
                py_library = os.path.join(py_libdir, lib_name)
                if os.path.exists(py_library):
                    python_library_found = True
                    break
                libs_tried.append(py_library)
            else:
                # At least on macOS 10.11, the system Python 2.6 does not include a symlink
                # to the framework file disguised as a .dylib file, thus finding the library would
                # fail. Manually check if a framework file "Python" exists in the Python framework
                # bundle.
                if sys.platform == 'darwin' and sys.version_info[:2] == (2, 6):
                    # These manipulations essentially transform
                    # /System/Library/Frameworks/Python.framework/Versions/2.6/lib
                    # to /System/Library/Frameworks/Python.framework/Versions/2.6/Python
                    possible_framework_path = os.path.realpath(os.path.join(py_libdir, '..'))
                    possible_framework_version = os.path.basename(possible_framework_path)
                    possible_framework_library = os.path.join(possible_framework_path, 'Python')

                    if possible_framework_version == '2.6' \
                        and os.path.exists(possible_framework_library):
                            py_library = possible_framework_library
                            python_library_found = True
                    else:
                        libs_tried.append(possible_framework_library)

                # Try to find shared libraries which have a multi arch suffix.
                if not python_library_found:
                    py_multiarch = get_config_var("MULTIARCH")
                    if py_multiarch and not python_library_found:
                        try_py_libdir = os.path.join(py_libdir, py_multiarch)
                        libs_tried = []
                        for lib_ext in lib_exts:
                            lib_name = "libpython%s%s%s" % (py_version, lib_suff, lib_ext)
                            py_library = os.path.join(try_py_libdir, lib_name)
                            if os.path.exists(py_library):
                                py_libdir = try_py_libdir
                                python_library_found = True
                                break
                            libs_tried.append(py_library)

            if not python_library_found:
                raise DistutilsSetupError(
                    "Failed to locate the Python library with %s" %
                    ', '.join(libs_tried))

            if py_library.endswith('.a'):
                # Python was compiled as a static library
                log.error("Failed to locate a dynamic Python library, using %s"
                          % py_library)

        self.qtinfo = QtInfo(QMAKE_COMMAND)
        qt_dir = os.path.dirname(OPTION_QMAKE)
        qt_version = self.qtinfo.version
        if not qt_version:
            log.error("Failed to query the Qt version with qmake %s" % self.qtinfo.qmake_command)
            sys.exit(1)

        # Update the PATH environment variable
        update_env_path([py_scripts_dir, qt_dir])

        build_name = "py%s-qt%s-%s-%s" % \
            (py_version, qt_version, platform.architecture()[0], build_type.lower())

        script_dir = os.getcwd()
        sources_dir = os.path.join(script_dir, "sources")
        build_dir = os.path.join(script_dir, prefix() + '_build', "%s" % build_name)
        install_dir = os.path.join(script_dir, prefix() + '_install', "%s" % build_name)

        # Try to ensure that tools built by this script (such as shiboken2)
        # are found before any that may already be installed on the system.
        update_env_path([os.path.join(install_dir, 'bin')])

        # Tell cmake to look here for *.cmake files
        os.environ['CMAKE_PREFIX_PATH'] = install_dir

        self.make_path = make_path
        self.make_generator = make_generator
        self.debug = OPTION_DEBUG
        self.script_dir = script_dir
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

        log.info("=" * 30)
        log.info("Package version: %s" % __version__)
        log.info("Build type: %s" % self.build_type)
        log.info("Build tests: %s" % self.build_tests)
        log.info("-" * 3)
        log.info("Make path: %s" % self.make_path)
        log.info("Make generator: %s" % self.make_generator)
        log.info("Make jobs: %s" % OPTION_JOBS)
        log.info("-" * 3)
        log.info("Script directory: %s" % self.script_dir)
        log.info("Sources directory: %s" % self.sources_dir)
        log.info("Build directory: %s" % self.build_dir)
        log.info("Install directory: %s" % self.install_dir)
        log.info("Python site-packages install directory: %s" % self.site_packages_dir)
        log.info("-" * 3)
        log.info("Python executable: %s" % self.py_executable)
        log.info("Python includes: %s" % self.py_include_dir)
        log.info("Python library: %s" % self.py_library)
        log.info("Python prefix: %s" % py_prefix)
        log.info("Python scripts: %s" % py_scripts_dir)
        log.info("-" * 3)
        log.info("Qt qmake: %s" % self.qtinfo.qmake_command)
        log.info("Qt version: %s" % self.qtinfo.version)
        log.info("Qt bins: %s" % self.qtinfo.bins_dir)
        log.info("Qt docs: %s" % self.qtinfo.docs_dir)
        log.info("Qt plugins: %s" % self.qtinfo.plugins_dir)
        log.info("-" * 3)
        log.info("OpenSSL libs: %s" % OPTION_OPENSSL)
        log.info("=" * 30)

        # Prepare folders
        if not os.path.exists(self.sources_dir):
            log.info("Creating sources folder %s..." % self.sources_dir)
            os.makedirs(self.sources_dir)
        if not os.path.exists(self.build_dir):
            log.info("Creating build folder %s..." % self.build_dir)
            os.makedirs(self.build_dir)
        if not os.path.exists(self.install_dir):
            log.info("Creating install folder %s..." % self.install_dir)
            os.makedirs(self.install_dir)

        if not OPTION_ONLYPACKAGE:
            # Build extensions
            for ext in ['shiboken2', 'pyside2', 'pyside2-tools']:
                self.build_extension(ext)

            if OPTION_BUILDTESTS:
                # we record the latest successful build and note the build
                # directory for supporting the tests.
                timestamp = time.strftime('%Y-%m-%d_%H%M%S')
                build_history = os.path.join(script_dir, 'build_history')
                unique_dir = os.path.join(build_history, timestamp)
                os.makedirs(unique_dir)
                fpath = os.path.join(unique_dir, 'build_dir.txt')
                with open(fpath, 'w') as f:
                    print(build_dir, file=f)
                log.info("Created %s" % build_history)

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

    def build_patchelf(self):
        if not sys.platform.startswith('linux'):
            return
        log.info("Building patchelf...")
        module_src_dir = os.path.join(self.sources_dir, "patchelf")
        build_cmd = [
            "g++",
            "%s/patchelf.cc" % (module_src_dir),
            "-o",
            "patchelf",
        ]
        if run_process(build_cmd) != 0:
            raise DistutilsSetupError("Error building patchelf")

    def build_extension(self, extension):
        # calculate the subrepos folder name
        folder = get_extension_folder(extension)

        log.info("Building module %s..." % extension)

        # Prepare folders
        os.chdir(self.build_dir)
        module_build_dir = os.path.join(self.build_dir,  extension)
        skipflag_file = module_build_dir + '-skip'
        if os.path.exists(skipflag_file):
            log.info("Skipping %s because %s exists" % (extension, skipflag_file))
            return

        module_build_exists = os.path.exists(module_build_dir)
        if module_build_exists:
            if not OPTION_REUSE_BUILD:
                log.info("Deleting module build folder %s..." % module_build_dir)
                try:
                    rmtree(module_build_dir)
                except Exception as e:
                    print('***** problem removing "{}"'.format(module_build_dir))
                    print('ignored error: {}'.format(e))
            else:
                log.info("Reusing module build folder %s..." % module_build_dir)
        if not os.path.exists(module_build_dir):
            log.info("Creating module build folder %s..." % module_build_dir)
            os.makedirs(module_build_dir)
        os.chdir(module_build_dir)

        module_src_dir = os.path.join(self.sources_dir, folder)

        # Build module
        cmake_cmd = [
            OPTION_CMAKE,
            "-G", self.make_generator,
            "-DQT_QMAKE_EXECUTABLE='%s'" % self.qtinfo.qmake_command,
            "-DBUILD_TESTS=%s" % self.build_tests,
            "-DQt5Help_DIR=%s" % self.qtinfo.docs_dir,
            "-DCMAKE_BUILD_TYPE=%s" % self.build_type,
            "-DCMAKE_INSTALL_PREFIX=%s" % self.install_dir,
            module_src_dir
        ]
        cmake_cmd.append("-DPYTHON_EXECUTABLE=%s" % self.py_executable)
        cmake_cmd.append("-DPYTHON_INCLUDE_DIR=%s" % self.py_include_dir)
        cmake_cmd.append("-DPYTHON_LIBRARY=%s" % self.py_library)
        # Add source location for generating documentation
        if qtSrcDir:
            cmake_cmd.append("-DQT_SRC_DIR=%s" % qtSrcDir)
        if self.build_type.lower() == 'debug':
            cmake_cmd.append("-DPYTHON_DEBUG_LIBRARY=%s" % self.py_library)

        if extension.lower() == "shiboken2":
            cmake_cmd.append("-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=yes")
            if sys.version_info[0] > 2:
                cmake_cmd.append("-DUSE_PYTHON_VERSION=3.3")

        if sys.platform == 'darwin':
            # Shiboken supports specifying multiple include paths separated by a colon on *nix
            # systems.
            # In a framework build, two paths should be included:
            # path_to_qt/lib -> which contains framework folders with headers, and
            # path_to_qt/include -> which contains headers for static libraries.
            # A non-framework build contains all headers in the path_to_qt/include folder.
            path_separator = ":"
            includes_dir = '-DALTERNATIVE_QT_INCLUDE_DIR=' + self.qtinfo.headers_dir
            if os.path.isdir(self.qtinfo.headers_dir + "/../lib/QtCore.framework"):
                includes_dir += path_separator + self.qtinfo.headers_dir + "/../lib/"
            cmake_cmd.append(includes_dir)

            if OPTION_OSXARCH:
                # also tell cmake which architecture to use
                cmake_cmd.append("-DCMAKE_OSX_ARCHITECTURES:STRING={}".format(OPTION_OSXARCH))

            if OPTION_OSX_USE_LIBCPP:
                # Explicitly link the libc++ standard library (useful for osx deployment targets
                # lower than 10.9). This is not on by default, because most libraries and
                # executables on OSX <= 10.8 are linked to libstdc++, and mixing standard libraries
                # can lead to crashes.
                # On OSX >= 10.9 with a similar minimum deployment target, libc++ is linked in
                # implicitly, thus the option is a no-op in those cases.
                cmake_cmd.append("-DOSX_USE_LIBCPP=ON")

            if OPTION_OSX_SYSROOT:
                cmake_cmd.append("-DCMAKE_OSX_SYSROOT={0}".format(OPTION_OSX_SYSROOT))
            else:
                latest_sdk_path = run_process_output(['xcrun', '--show-sdk-path'])
                if latest_sdk_path:
                    latest_sdk_path = latest_sdk_path[0]
                    cmake_cmd.append("-DCMAKE_OSX_SYSROOT={0}".format(latest_sdk_path))


        if not OPTION_SKIP_CMAKE:
            log.info("Configuring module %s (%s)..." % (extension,  module_src_dir))
            if run_process(cmake_cmd) != 0:
                raise DistutilsSetupError("Error configuring " + extension)
        else:
            log.info("Reusing old configuration for module %s (%s)..." % (extension,
                                                                          module_src_dir))

        log.info("Compiling module %s..." % extension)
        cmd_make = [self.make_path]
        if OPTION_JOBS:
            cmd_make.append(OPTION_JOBS)
        if run_process(cmd_make) != 0:
            raise DistutilsSetupError("Error compiling " + extension)

        if extension.lower() == "shiboken2":
            log.info("Generating Shiboken documentation %s..." % extension)
            if run_process([self.make_path, "doc"]) != 0:
                raise DistutilsSetupError("Error generating documentation " + extension)

        if not OPTION_SKIP_MAKE_INSTALL:
            log.info("Installing module %s..." % extension)
            # Need to wait a second, so installed file timestamps are older than build file
            # timestamps.
            # See https://gitlab.kitware.com/cmake/cmake/issues/16155 for issue details.
            if sys.platform == 'darwin':
                log.info("Waiting 1 second, to ensure installation is successful...")
                time.sleep(1)
            if run_process([self.make_path, "install/fast"]) != 0:
                raise DistutilsSetupError("Error pseudo installing " + extension)
        else:
            log.info("Skipped installing module %s" % extension)

        os.chdir(self.script_dir)

    def prepare_packages(self):
        try:
            log.info("Preparing packages...")
            version_str = "%sqt%s%s" % (__version__, self.qtinfo.version.replace(".", "")[0:3],
                self.debug and "dbg" or "")
            vars = {
                "site_packages_dir": self.site_packages_dir,
                "sources_dir": self.sources_dir,
                "install_dir": self.install_dir,
                "build_dir": self.build_dir,
                "script_dir": self.script_dir,
                "dist_dir": os.path.join(self.script_dir, 'pyside_package'),
                "ssl_libs_dir": OPTION_OPENSSL,
                "py_version": self.py_version,
                "qt_version": self.qtinfo.version,
                "qt_bin_dir": self.qtinfo.bins_dir,
                "qt_doc_dir": self.qtinfo.docs_dir,
                "qt_lib_dir": self.qtinfo.libs_dir,
                "qt_plugins_dir": self.qtinfo.plugins_dir,
                "qt_translations_dir": self.qtinfo.translations_dir,
                "version": version_str,
            }
            os.chdir(self.script_dir)
            if sys.platform == "win32":
                vars['dbgPostfix'] = OPTION_DEBUG and "_d" or ""
                return self.prepare_packages_win32(vars)
            else:
                return self.prepare_packages_posix(vars)
        except FileNotFoundError as e:
            print('setup.py/prepare_packages: ', e)
            raise

    def prepare_packages_posix(self, vars):
        executables = []
        if sys.platform.startswith('linux'):
            so_ext = '.so'
            so_star = so_ext + '.*'
        elif sys.platform == 'darwin':
            so_ext = '.dylib'
            so_star = so_ext
        # <build>/shiboken2/doc/html/* -> <setup>/PySide2/docs/shiboken2
        copydir(
            "{build_dir}/shiboken2/doc/html",
            "{dist_dir}/PySide2/docs/shiboken2",
            force=False, vars=vars)
        # <install>/lib/site-packages/PySide2/* -> <setup>/PySide2
        copydir(
            "{site_packages_dir}/PySide2",
            "{dist_dir}/PySide2",
            vars=vars)
        # <install>/lib/site-packages/shiboken2.so -> <setup>/PySide2/shiboken2.so
        copyfile(
            "{site_packages_dir}/shiboken2.so",
            "{dist_dir}/PySide2/shiboken2.so",
            vars=vars)
        # <install>/lib/site-packages/pyside2uic/* -> <setup>/pyside2uic
        copydir(
            "{site_packages_dir}/pyside2uic",
            "{dist_dir}/pyside2uic",
            force=False, vars=vars)
        if sys.version_info[0] > 2:
            rmtree("{dist_dir}/pyside2uic/port_v2".format(**vars))
        else:
            rmtree("{dist_dir}/pyside2uic/port_v3".format(**vars))
        # <install>/bin/pyside2-uic -> PySide2/scripts/uic.py
        makefile(
            "{dist_dir}/PySide2/scripts/__init__.py",
            vars=vars)
        copyfile(
            "{install_dir}/bin/pyside2-uic",
            "{dist_dir}/PySide2/scripts/uic.py",
            force=False, vars=vars)
        # <install>/bin/* -> PySide2/
        executables.extend(copydir(
            "{install_dir}/bin/",
            "{dist_dir}/PySide2",
            filter=[
                "pyside2-lupdate",
                "pyside2-rcc",
                "shiboken2",
            ],
            recursive=False, vars=vars))
        # <install>/lib/lib* -> PySide2/
        copydir(
            "{install_dir}/lib/",
            "{dist_dir}/PySide2",
            filter=[
                "libpyside*" + so_star,
                "libshiboken*" + so_star,
            ],
            recursive=False, vars=vars)
        # <install>/share/PySide2/typesystems/* -> <setup>/PySide2/typesystems
        copydir(
            "{install_dir}/share/PySide2/typesystems",
            "{dist_dir}/PySide2/typesystems",
            vars=vars)
        # <install>/include/* -> <setup>/PySide2/include
        copydir(
            "{install_dir}/include",
            "{dist_dir}/PySide2/include",
            vars=vars)
        if not OPTION_NOEXAMPLES:
            # <sources>/pyside2-examples/examples/* -> <setup>/PySide2/examples
            folder = get_extension_folder('pyside2-examples')
            copydir(
                "{sources_dir}/%s/examples" % folder,
                "{dist_dir}/PySide2/examples",
                force=False, vars=vars)
            # Re-generate examples Qt resource files for Python 3 compatibility
            if sys.version_info[0] == 3:
                examples_path = "{dist_dir}/PySide2/examples".format(**vars)
                pyside_rcc_path = "{install_dir}/bin/pyside2-rcc".format(**vars)
                pyside_rcc_options = '-py3'
                regenerate_qt_resources(examples_path, pyside_rcc_path,
                    pyside_rcc_options)
        # Copy Qt libs to package
        if OPTION_STANDALONE:
            if sys.platform == 'darwin':
                raise RuntimeError('--standalone not yet supported for OSX')
            # <qt>/bin/* -> <setup>/PySide2
            executables.extend(copydir("{qt_bin_dir}", "{dist_dir}/PySide2",
                filter=[
                    "designer",
                    "linguist",
                    "lrelease",
                    "lupdate",
                    "lconvert",
                ],
                recursive=False, vars=vars))
            # <qt>/lib/* -> <setup>/PySide2
            copydir("{qt_lib_dir}", "{dist_dir}/PySide2",
                filter=[
                    "libQt*.so.?",
                    "libphonon.so.?",
                ],
                recursive=False, vars=vars)
            # <qt>/plugins/* -> <setup>/PySide2/plugins
            copydir("{qt_plugins_dir}", "{dist_dir}/PySide2/plugins",
                filter=["*.so"],
                vars=vars)
            # <qt>/translations/* -> <setup>/PySide2/translations
            copydir("{qt_translations_dir}", "{dist_dir}/PySide2/translations",
                filter=["*.qm"],
                vars=vars)
        # Update rpath to $ORIGIN
        if sys.platform.startswith('linux') or sys.platform.startswith('darwin'):
            self.update_rpath("{dist_dir}/PySide2".format(**vars), executables)

    def prepare_packages_win32(self, vars):
        pdbs = ['*.pdb'] if self.debug or self.build_type == 'RelWithDebInfo' else []
        # <install>/lib/site-packages/PySide2/* -> <setup>/PySide2
        copydir(
            "{site_packages_dir}/PySide2",
            "{dist_dir}/PySide2",
            vars=vars)
        if self.debug or self.build_type == 'RelWithDebInfo':
            # <build>/pyside2/PySide2/*.pdb -> <setup>/PySide2
            copydir(
                "{build_dir}/pyside2/PySide2",
                "{dist_dir}/PySide2",
                filter=pdbs,
                recursive=False, vars=vars)
        # <build>/shiboken2/doc/html/* -> <setup>/PySide2/docs/shiboken2
        copydir(
            "{build_dir}/shiboken2/doc/html",
            "{dist_dir}/PySide2/docs/shiboken2",
            force=False, vars=vars)
        # <install>/lib/site-packages/shiboken2.pyd -> <setup>/PySide2/shiboken2.pyd
        copyfile(
            "{site_packages_dir}/shiboken2{dbgPostfix}.pyd",
            "{dist_dir}/PySide2/shiboken2{dbgPostfix}.pyd",
            vars=vars)
        if self.debug or self.build_type == 'RelWithDebInfo':
            copyfile(
                "{build_dir}/shiboken2/shibokenmodule/shiboken2{dbgPostfix}.pdb",
                "{dist_dir}/PySide2/shiboken2{dbgPostfix}.pdb",
                vars=vars)
        # <install>/lib/site-packages/pyside2uic/* -> <setup>/pyside2uic
        copydir(
            "{site_packages_dir}/pyside2uic",
            "{dist_dir}/pyside2uic",
            force=False, vars=vars)
        if sys.version_info[0] > 2:
            rmtree("{dist_dir}/pyside2uic/port_v2".format(**vars))
        else:
            rmtree("{dist_dir}/pyside2uic/port_v3".format(**vars))
        # <install>/bin/pyside2-uic -> PySide2/scripts/uic.py
        makefile(
            "{dist_dir}/PySide2/scripts/__init__.py",
            vars=vars)
        copyfile(
            "{install_dir}/bin/pyside2-uic",
            "{dist_dir}/PySide2/scripts/uic.py",
            force=False, vars=vars)
        # <install>/bin/*.exe,*.dll,*.pdb -> PySide2/
        copydir(
            "{install_dir}/bin/",
            "{dist_dir}/PySide2",
            filter=["*.exe", "*.dll"] + pdbs,
            recursive=False, vars=vars)
        # <install>/lib/*.lib -> PySide2/
        copydir(
            "{install_dir}/lib/",
            "{dist_dir}/PySide2",
            filter=["*.lib"],
            recursive=False, vars=vars)
        # <install>/share/PySide2/typesystems/* -> <setup>/PySide2/typesystems
        copydir(
            "{install_dir}/share/PySide2/typesystems",
            "{dist_dir}/PySide2/typesystems",
            vars=vars)
        # <install>/include/* -> <setup>/PySide2/include
        copydir(
            "{install_dir}/include",
            "{dist_dir}/PySide2/include",
            vars=vars)
        if not OPTION_NOEXAMPLES:
            # <sources>/pyside2-examples/examples/* -> <setup>/PySide2/examples
            folder = get_extension_folder('pyside2-examples')
            copydir(
                "{sources_dir}/%s/examples" % folder,
                "{dist_dir}/PySide2/examples",
                force=False, vars=vars)
            # Re-generate examples Qt resource files for Python 3 compatibility
            if sys.version_info[0] == 3:
                examples_path = "{dist_dir}/PySide2/examples".format(**vars)
                pyside_rcc_path = "{install_dir}/bin/pyside2-rcc".format(**vars)
                pyside_rcc_options = '-py3'
                regenerate_qt_resources(examples_path, pyside_rcc_path,
                    pyside_rcc_options)
        # <ssl_libs>/* -> <setup>/PySide2/openssl
        copydir("{ssl_libs_dir}", "{dist_dir}/PySide2/openssl",
            filter=[
                "libeay32.dll",
                "ssleay32.dll"],
            force=False, vars=vars)

        # <qt>/bin/*.dll -> <setup>/PySide2
        copydir("{qt_bin_dir}", "{dist_dir}/PySide2",
            filter=[
                "*.dll",
                "designer.exe",
                "linguist.exe",
                "lrelease.exe",
                "lupdate.exe",
                "lconvert.exe"],
            ignore=["*d4.dll"],
            recursive=False, vars=vars)
        if self.debug:
            # <qt>/bin/*d4.dll -> <setup>/PySide2
            copydir("{qt_bin_dir}", "{dist_dir}/PySide2",
                filter=["*d4.dll"] + pdbs,
                recursive=False, vars=vars)

        if self.debug  or self.build_type == 'RelWithDebInfo':
            # <qt>/lib/*.pdb -> <setup>/PySide2
            copydir("{qt_lib_dir}", "{dist_dir}/PySide2",
                filter=["*.pdb"],
                recursive=False, vars=vars)

        # I think these are the qt-mobility DLLs, at least some are,
        # so let's copy them too
        # <qt>/lib/*.dll -> <setup>/PySide2
        copydir("{qt_lib_dir}", "{dist_dir}/PySide2",
            filter=["*.dll"],
            ignore=["*d?.dll"],
            recursive=False, vars=vars)
        if self.debug:
            # <qt>/lib/*d4.dll -> <setup>/PySide2
            copydir("{qt_lib_dir}", "{dist_dir}/PySide2",
                filter=["*d?.dll"],
                recursive=False, vars=vars)
        if self.debug  or self.build_type == 'RelWithDebInfo':
            # <qt>/lib/*pdb -> <setup>/PySide2
            copydir("{qt_lib_dir}", "{dist_dir}/PySide2",
                filter=pdbs,
                recursive=False, vars=vars)

        # <qt>/plugins/* -> <setup>/PySide2/plugins
        copydir("{qt_plugins_dir}", "{dist_dir}/PySide2/plugins",
            filter=["*.dll"] + pdbs,
            vars=vars)
        # <qt>/translations/* -> <setup>/PySide2/translations
        copydir("{qt_translations_dir}", "{dist_dir}/PySide2/translations",
            filter=["*.qm"],
            vars=vars)

        # pdb files for libshiboken and libpyside
        if self.debug or self.build_type == 'RelWithDebInfo':
            # XXX dbgPostfix gives problems - the structure in shiboken2/data should be re-written!
            copyfile(
                "{build_dir}/shiboken2/libshiboken/shiboken2.pdb",
                "{dist_dir}/PySide2/shiboken2.pdb", # omitted dbgPostfix
                vars=vars)
            copyfile(
                "{build_dir}/pyside2/libpyside/pyside2.pdb",
                "{dist_dir}/PySide2/pyside2.pdb", # omitted dbgPostfix
                vars=vars)

    def update_rpath(self, package_path, executables):
        if sys.platform.startswith('linux'):
            pyside_libs = [lib for lib in os.listdir(package_path) if filter_match(
                           lib, ["*.so", "*.so.*"])]

            patchelf_path = os.path.join(self.script_dir, "patchelf")

            def rpath_cmd(srcpath):
                cmd = [patchelf_path, '--set-rpath', '$ORIGIN/', srcpath]
                if run_process(cmd) != 0:
                    raise RuntimeError("Error patching rpath in " + srcpath)

        elif sys.platform == 'darwin':
            pyside_libs = [lib for lib in os.listdir(package_path) if filter_match(
                          lib, ["*.so", "*.dylib"])]
            def rpath_cmd(srcpath):
                osx_localize_libpaths(srcpath, pyside_libs, None)

        else:
            raise RuntimeError('Not configured for platform ' +
                               sys.platform)

        pyside_libs.extend(executables)

        # Update rpath in PySide2 libs
        for srcname in pyside_libs:
            srcpath = os.path.join(package_path, srcname)
            if os.path.isdir(srcpath):
                continue
            if not os.path.exists(srcpath):
                continue
            rpath_cmd(srcpath)
            print("Patched rpath to '$ORIGIN/' (Linux) or updated rpath (OS/X) in %s." % (srcpath))


try:
    with open(os.path.join(script_dir, 'README.rst')) as f:
        README = f.read()
    with open(os.path.join(script_dir, 'CHANGES.rst')) as f:
        CHANGES = f.read()
except IOError:
    README = CHANGES = ''


setup(
    name = "PySide2",
    version = __version__,
    description = ("Python bindings for the Qt cross-platform application and UI framework"),
    long_description = README + "\n\n" + CHANGES,
    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'Environment :: MacOS X',
        'Environment :: X11 Applications :: Qt',
        'Environment :: Win32 (MS Windows)',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX',
        'Operating System :: POSIX :: Linux',
        'Operating System :: Microsoft',
        'Operating System :: Microsoft :: Windows',
        'Programming Language :: C++',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Topic :: Database',
        'Topic :: Software Development',
        'Topic :: Software Development :: Code Generators',
        'Topic :: Software Development :: Libraries :: Application Frameworks',
        'Topic :: Software Development :: User Interfaces',
        'Topic :: Software Development :: Widget Sets',
    ],
    keywords = 'Qt',
    author = 'PySide2 Team',
    author_email = 'contact@pyside.org',
    url = 'http://www.pyside.org',
    download_url = 'https://download.qt-project.org/official_releases/pyside2/',
    license = 'LGPL',
    packages = ['PySide2', 'pyside2uic'],
    package_dir = {'': 'pyside_package'},
    include_package_data = True,
    zip_safe = False,
    entry_points = {
        'console_scripts': [
            'pyside2-uic = PySide2.scripts.uic:main',
        ]
    },
    cmdclass = {
        'build': pyside_build,
        'build_ext': pyside_build_ext,
        'bdist_egg': pyside_bdist_egg,
        'develop': pyside_develop,
        'install': pyside_install
    },

    # Add a bogus extension module (will never be built here since we are
    # overriding the build command to do it using cmake) so things like
    # bdist_egg will know that there are extension modules and will name the
    # dist with the full platform info.
    ext_modules = [Extension('QtCore', [])],
    ext_package = 'PySide2',
)
