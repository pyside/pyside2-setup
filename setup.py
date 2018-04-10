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
from distutils.version import LooseVersion

"""
This is a distutils setup-script for the Qt for Python project

To build PySide2 simply execute:
  python setup.py build
or
  python setup.py install
to build and install into your current Python installation.


Optionally, one can specify the location of qmake and cmake if it is
not on the current PATH with:
    --qmake=/path/to/qt/bin/qmake
and
    --cmake=/path/to/bin/cmake
respectively.

For windows, if OpenSSL support is required, it is necessary to specify
the directory with:
    --openssl=/path/to/openssl/bin

ADDITIONAL OPTIONS:

On Linux and macOS you can use the option `--standalone` to embed Qt
libraries into the PySide2 package.
The option does not affect Windows, because it is used implicitly,
i.e. all relevant DLLs have to be copied into the PySide2 package
anyway, because there is no proper rpath support on the platform.

You can use the option `--rpath=/path/to/lib/path` to specify which
rpath values should be embedded into the PySide2 modules and shared
libraries.
This overrides the automatically generated values when the option is
not specified.

You can use the option `--only-package` if you want to create more
binary packages (bdist_wheel, bdist_egg, ...) without rebuilding the
entire PySide2 every time:

e.g.:

* First, we create a bdist_wheel from a full PySide2 build:

  python setup.py bdist_wheel --qmake=c:\Qt\5.9\bin\qmake.exe
    --cmake=c:\tools\cmake\bin\cmake.exe
    --openssl=c:\libs\OpenSSL32bit\bin

* Then,  we create a bdist_egg reusing PySide2 build with option
  `--only-package`:

  python setup.py bdist_egg --only-package
    --qmake=c:\Qt\5.9\bin\qmake.exe
    --cmake=c:\tools\cmake\bin\cmake.exe
    --openssl=c:\libs\OpenSSL32bit\bin

You can use the option `--qt-conf-prefix` to pass a path relative to
the PySide2 installed package, which will be embedded into an
auto-generated `qt.conf` registered in the Qt resource system.
This path will serve as the PrefixPath for QLibraryInfo, thus allowing
to choose where Qt plugins should be loaded from.
This option overrides the usual prefix chosen by `--standalone` option,
or when building on Windows.

To temporarily disable registration of the internal `qt.conf` file, a
new environment variable called PYSIDE_DISABLE_INTERNAL_QT_CONF is
introduced.

You should assign the integer "1" to disable the internal `qt.conf`,
or "0" (or leave empty) to keep usining the internal `qt.conf` file.

DEVELOPMENT OPTIONS:

For development purposes the following options might be of use, when
using `setup.py build`:

  --reuse-build option allows recompiling only the modified sources and
        not the whole world, shortening development iteration time,
  --skip-cmake will reuse the already generated Makefiles (or
        equivalents), instead of invoking, CMake to update the
        Makefiles (note, CMake should be ran at least once to generate
        the files),
  --skip-make-install will not run make install (or equivalent) for
        each module built,
  --skip-packaging will skip creation of the python package,
  --ignore-git will skip the fetching and checkout steps for
        supermodule and all submodules.
  --verbose-build will output the compiler invocation with command line
        arguments, etc.
  --sanitize-address will build the project with address sanitizer
    enabled (Linux or macOS only).

REQUIREMENTS:

* Python: 2.7 and 3.3+ are supported
* CMake: Specify the path to cmake with `--cmake` option or add cmake
  to the system path.
* Qt: 5.9 and 5.11 are supported. Specify the path to qmake with
  `--qmake` option or add qmake to the system path.

OPTIONAL:

* OpenSSL:
    Specifying the --openssl option is only required on Windows.
    It is a no-op for other platforms.

    You can specify the location of OpenSSL DLLs with option:
        --openssl=</path/to/openssl/bin>.

    You can download OpenSSL for Windows here:
        http://slproweb.com/products/Win32OpenSSL.html (*)

    Official Qt packages do not link to the SSL library directly, but
    rather try to find the library at runtime.

    On Windows, official Qt builds will try to pick up OpenSSL
    libraries at application path, system registry, or in the PATH
    environment variable.

    On macOS, official Qt builds use SecureTransport (provided by OS)
    instead of OpenSSL.

    On Linux, official Qt builds will try to pick up the system OpenSSL
    library.

    Note: this means that Qt packages that directly link to the OpenSSL
          shared libraries, are not currently compatible with
          standalone PySide2 packages.

    (*) Revised on 23.03.2018

* macOS SDK:
    You can specify which macOS SDK should be used for compilation with
    the option:
        --osx-sysroot=</path/to/sdk>.

    e.g.: "--osx-sysroot=/Applications/.../Developer/SDKs/MacOSX10.11.sdk/"

* macOS minimum deployment target:
    You can specify a custom macOS minimum deployment target with the
    option:
        --osx-deployment-target=<value>

    e.g.: "--osx-deployment-target=10.10"

    If the option is not set, the minimum deployment target of the used
    Qt library will be used instead. Thus it is not necessary to use
    the option without a good reason.

    If a new value is specified, it has to be higher or equal to both
    Python's and Qt's minimum deployment targets.

    Description: macOS allows specifying a minimum OS version on which
                 a binary will be able to run. This implies that an
                 application can be built on a machine with the latest
                 macOS version installed, with latest Xcode version and
                 SDK version and the built application can still run on
                 an older OS version.
"""

import os
import time
from utils import memoize, has_option, get_python_dict
OPTION_SNAPSHOT_BUILD = has_option("snapshot-build")

# Change the cwd to setup.py's dir.
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))
script_dir = os.getcwd()

@memoize
def get_package_timestamp():
    return int(time.time())

@memoize
def get_package_version():
    """ Returns the version string for the PySide2 package. """
    pyside_version_py = os.path.join(
        script_dir, "sources", "pyside2", "pyside_version.py")
    d = get_python_dict(pyside_version_py)

    final_version = "{}.{}.{}".format(
        d['major_version'], d['minor_version'], d['patch_version'])
    pre_release_version_type = d['pre_release_version_type']
    pre_release_version = d['pre_release_version']
    if pre_release_version and pre_release_version:
        final_version += pre_release_version_type + pre_release_version

    # Add the current timestamp to the version number, to suggest it
    # is a development snapshot build.
    if OPTION_SNAPSHOT_BUILD:
        final_version += ".dev{}".format(get_package_timestamp())
    return final_version

# The __version__ variable is just for PEP compliancy, and shouldn't be
# used as a value source.
__version__ = get_package_version()

# Buildable extensions.
containedModules = ['shiboken2', 'pyside2', 'pyside2-tools']

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

wheel_module_exists = False
try:
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel
    from wheel.bdist_wheel import safer_name as _safer_name
    wheel_module_exists = True
except ImportError:
    pass

from qtinfo import QtInfo
from utils import rmtree, detectClang
from utils import makefile
from utils import copyfile
from utils import copydir
from utils import run_process_output, run_process
from utils import option_value
from utils import update_env_path
from utils import init_msvc_env
from utils import regenerate_qt_resources
from utils import filter_match
from utils import osx_fix_rpaths_for_library
from utils import copy_icu_libs
from utils import find_files_using_glob

from textwrap import dedent

# make sure that setup.py is run with an allowed python version
def check_allowed_python_version():
    import re
    pattern = "'Programming Language :: Python :: (\d+)\.(\d+)'"
    supported = []
    with open(this_file) as setup:
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
OPTION_VERSION = option_value("version") # Deprecated
OPTION_LISTVERSIONS = has_option("list-versions") # Deprecated
OPTION_MAKESPEC = option_value("make-spec")
OPTION_IGNOREGIT = has_option("ignore-git")
# don't include pyside2-examples
OPTION_NOEXAMPLES = has_option("no-examples")
# number of parallel build jobs
OPTION_JOBS = option_value('jobs')
# Legacy, not used any more.
OPTION_JOM = has_option('jom')
# Do not use jom instead of nmake with msvc
OPTION_NO_JOM = has_option('no-jom')
OPTION_BUILDTESTS = has_option("build-tests")
OPTION_OSXARCH = option_value("osx-arch")
OPTION_OSX_USE_LIBCPP = has_option("osx-use-libc++")
OPTION_OSX_SYSROOT = option_value("osx-sysroot")
OPTION_OSX_DEPLOYMENT_TARGET = option_value("osx-deployment-target")
OPTION_XVFB = has_option("use-xvfb")
OPTION_REUSE_BUILD = has_option("reuse-build")
OPTION_SKIP_CMAKE = has_option("skip-cmake")
OPTION_SKIP_MAKE_INSTALL = has_option("skip-make-install")
OPTION_SKIP_PACKAGING = has_option("skip-packaging")
OPTION_MODULE_SUBSET = option_value("module-subset")
OPTION_RPATH_VALUES = option_value("rpath")
OPTION_QT_CONF_PREFIX = option_value("qt-conf-prefix")
OPTION_QT_SRC = option_value("qt-src-dir")
OPTION_ICULIB = option_value("iculib-url") # Deprecated
OPTION_VERBOSE_BUILD = has_option("verbose-build")
OPTION_SANITIZE_ADDRESS = has_option("sanitize-address")

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

if OPTION_ICULIB:
    if not OPTION_STANDALONE:
        print("--iculib-url "
            "option is a no-op option and will be removed soon.")

def is_debug_python():
    return getattr(sys, "gettotalrefcount", None) is not None

# Return a prefix suitable for the _install/_build directory
def prefix():
    virtualEnvName = os.environ.get('VIRTUAL_ENV', None)
    if virtualEnvName is not None:
        name = os.path.basename(virtualEnvName)
    else:
        name = "pyside"
    name += str(sys.version_info[0])
    if OPTION_DEBUG:
        name += 'd'
    if is_debug_python():
        name += 'p'
    return name

# Initialize, pull and checkout submodules
def prepareSubModules():
    print("Initializing submodules for PySide2 version: {}".format(
        get_package_version()))
    submodules_dir = os.path.join(script_dir, "sources")

    # Create list of [name, desired branch, absolute path, desired
    # branch] and determine whether all submodules are present
    needInitSubModules = False

    for m in submodules:
        module_name = m[0]
        module_dir = m[1] if len(m) > 1 else ''
        module_dir = os.path.join(submodules_dir, module_dir, module_name)
        # Check for non-empty directory (repository checked out)
        if not os.listdir(module_dir):
            needInitSubModules = True
            break

    if needInitSubModules:
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

def prepareBuild():
    if (os.path.isdir(".git") and not OPTION_IGNOREGIT and
            not OPTION_ONLYPACKAGE and not OPTION_REUSE_BUILD):
        prepareSubModules()
    # Clean up temp and package folders
    for n in [pyside_package_dir_name, "build"]:
        d = os.path.join(script_dir, n)
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
        pkg_dir = os.path.join(script_dir, pkg)
        os.makedirs(pkg_dir)
    # locate Qt sources for the documentation
    if OPTION_QT_SRC is None:
        installPrefix = qtinfo.prefix_dir
        if installPrefix:
            global qtSrcDir
            # In-source, developer build
            if installPrefix.endswith("qtbase"):
                qtSrcDir = installPrefix
            else: # SDK: Use 'Src' directory
                qtSrcDir = os.path.join(os.path.dirname(installPrefix),
                    'Src', 'qtbase')

class pyside_install(_install):
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

if wheel_module_exists:
    class pyside_build_wheel(_bdist_wheel):
        def __init__(self, *args, **kwargs):
            _bdist_wheel.__init__(self, *args, **kwargs)

        @property
        def wheel_dist_name(self):
            # Slightly modified version of wheel's wheel_dist_name
            # method, to add the Qt version as well.
            # Example:
            #   PySide2-5.6-5.6.4-cp27-cp27m-macosx_10_10_intel.whl
            # The PySide2 version is "5.6".
            # The Qt version built against is "5.6.4".
            qt_version = get_qt_version()
            package_version = get_package_version()
            wheel_version = "{}-{}".format(package_version, qt_version)
            components = (_safer_name(self.distribution.get_name()),
                wheel_version)
            if self.build_number:
                components += (self.build_number,)
            return '-'.join(components)

        def finalize_options(self):
            if sys.platform == 'darwin':
                # Override the platform name to contain the correct
                # minimum deployment target.
                # This is used in the final wheel name.
                self.plat_name = pyside_build.macos_plat_name()
            _bdist_wheel.finalize_options(self)

# pyside_build_py and pyside_install_lib are reimplemented to preserve
# symlinks when distutils / setuptools copy files to various
# directories through the different build stages.
class pyside_build_py(_build_py):

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

class pyside_install_lib(_install_lib):

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

class pyside_build(_build):

    def __init__(self, *args, **kwargs):
        _build.__init__(self, *args, **kwargs)

    def finalize_options(self):
        os_name_backup = os.name
        if sys.platform == 'darwin':
            self.plat_name = pyside_build.macos_plat_name()
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
        prepareBuild()
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
        dbgPostfix = ""
        if build_type == "Debug":
            dbgPostfix = "_d"
        if sys.platform == "win32":
            if OPTION_MAKESPEC == "mingw":
                static_lib_name = "libpython{}{}.a".format(
                    py_version.replace(".", ""), dbgPostfix)
                py_library = os.path.join(py_libdir, static_lib_name)
            else:
                python_lib_name = "python{}{}.lib".format(
                    py_version.replace(".", ""), dbgPostfix)
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

            if sys.version_info[0] == 2 and dbgPostfix:
                # For Python2 add a duplicate set of extensions
                # combined with the dbgPostfix, so we test for both the
                # debug version of the lib and the normal one.
                # This allows a debug PySide2 to be built with a
                # non-debug Python.
                lib_exts = [dbgPostfix + e for e in lib_exts] + lib_exts

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
        additionalPaths = [py_scripts_dir, qt_dir]

        # Add Clang to path for Windows.
        # Revisit once Clang is bundled with Qt.
        if (sys.platform == "win32" and
                LooseVersion(self.qtinfo.version) >= LooseVersion("5.7.0")):
            clangDir = detectClang()
            if clangDir[0]:
                clangBinDir = os.path.join(clangDir[0], 'bin')
                if not clangBinDir in os.environ.get('PATH'):
                    log.info("Adding {} as detected by {} to PATH".format(
                        clangBinDir, clangDir[1]))
                    additionalPaths.append(clangBinDir)
            else:
                raise DistutilsSetupError("Failed to detect Clang when checking "
                                          "LLVM_INSTALL_DIR, CLANG_INSTALL_DIR, llvm-config")

        update_env_path(additionalPaths)

        build_name = "py{}-qt{}-{}-{}".format(py_version, qt_version,
            platform.architecture()[0], build_type.lower())

        script_dir = os.getcwd()
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

        log.info("Script directory: {}".format(self.script_dir))
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
                pyside_build.macos_pyside_min_deployment_target()
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
            for ext in containedModules:
                self.build_extension(ext)

            if OPTION_BUILDTESTS:
                # we record the latest successful build and note the
                # build directory for supporting the tests.
                timestamp = time.strftime('%Y-%m-%d_%H%M%S')
                build_history = os.path.join(script_dir, 'build_history')
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
        qt_target = pyside_build.macos_qt_min_deployment_target()
        setup_target = OPTION_OSX_DEPLOYMENT_TARGET

        qt_target_split = [int(x) for x in qt_target.split('.')]
        if python_target:
            python_target_split = [int(x) for x in python_target.split('.')]
        if setup_target:
            setup_target_split = [int(x) for x in setup_target.split('.')]

        message = ("Can't set MACOSX_DEPLOYMENT_TARGET value to {} because "
                  "{} was built with minimum deployment target set to {}.")
        # setup.py provided OPTION_OSX_DEPLOYMENT_TARGET value takes
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
        deployment_target = pyside_build.macos_pyside_min_deployment_target()
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
            moduleSubSet = ''
            for m in OPTION_MODULE_SUBSET.split(','):
                if m.startswith('Qt'):
                    m = m[2:]
                if moduleSubSet:
                    moduleSubSet += ';'
                moduleSubSet += m
            cmake_cmd.append("-DMODULES={}".format(moduleSubSet))
        # Add source location for generating documentation
        cmake_src_dir = OPTION_QT_SRC if OPTION_QT_SRC else qtSrcDir
        cmake_cmd.append("-DQT_SRC_DIR={}".format(cmake_src_dir))
        log.info("Qt Source dir: {}".format(cmake_src_dir))

        if self.build_type.lower() == 'debug':
            cmake_cmd.append("-DPYTHON_DEBUG_LIBRARY={}".format(
                self.py_library))

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
            if OPTION_OSXARCH:
                # also tell cmake which architecture to use
                cmake_cmd.append("-DCMAKE_OSX_ARCHITECTURES:STRING={}".format(
                    OPTION_OSXARCH))

            if OPTION_OSX_USE_LIBCPP:
                # Explicitly link the libc++ standard library (useful
                # for macOS deployment targets lower than 10.9).
                # This is not on by default, because most libraries and
                # executables on macOS <= 10.8 are linked to libstdc++,
                # and mixing standard libraries can lead to crashes.
                # On macOS >= 10.9 with a similar minimum deployment
                # target, libc++ is linked in implicitly, thus the
                # option is a no-op in those cases.
                cmake_cmd.append("-DOSX_USE_LIBCPP=ON")

            if OPTION_OSX_SYSROOT:
                cmake_cmd.append("-DCMAKE_OSX_SYSROOT={}".format(
                    OPTION_OSX_SYSROOT))
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
                pyside_build.macos_pyside_min_deployment_target()
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

        if extension.lower() == "shiboken2":
            log.info("Generating Shiboken documentation {}...".format(
                extension))
            if run_process([self.make_path, "doc"]) != 0:
                raise DistutilsSetupError("Error generating documentation "
                    "{}".format(extension))

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
                vars['dbgPostfix'] = OPTION_DEBUG and "_d" or ""
                return self.prepare_packages_win32(vars)
            else:
                return self.prepare_packages_posix(vars)
        except IOError as e:
            print('setup.py/prepare_packages: ', e)
            raise

    def get_built_pyside_config(self, vars):
        # Get config that contains list of built modules, and
        # SOVERSIONs of the built libraries.
        pyside_package_dir = vars['pyside_package_dir']
        config_path = os.path.join(pyside_package_dir, "PySide2", "_config.py")
        config = get_python_dict(config_path)
        return config

    def prepare_packages_posix(self, vars):
        executables = []
        # <build>/shiboken2/doc/html/* ->
        #   <setup>/PySide2/docs/shiboken2
        copydir(
            "{build_dir}/shiboken2/doc/html",
            "{pyside_package_dir}/PySide2/docs/shiboken2",
            force=False, vars=vars)
        # <install>/lib/site-packages/PySide2/* -> <setup>/PySide2
        copydir(
            "{site_packages_dir}/PySide2",
            "{pyside_package_dir}/PySide2",
            vars=vars)
        # <install>/lib/site-packages/shiboken2.so ->
        #   <setup>/PySide2/shiboken2.so
        shiboken_module_name = 'shiboken2.so'
        shiboken_src_path = "{site_packages_dir}".format(**vars)
        maybe_shiboken_names = [f for f in os.listdir(shiboken_src_path)
                                if re.match(r'shiboken.*\.so', f)]
        if maybe_shiboken_names:
            shiboken_module_name = maybe_shiboken_names[0]
        vars.update({'shiboken_module_name': shiboken_module_name})
        copyfile(
            "{site_packages_dir}/{shiboken_module_name}",
            "{pyside_package_dir}/PySide2/{shiboken_module_name}",
            vars=vars)
        # <install>/lib/site-packages/pyside2uic/* ->
        #   <setup>/pyside2uic
        copydir(
            "{site_packages_dir}/pyside2uic",
            "{pyside_package_dir}/pyside2uic",
            force=False, vars=vars)
        if sys.version_info[0] > 2:
            rmtree("{pyside_package_dir}/pyside2uic/port_v2".format(**vars))
        else:
            rmtree("{pyside_package_dir}/pyside2uic/port_v3".format(**vars))
        # <install>/bin/pyside2-uic -> PySide2/scripts/uic.py
        makefile(
            "{pyside_package_dir}/PySide2/scripts/__init__.py",
            vars=vars)
        copyfile(
            "{install_dir}/bin/pyside2-uic",
            "{pyside_package_dir}/PySide2/scripts/uic.py",
            force=False, vars=vars)
        # <install>/bin/* -> PySide2/
        executables.extend(copydir(
            "{install_dir}/bin/",
            "{pyside_package_dir}/PySide2",
            filter=[
                "pyside2-lupdate",
                "pyside2-rcc",
                "shiboken2",
            ],
            recursive=False, vars=vars))
        # <install>/lib/lib* -> PySide2/
        config = self.get_built_pyside_config(vars)
        def adjusted_lib_name(name, version):
            postfix = ''
            if sys.platform.startswith('linux'):
                postfix = '.so.' + version
            elif sys.platform == 'darwin':
                postfix = '.' + version + '.dylib'
            return name + postfix
        copydir(
            "{install_dir}/lib/",
            "{pyside_package_dir}/PySide2",
            filter=[
                adjusted_lib_name("libpyside*",
                    config['pyside_library_soversion']),
                adjusted_lib_name("libshiboken*",
                    config['shiboken_library_soversion']),
            ],
            recursive=False, vars=vars, force_copy_symlinks=True)
        # <install>/share/PySide2/typesystems/* ->
        #   <setup>/PySide2/typesystems
        copydir(
            "{install_dir}/share/PySide2/typesystems",
            "{pyside_package_dir}/PySide2/typesystems",
            vars=vars)
        # <install>/include/* -> <setup>/PySide2/include
        copydir(
            "{install_dir}/include",
            "{pyside_package_dir}/PySide2/include",
            vars=vars)
        # <source>/pyside2/PySide2/support/* ->
        #   <setup>/PySide2/support/*
        copydir(
            "{build_dir}/pyside2/PySide2/support",
            "{pyside_package_dir}/PySide2/support",
            vars=vars)
        if not OPTION_NOEXAMPLES:
            # examples/* -> <setup>/PySide2/examples
            copydir(os.path.join(self.script_dir, "examples"),
                    "{pyside_package_dir}/PySide2/examples",
                    force=False, vars=vars)
            # Re-generate examples Qt resource files for Python 3
            # compatibility
            if sys.version_info[0] == 3:
                examples_path = "{pyside_package_dir}/PySide2/examples".format(
                    **vars)
                pyside_rcc_path = "{install_dir}/bin/pyside2-rcc".format(
                    **vars)
                pyside_rcc_options = '-py3'
                regenerate_qt_resources(examples_path, pyside_rcc_path,
                    pyside_rcc_options)
        # Copy Qt libs to package
        if OPTION_STANDALONE:
            vars['built_modules'] = config['built_modules']
            if sys.platform == 'darwin':
                self.prepare_standalone_package_osx(executables, vars)
            else:
                self.prepare_standalone_package_linux(executables, vars)

            # Copy over clang before rpath patching.
            self.prepare_standalone_clang(is_win=False)

        # Update rpath to $ORIGIN
        if (sys.platform.startswith('linux') or
                sys.platform.startswith('darwin')):
            self.update_rpath("{pyside_package_dir}/PySide2".format(**vars),
                executables)

    def qt_is_framework_build(self):
        if os.path.isdir(self.qtinfo.headers_dir + "/../lib/QtCore.framework"):
            return True
        return False

    def prepare_standalone_package_linux(self, executables, vars):
        built_modules = vars['built_modules']

        # <qt>/lib/* -> <setup>/PySide2/Qt/lib
        destination_lib_dir = "{pyside_package_dir}/PySide2/Qt/lib"
        copydir("{qt_lib_dir}", destination_lib_dir,
            filter=[
                "libQt5*.so.?",
                "libicu*.so.??",
            ],
            recursive=False, vars=vars, force_copy_symlinks=True)

        # Check if ICU libraries were copied over to the destination
        # Qt libdir.
        resolved_destination_lib_dir = destination_lib_dir.format(**vars)
        maybe_icu_libs = find_files_using_glob(resolved_destination_lib_dir,
            "libicu*")

        # If no ICU libraries are present in the Qt libdir (like when
        # Qt is built against system ICU, or in the Coin CI where ICU
        # libs are in a different directory) try to find out / resolve
        # which ICU libs are used by QtCore (if used at all) using a
        # custom written ldd, and copy the ICU libs to the Pyside Qt
        # dir if necessary. We choose the QtCore lib to inspect, by
        # checking which QtCore library the shiboken2 executable uses.
        if not maybe_icu_libs:
            copy_icu_libs(resolved_destination_lib_dir)

        if 'WebEngineWidgets' in built_modules:
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

        # <qt>/plugins/* -> <setup>/PySide2/Qt/plugins
        copydir("{qt_plugins_dir}",
            "{pyside_package_dir}/PySide2/Qt/plugins",
            filter=["*.so"],
            recursive=True,
            vars=vars)

        # <qt>/qml/* -> <setup>/PySide2/Qt/qml
        copydir("{qt_qml_dir}",
            "{pyside_package_dir}/PySide2/Qt/qml",
            filter=None,
            force=False,
            recursive=True,
            vars=vars)

        # <qt>/translations/* -> <setup>/PySide2/Qt/translations

        copydir("{qt_translations_dir}",
            "{pyside_package_dir}/PySide2/Qt/translations",
            filter=["*.qm", "*.pak"],
            force=False,
            vars=vars)

    def prepare_standalone_package_osx(self, executables, vars):
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
                    if (dir_name.startswith('QtWebEngine') and
                            'QtWebEngineWidgets.framework' not in
                                framework_built_modules):
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
            if 'QtWebEngineWidgets.framework' in framework_built_modules:
                qt_lib_path = "{pyside_package_dir}/PySide2/Qt/lib".format(
                    **vars)
                bundle = "QtWebEngineCore.framework/Helpers/"
                bundle += "QtWebEngineProcess.app"
                binary = "Contents/MacOS/QtWebEngineProcess"
                webengine_process_path = os.path.join(bundle, binary)
                final_path = os.path.join(qt_lib_path, webengine_process_path)
                rpath = "@loader_path/../../../../../"
                osx_fix_rpaths_for_library(final_path, rpath)
        else:
            ignored_modules = []
            if 'WebEngineWidgets' not in built_modules:
                ignored_modules.extend(['libQt5WebEngine*.dylib'])
            if 'WebKit' not in built_modules:
                ignored_modules.extend(['libQt5WebKit*.dylib'])
            accepted_modules = ['libQt5*.5.dylib']

            copydir("{qt_lib_dir}",
                "{pyside_package_dir}/PySide2/Qt/lib",
                filter=accepted_modules,
                ignore=ignored_modules,
                recursive=True, vars=vars, force_copy_symlinks=True)

            if 'WebEngineWidgets' in built_modules:
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
                osx_fix_rpaths_for_library(final_path, rpath)

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

    def prepare_packages_win32(self, vars):
        # For now, debug symbols will not be shipped into the package.
        copy_pdbs = False
        pdbs = []
        if (self.debug or self.build_type == 'RelWithDebInfo') and copy_pdbs:
            pdbs = ['*.pdb']
        # <install>/lib/site-packages/PySide2/* -> <setup>/PySide2
        copydir(
            "{site_packages_dir}/PySide2",
            "{pyside_package_dir}/PySide2",
            vars=vars)
        built_modules = self.get_built_pyside_config(vars)['built_modules']

        # <build>/pyside2/PySide2/*.pdb -> <setup>/PySide2
        copydir(
            "{build_dir}/pyside2/PySide2",
            "{pyside_package_dir}/PySide2",
            filter=pdbs,
            recursive=False, vars=vars)

        # <build>/shiboken2/doc/html/* ->
        #   <setup>/PySide2/docs/shiboken2
        copydir(
            "{build_dir}/shiboken2/doc/html",
            "{pyside_package_dir}/PySide2/docs/shiboken2",
            force=False, vars=vars)

        # <install>/lib/site-packages/shiboken2.pyd ->
        #   <setup>/PySide2/shiboken2.pyd
        shiboken_module_name = 'shiboken2.pyd'
        shiboken_src_path = "{site_packages_dir}".format(**vars)
        maybe_shiboken_names = [f for f in os.listdir(shiboken_src_path)
                                if re.match(r'shiboken.*\.pyd', f)]
        if maybe_shiboken_names:
            shiboken_module_name = maybe_shiboken_names[0]
        vars.update({'shiboken_module_name': shiboken_module_name})
        copyfile(
            "{site_packages_dir}/{shiboken_module_name}",
            "{pyside_package_dir}/PySide2/{shiboken_module_name}",
            vars=vars)
        # @TODO: Fix this .pdb file not to overwrite release
        # {shibokengenerator}.pdb file.
        # Task-number: PYSIDE-615
        copydir(
            "{build_dir}/shiboken2/shibokenmodule",
            "{pyside_package_dir}/PySide2",
            filter=pdbs,
            recursive=False, vars=vars)

        # <install>/lib/site-packages/pyside2uic/* ->
        #   <setup>/pyside2uic
        copydir(
            "{site_packages_dir}/pyside2uic",
            "{pyside_package_dir}/pyside2uic",
            force=False, vars=vars)
        if sys.version_info[0] > 2:
            rmtree("{pyside_package_dir}/pyside2uic/port_v2".format(**vars))
        else:
            rmtree("{pyside_package_dir}/pyside2uic/port_v3".format(**vars))

        # <install>/bin/pyside2-uic -> PySide2/scripts/uic.py
        makefile(
            "{pyside_package_dir}/PySide2/scripts/__init__.py",
            vars=vars)
        copyfile(
            "{install_dir}/bin/pyside2-uic",
            "{pyside_package_dir}/PySide2/scripts/uic.py",
            force=False, vars=vars)

        # <install>/bin/*.exe,*.dll,*.pdb -> PySide2/
        copydir(
            "{install_dir}/bin/",
            "{pyside_package_dir}/PySide2",
            filter=["*.exe", "*.dll"],
            recursive=False, vars=vars)
        # @TODO: Fix this .pdb file not to overwrite release
        # {shibokenmodule}.pdb file.
        # Task-number: PYSIDE-615
        copydir(
            "{build_dir}/shiboken2/generator",
            "{pyside_package_dir}/PySide2",
            filter=pdbs,
            recursive=False, vars=vars)

        # <install>/lib/*.lib -> PySide2/
        copydir(
            "{install_dir}/lib/",
            "{pyside_package_dir}/PySide2",
            filter=["*.lib"],
            recursive=False, vars=vars)

        # <install>/share/PySide2/typesystems/* ->
        #   <setup>/PySide2/typesystems
        copydir(
            "{install_dir}/share/PySide2/typesystems",
            "{pyside_package_dir}/PySide2/typesystems",
            vars=vars)

        # <install>/include/* -> <setup>/PySide2/include
        copydir(
            "{install_dir}/include",
            "{pyside_package_dir}/PySide2/include",
            vars=vars)

        # <source>/pyside2/PySide2/support/* ->
        #   <setup>/PySide2/support/*
        copydir(
            "{build_dir}/pyside2/PySide2/support",
            "{pyside_package_dir}/PySide2/support",
            vars=vars)

        if not OPTION_NOEXAMPLES:
            # examples/* -> <setup>/PySide2/examples
            copydir(os.path.join(self.script_dir, "examples"),
                    "{pyside_package_dir}/PySide2/examples",
                    force=False, vars=vars)
            # Re-generate examples Qt resource files for Python 3
            # compatibility
            if sys.version_info[0] == 3:
                examples_path = "{pyside_package_dir}/PySide2/examples".format(
                    **vars)
                pyside_rcc_path = "{install_dir}/bin/pyside2-rcc".format(
                    **vars)
                pyside_rcc_options = '-py3'
                regenerate_qt_resources(examples_path, pyside_rcc_path,
                    pyside_rcc_options)

        # <ssl_libs>/* -> <setup>/PySide2/openssl
        copydir("{ssl_libs_dir}", "{pyside_package_dir}/PySide2/openssl",
            filter=[
                "libeay32.dll",
                "ssleay32.dll"],
            force=False, vars=vars)

        # <qt>/bin/*.dll and Qt *.exe -> <setup>/PySide2
        qt_artifacts_permanent = [
            "opengl*.dll",
            "d3d*.dll",
            "libEGL*.dll",
            "libGLESv2*.dll",
            "designer.exe",
            "linguist.exe",
            "lrelease.exe",
            "lupdate.exe",
            "lconvert.exe",
            "qtdiag.exe"
        ]
        copydir("{qt_bin_dir}", "{pyside_package_dir}/PySide2",
            filter=qt_artifacts_permanent,
            recursive=False, vars=vars)

        # <qt>/bin/*.dll and Qt *.pdbs -> <setup>/PySide2 part two
        # File filter to copy only debug or only release files.
        qt_dll_patterns = ["Qt5*{}.dll", "lib*{}.dll"]
        if copy_pdbs:
            qt_dll_patterns += ["Qt5*{}.pdb", "lib*{}.pdb"]
        def qt_build_config_filter(patterns, file_name, file_full_path):
            release = [a.format('') for a in patterns]
            debug = [a.format('d') for a in patterns]

            # If qt is not a debug_and_release build, that means there
            # is only one set of shared libraries, so we can just copy
            # them.
            if qtinfo.build_type != 'debug_and_release':
                if filter_match(file_name, release):
                    return True
                return False

            # In debug_and_release case, choosing which files to copy
            # is more difficult. We want to copy only the files that
            # match the PySide2 build type. So if PySide2 is built in
            # debug mode, we want to copy only Qt debug libraries
            # (ending with "d.dll"). Or vice versa. The problem is that
            # some libraries have "d" as the last character of the
            # actual library name (for example Qt5Gamepad.dll and
            # Qt5Gamepadd.dll). So we can't just match a pattern ending
            # in "d". Instead we check if there exists a file with the
            # same name plus an additional "d" at the end, and using
            # that information we can judge if the currently processed
            # file is a debug or release file.

            # e.g. ["Qt5Cored", ".dll"]
            file_split = os.path.splitext(file_name)
            file_base_name = file_split[0]
            file_ext = file_split[1]
            # e.g. "/home/work/qt/qtbase/bin"
            file_path_dir_name = os.path.dirname(file_full_path)
            # e.g. "Qt5Coredd"
            maybe_debug_name = file_base_name + 'd'
            if self.debug:
                filter = debug
                def predicate(path): return not os.path.exists(path)
            else:
                filter = release
                def predicate(path): return os.path.exists(path)
            # e.g. "/home/work/qt/qtbase/bin/Qt5Coredd.dll"
            other_config_path = os.path.join(file_path_dir_name,
                maybe_debug_name + file_ext)

            if (filter_match(file_name, filter) and
                    predicate(other_config_path)):
                return True
            return False

        qt_dll_filter = functools.partial(qt_build_config_filter,
            qt_dll_patterns)
        copydir("{qt_bin_dir}", "{pyside_package_dir}/PySide2",
            file_filter_function=qt_dll_filter,
            recursive=False, vars=vars)

        # <qt>/plugins/* -> <setup>/PySide2/plugins
        plugin_dll_patterns = ["*{}.dll"]
        pdb_pattern = "*{}.pdb"
        if copy_pdbs:
            plugin_dll_patterns += [pdb_pattern]
        plugin_dll_filter = functools.partial(qt_build_config_filter,
            plugin_dll_patterns)
        copydir("{qt_plugins_dir}", "{pyside_package_dir}/PySide2/plugins",
            file_filter_function=plugin_dll_filter,
            vars=vars)

        # <qt>/translations/* -> <setup>/PySide2/translations
        copydir("{qt_translations_dir}",
            "{pyside_package_dir}/PySide2/translations",
            filter=["*.qm", "*.pak"],
            force=False,
            vars=vars)

        # <qt>/qml/* -> <setup>/PySide2/qml
        qml_dll_patterns = ["*{}.dll"]
        qml_ignore_patterns = qml_dll_patterns + [pdb_pattern]
        # Remove the "{}" from the patterns
        qml_ignore = [a.format('') for a in qml_ignore_patterns]
        if copy_pdbs:
            qml_dll_patterns += [pdb_pattern]
        qml_ignore = [a.format('') for a in qml_dll_patterns]
        qml_dll_filter = functools.partial(qt_build_config_filter,
            qml_dll_patterns)
        copydir("{qt_qml_dir}", "{pyside_package_dir}/PySide2/qml",
            ignore=qml_ignore,
            force=False,
            recursive=True,
            vars=vars)
        copydir("{qt_qml_dir}", "{pyside_package_dir}/PySide2/qml",
            file_filter_function=qml_dll_filter,
            force=False,
            recursive=True,
            vars=vars)

        if 'WebEngineWidgets' in built_modules:
            copydir("{qt_prefix_dir}/resources",
                "{pyside_package_dir}/PySide2/resources",
                filter=None,
                recursive=False,
                vars=vars)

            filter = 'QtWebEngineProcess{}.exe'.format(
                'd' if self.debug else '')
            copydir("{qt_bin_dir}",
                "{pyside_package_dir}/PySide2",
                filter=[filter],
                recursive=False, vars=vars)

        self.prepare_standalone_clang(is_win=True)

        # pdb files for libshiboken and libpyside
        copydir(
            "{build_dir}/shiboken2/libshiboken",
            "{pyside_package_dir}/PySide2",
            filter=pdbs,
            recursive=False, vars=vars)
        copydir(
            "{build_dir}/pyside2/libpyside",
            "{pyside_package_dir}/PySide2",
            filter=pdbs,
            recursive=False, vars=vars)

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
            raise RuntimeError("Could not finding location of libclang "
                "library from CMake cache.")

        if is_win:
            # clang_lib_path points to the static import library
            # (lib/libclang.lib), whereas we want to copy the shared
            # library (bin/libclang.dll).
            clang_lib_path = re.sub(r'lib/libclang.lib$', 'bin/libclang.dll',
                clang_lib_path)

        # Path to directory containing clang.
        clang_lib_dir = os.path.dirname(clang_lib_path)

        # The name of the clang file found by CMake.
        basename = os.path.basename(clang_lib_path)

        # We want to copy the library and all the symlinks for now,
        # thus the wildcard.
        clang_filter = basename + "*"

        # Destination is the package folder near the other extension
        # modules.
        destination_dir = "{}/PySide2".format(os.path.join(self.script_dir,
            'pyside_package'))
        if os.path.exists(clang_lib_path):
            log.info('Copying libclang shared library to the pyside package.')

            copydir(clang_lib_dir, destination_dir,
                filter=[clang_filter],
                recursive=False)
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
                osx_fix_rpaths_for_library(srcpath, final_rpath)

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
    with open(os.path.join(script_dir, 'README.rst')) as f:
        README = f.read()
    with open(os.path.join(script_dir, 'CHANGES.rst')) as f:
        CHANGES = f.read()
except IOError:
    README = CHANGES = ''


cmd_class_dict = {
    'build': pyside_build,
    'build_py': pyside_build_py,
    'build_ext': pyside_build_ext,
    'bdist_egg': pyside_bdist_egg,
    'develop': pyside_develop,
    'install': pyside_install,
    'install_lib': pyside_install_lib
}
if wheel_module_exists:
    cmd_class_dict['bdist_wheel'] = pyside_build_wheel

setup(
    name = "PySide2",
    version = get_package_version(),
    description = ("Python bindings for the Qt cross-platform application and "
        "UI framework"),
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
    packages = ['PySide2', 'pyside2uic',
                'pyside2uic.Compiler',
                'pyside2uic.port_v{}'.format(sys.version_info[0]) ],
    package_dir = {'': pyside_package_dir_name},
    include_package_data = True,
    zip_safe = False,
    entry_points = {
        'console_scripts': [
            'pyside2-uic = PySide2.scripts.uic:main',
        ]
    },
    cmdclass = cmd_class_dict,
    # Add a bogus extension module (will never be built here since we
    # are overriding the build command to do it using cmake) so things
    # like bdist_egg will know that there are extension modules and
    # will name the dist with the full platform info.
    ext_modules = [Extension('QtCore', [])],
    ext_package = 'PySide2',
)
