"""This is a distutils setup-script for the PySide project

To build the PySide, simply execute:
  python setup.py build --qmake=</path/to/qt/bin/qmake> [--cmake=</path/to/cmake>] [--opnessl=</path/to/openssl/bin>]
or
  python setup.py install --qmake=</path/to/qt/bin/qmake> [--cmake=</path/to/cmake>] [--opnessl=</path/to/openssl/bin>]
to build and install into your current Python installation.

You can use special option --only-package, if you want to create more binary packages (bdist_egg, bdist_wininst, ...)
without rebuilding entire PySide every time.

Examples:
  # First time we create bdist_winist with full PySide build
  python setup.py bdist_winist --qmake=c:\Qt\4.7.4\bin\qmake.exe --cmake=c:\tools\cmake\bin\cmake.exe --opnessl=c:\libs\OpenSSL32bit\bin
  
  # Then we crate bdist_egg reusing PySide build with option --only-package
  python setup.py bdist_egg --only-package --qmake=c:\Qt\4.7.4\bin\qmake.exe --cmake=c:\tools\cmake\bin\cmake.exe --opnessl=c:\libs\OpenSSL32bit\bin

REQUIREMENTS:
- Python: 2.6, 2.7 and 3.2 is supported
- Cmake: Specify the path to cmake with --cmake option or add cmake to the system path.
- Qt: 4.6, 4.7 and 4.8 is supported. Specify the path to qmake with --qmake option or add qmake to the system path.

OPTIONAL:
OpenSSL: You can specify the location of OpenSSL DLLs with option --opnessl=</path/to/openssl/bin>.
    You can download OpenSSL for windows here: http://slproweb.com/products/Win32OpenSSL.html

NOTES:
On Windows You need to execute this script from Visual Studio 2008 Command Prompt.
Building 64bit version is not supported with Visual Studio 2008 Express Edition.
"""

__version__ = "1.1.2"

submodules = {
    __version__: [
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
    from distribute_setup import use_setuptools
    use_setuptools()

import os
import sys
import platform

from distutils import log
from distutils.errors import DistutilsOptionError
from distutils.errors import DistutilsSetupError
from distutils.sysconfig import get_config_var
from distutils.spawn import find_executable
from distutils.command.build import build as _build

from setuptools import setup
from setuptools.command.install import install as _install
from setuptools.command.bdist_egg import bdist_egg as _bdist_egg

from qtinfo import QtInfo
from utils import rmtree
from utils import makefile
from utils import copyfile
from utils import copydir
from utils import run_process
from utils import has_option
from utils import option_value

OPTION_DEBUG = has_option("debug")
OPTION_QMAKE = option_value("qmake")
OPTION_CMAKE = option_value("cmake")
OPTION_OPENSSL = option_value("openssl")
OPTION_ONLYPACKAGE = has_option("only-package")
OPTION_STANDALONE = has_option("standalone")
OPTION_VERSION = option_value("version")
OPTION_LISTVERSIONS = has_option("list-versions")

if OPTION_QMAKE is None:
    OPTION_QMAKE = find_executable("qmake")
if OPTION_CMAKE is None:
    OPTION_CMAKE = find_executable("cmake")

if OPTION_LISTVERSIONS:
    for v in submodules:
        print("%s" % (v))
        for m in submodules[v]:
            print("  %s %s" % (m[0], m[1]))
    sys.exit(1)

if OPTION_VERSION:
    if not OPTION_VERSION in submodules:
        print("""Invalid version specified %s
Use --list-versions option to get list of available versions""" % OPTION_VERSION)
        sys.exit(1)
    __version__ = OPTION_VERSION

# Change the cwd to our source dir
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))
script_dir = os.getcwd()

# Clean temp build folders
for n in ["build", "PySide.egg-info", "PySide-%s" % __version__,
    "PySide", "pysideuic"]:
    d = os.path.join(script_dir, n)
    if os.path.isdir(d):
        print("Removing %s" % d)
        rmtree(d)

# Create empty package folders
for pkg in ["PySide", "pysideuic"]:
    pkg_dir = os.path.join(script_dir, pkg)
    os.makedirs(pkg_dir)

# Ensure that git submodules are initialized, if this is the git repo clone
if os.path.isdir(".git"):
    print("Initializing submodules for PySide version %s" % __version__)
    git_update_cmd = ["git", "submodule", "update", "--init"]
    if run_process(git_update_cmd) != 0:
        raise DistutilsSetupError("Failed to initialize the git submodules")
    git_pull_cmd = ["git", "submodule", "foreach", "git", "pull", "origin", "master"]
    if run_process(git_pull_cmd) != 0:
        raise DistutilsSetupError("Failed to initialize the git submodules")
    submodules_dir = os.path.join(script_dir, "sources")
    for m in submodules[__version__]:
        module_name = m[0]
        module_version = m[1]
        print("Checking out submodule %s to branch %s" % (module_name, module_version))
        module_dir = os.path.join(submodules_dir, module_name)
        os.chdir(module_dir)
        git_checkout_cmd = ["git", "checkout", module_version]
        if run_process(git_checkout_cmd) != 0:
            raise DistutilsSetupError("Failed to initialize the git submodule %s" % module_name)
        os.chdir(script_dir)

class pyside_install(_install):
    def run(self):
        _install.run(self)
        # Custom script we run at the end of installing - this is the same script
        # run by bdist_wininst
        # If self.root has a value, it means we are being "installed" into
        # some other directory than Python itself (eg, into a temp directory
        # for bdist_wininst to use) - in which case we must *not* run our
        # installer
        if not self.dry_run and not self.root:
            if sys.platform == "win32":
                filename = os.path.join(self.prefix, "Scripts", "pyside_postinstall.py")
            else:
                filename = os.path.join(self.prefix, "bin", "pyside_postinstall.py")
            if not os.path.isfile(filename):
                raise RuntimeError("Can't find '%s'" % (filename,))
            print("Executing post install script '%s'..." % filename)
            cmd = [
                os.path.abspath(sys.executable),
                filename,
                "-install"
            ]
            run_process(cmd)

class pyside_bdist_egg(_bdist_egg):

    def __init__(self, *args, **kwargs):
        _bdist_egg.__init__(self, *args, **kwargs)

    def run(self):
        self.run_command("build")
        _bdist_egg.run(self)

class pyside_build(_build):

    def __init__(self, *args, **kwargs):
        _build.__init__(self, *args, **kwargs)

    def initialize_options(self):
        _build.initialize_options(self)
        self.debug = False
        self.script_dir = None
        self.sources_dir = None
        self.build_dir = None
        self.install_dir = None
        self.qmake_path = None
        self.py_executable = None
        self.py_include_dir = None
        self.py_library = None
        self.py_version = None
        self.build_type = "Release"
        self.qtinfo = None
    
    def run(self):
        # Check env
        if not OPTION_ONLYPACKAGE:
            make_name = sys.platform == "win32" and "nmake" or "make"
            make_path = find_executable(make_name)
            if make_path is None or not os.path.exists(make_path):
                raise DistutilsSetupError(
                    "You need the program \"%s\" on your system path to compile PySide." \
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
        build_type = OPTION_DEBUG and "Debug" or "Release"
        py_executable = sys.executable
        py_version = "%s.%s" % (sys.version_info[0], sys.version_info[1])
        py_include_dir = get_config_var("INCLUDEPY")
        py_libdir = get_config_var("LIBDIR")
        if py_libdir is None or not os.path.exists(py_libdir):
            py_prefix = get_config_var("prefix")
            if sys.platform == "win32":
                py_libdir = os.path.join(py_prefix, "libs")
            else:
                py_libdir = os.path.join(py_prefix, "lib")
        dbgPostfix = ""
        if build_type == "Debug":
            dbgPostfix = "_d"
        if sys.platform == "win32":
            py_library = os.path.join(py_libdir, "python%s%s.lib" % \
                (py_version.replace(".", ""), dbgPostfix))
        else:
            if sys.version_info[0] > 2:
                py_abiflags = getattr(sys, 'abiflags', None)
                py_library = os.path.join(py_libdir, "libpython%s%s.so" % \
                    (py_version, py_abiflags))
            else:
                py_library = os.path.join(py_libdir, "libpython%s%s.so" % \
                    (py_version, dbgPostfix))
                if not os.path.exists(py_library):
                    py_library = py_library + ".1"
        if not os.path.exists(py_library):
            raise DistutilsSetupError(
                "Failed to locate the Python library %s" % py_library)
        
        qtinfo = QtInfo(OPTION_QMAKE)
        
        # Update os.path
        paths = os.environ['PATH'].lower().split(os.pathsep)
        def updatepath(path):
            if not path.lower() in paths:
                log.info("Adding path \"%s\" to environment" % path)
                paths.append(path)
        qt_dir = os.path.dirname(OPTION_QMAKE)
        updatepath(qt_dir)
        os.environ['PATH'] = os.pathsep.join(paths)
        
        qt_version = qtinfo.version
        if not qt_version:
            log.error("Failed to query the Qt version with qmake %s" % qtinfo.qmake_path)
            sys.exit(1)
        
        build_name = "py%s-qt%s-%s-%s" % \
            (py_version, qt_version, platform.architecture()[0], build_type.lower())
        
        script_dir = os.getcwd()
        sources_dir = os.path.join(script_dir, "sources")
        build_dir = os.path.join(script_dir, os.path.join("pyside_build", "%s" % build_name))
        install_dir = os.path.join(script_dir, os.path.join("pyside_install", "%s" % build_name))
        
        self.debug = OPTION_DEBUG
        self.script_dir = script_dir
        self.sources_dir = sources_dir
        self.build_dir = build_dir
        self.install_dir = install_dir
        self.qmake_path = OPTION_QMAKE
        self.py_executable = py_executable
        self.py_include_dir = py_include_dir
        self.py_library = py_library
        self.py_version = py_version
        self.build_type = build_type
        self.qtinfo = qtinfo
        
        log.info("=" * 30)
        log.info("Build type: %s" % self.build_type)
        log.info("Package version: %s" % __version__)
        log.info("-" * 3)
        log.info("Script directory: %s" % self.script_dir)
        log.info("Sources directory: %s" % self.sources_dir)
        log.info("Build directory: %s" % self.build_dir)
        log.info("Install directory: %s" % self.install_dir)
        log.info("-" * 3)
        log.info("Python executable: %s" % self.py_executable)
        log.info("Python includes: %s" % self.py_include_dir)
        log.info("Python library: %s" % self.py_library)
        log.info("-" * 3)
        log.info("Qt qmake: %s" % self.qmake_path)
        log.info("Qt version: %s" % qtinfo.version)
        log.info("Qt bins: %s" % qtinfo.bins_dir)
        log.info("Qt plugins: %s" % qtinfo.plugins_dir)
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
            for ext in ['shiboken', 'pyside', 'pyside-tools']:
                self.build_extension(ext)

        # Build patchelf
        self.build_patchelf()

        # Prepare packages
        self.prepare_packages()
        
        # Build packages
        _build.run(self)

    def build_patchelf(self):
        if sys.platform != "linux2":
            return
        
        log.info("Building patchelf...")
        
        module_src_dir = os.path.join(self.sources_dir, "patchelf")
        
        build_cmd = [
            "g++",
            "%s/patchelf.cc" % (module_src_dir),
            "-o",
            "patchelf",
        ]
        
        if run_process(build_cmd, log) != 0:
            raise DistutilsSetupError("Error building patchelf")

    def build_extension(self, extension):
        log.info("Building module %s..." % extension)
        
        # Prepare folders
        os.chdir(self.build_dir)
        module_build_dir = os.path.join(self.build_dir,  extension)
        if os.path.exists(module_build_dir):
            log.info("Deleting module build folder %s..." % module_build_dir)
            rmtree(module_build_dir)
        log.info("Creating module build folder %s..." % module_build_dir)
        os.mkdir(module_build_dir)
        os.chdir(module_build_dir)
        
        module_src_dir = os.path.join(self.sources_dir, extension)
        
        # Build module
        if sys.platform == "win32":
            cmake_generator = "NMake Makefiles"
            make_cmd = "nmake"
        else:
            cmake_generator = "Unix Makefiles"
            make_cmd = "make"
        cmake_cmd = [
            "cmake",
            "-G", cmake_generator,
            "-DQT_QMAKE_EXECUTABLE=%s" % self.qmake_path,
            "-DBUILD_TESTS=False",
            "-DDISABLE_DOCSTRINGS=True",
            "-DCMAKE_BUILD_TYPE=%s" % self.build_type,
            "-DCMAKE_INSTALL_PREFIX=%s" % self.install_dir,
            module_src_dir
        ]
        if sys.version_info[0] > 2:
            cmake_cmd.append("-DPYTHON3_EXECUTABLE=%s" % self.py_executable)
            cmake_cmd.append("-DPYTHON3_INCLUDE_DIR=%s" % self.py_include_dir)
            cmake_cmd.append("-DPYTHON3_LIBRARY=%s" % self.py_library)
            if self.build_type.lower() == 'debug':
                cmake_cmd.append("-DPYTHON3_DBG_EXECUTABLE=%s" % self.py_executable)
                cmake_cmd.append("-DPYTHON3_DEBUG_LIBRARY=%s" % self.py_library)
        else:
            cmake_cmd.append("-DPYTHON_EXECUTABLE=%s" % self.py_executable)
            cmake_cmd.append("-DPYTHON_INCLUDE_DIR=%s" % self.py_include_dir)
            cmake_cmd.append("-DPYTHON_LIBRARY=%s" % self.py_library)
            if self.build_type.lower() == 'debug':
                cmake_cmd.append("-DPYTHON_DEBUG_LIBRARY=%s" % self.py_library)
        if extension.lower() == "shiboken":
            cmake_cmd.append("-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=yes")
            if sys.version_info[0] > 2:
                cmake_cmd.append("-DUSE_PYTHON3=ON")
        
        log.info("Configuring module %s (%s)..." % (extension,  module_src_dir))
        if run_process(cmake_cmd, log) != 0:
            raise DistutilsSetupError("Error configuring " + extension)
        
        log.info("Compiling module %s..." % extension)
        if run_process([make_cmd], log) != 0:
            raise DistutilsSetupError("Error compiling " + extension)
        
        log.info("Installing module %s..." % extension)
        if run_process([make_cmd, "install/fast"], log) != 0:
            raise DistutilsSetupError("Error pseudo installing " + extension)
        
        os.chdir(self.script_dir)

    def prepare_packages(self):
        log.info("Preparing packages...")
        version_str = "%sqt%s%s" % (__version__, self.qtinfo.version.replace(".", "")[0:3],
            self.debug and "dbg" or "")
        vars = {
            "sources_dir": self.sources_dir,
            "install_dir": self.install_dir,
            "build_dir": self.build_dir,
            "setup_dir": self.script_dir,
            "ssl_libs_dir": OPTION_OPENSSL,
            "py_version": self.py_version,
            "qt_version": self.qtinfo.version,
            "qt_bin_dir": self.qtinfo.bins_dir,
            "qt_lib_dir": self.qtinfo.libs_dir,
            "qt_plugins_dir": self.qtinfo.plugins_dir,
            "qt_imports_dir": self.qtinfo.imports_dir,
            "qt_translations_dir": self.qtinfo.translations_dir,
            "version": version_str,
        }
        os.chdir(self.script_dir)
        if sys.platform == "win32":
            return self.prepare_packages_win32(vars)
        return self.prepare_packages_linux(vars)

    def prepare_packages_linux(self, vars):
        # patchelf -> PySide/patchelf
        copyfile(
            "{setup_dir}/patchelf",
            "{setup_dir}/PySide/patchelf",
            logger=log, vars=vars)
        # <install>/lib/site-packages/PySide/* -> <setup>/PySide
        copydir(
            "{install_dir}/lib/python{py_version}/site-packages/PySide",
            "{setup_dir}/PySide",
            logger=log, vars=vars)
        # <install>/lib/site-packages/pysideuic/* -> <setup>/pysideuic
        copydir(
            "{install_dir}/lib/python{py_version}/site-packages/pysideuic",
            "{setup_dir}/pysideuic",
            force=False, logger=log, vars=vars)
        # <install>/bin/pyside-uic -> PySide/scripts/uic.py
        makefile(
            "{setup_dir}/PySide/scripts/__init__.py",
            logger=log, vars=vars)
        copyfile(
            "{install_dir}/bin/pyside-uic",
            "{setup_dir}/PySide/scripts/uic.py",
            force=False, logger=log, vars=vars)
        # <install>/bin/* -> PySide/
        copydir(
            "{install_dir}/bin/",
            "{setup_dir}/PySide",
            filter=[
                "pyside-lupdate",
                "pyside-rcc",
                "shiboken",
            ],
            recursive=False, logger=log, vars=vars)
        # <install>/lib/lib* -> PySide/
        copydir(
            "{install_dir}/lib/",
            "{setup_dir}/PySide",
            filter=[
                "libpyside*.so.*",
                "libshiboken*.so.*",
            ],
            recursive=False, logger=log, vars=vars)
        # <install>/share/PySide/typesystems/* -> <setup>/PySide/typesystems
        copydir(
            "{install_dir}/share/PySide/typesystems",
            "{setup_dir}/PySide/typesystems",
            logger=log, vars=vars)
        # <install>/include/* -> <setup>/PySide/include
        copydir(
            "{install_dir}/include",
            "{setup_dir}/PySide/include",
            logger=log, vars=vars)
        # <sources>/pyside-examples/examples/* -> <setup>/PySide/examples
        copydir(
            "{sources_dir}/pyside-examples/examples",
            "{setup_dir}/PySide/examples",
            force=False, logger=log, vars=vars)
        # Copy Qt libs to package
        if OPTION_STANDALONE:
            # <qt>/bin/* -> <setup>/PySide
            copydir("{qt_bin_dir}", "{setup_dir}/PySide",
                filter=[
                    "designer",
                    "linguist",
                    "lrelease",
                    "lupdate",
                    "lconvert",
                ],
                recursive=False, logger=log, vars=vars)
            # <qt>/lib/* -> <setup>/PySide
            copydir("{qt_lib_dir}", "{setup_dir}/PySide",
                filter=[
                    "libQt*.so.?",
                    "libphonon.so.?",
                ],
                recursive=False, logger=log, vars=vars)
            # <qt>/plugins/* -> <setup>/PySide/plugins
            copydir("{qt_plugins_dir}", "{setup_dir}/PySide/plugins",
                filter=["*.so"],
                logger=log, vars=vars)
            # <qt>/imports/* -> <setup>/PySide/imports
            if float(vars["qt_version"][:3]) > 4.6:
                copydir("{qt_imports_dir}", "{setup_dir}/PySide/imports",
                    filter=["qmldir", "*.so"],
                    logger=log, vars=vars)
            # <qt>/translations/* -> <setup>/PySide/translations
            copydir("{qt_translations_dir}", "{setup_dir}/PySide/translations",
                filter=["*.qm"],
                logger=log, vars=vars)

    def prepare_packages_win32(self, vars):
        # <install>/lib/site-packages/PySide/* -> <setup>/PySide
        copydir(
            "{install_dir}/lib/site-packages/PySide",
            "{setup_dir}/PySide",
            logger=log, vars=vars)
        if self.debug:
            # <build>/pyside/PySide/*.pdb -> <setup>/PySide
            copydir(
                "{build_dir}/pyside/PySide",
                "{setup_dir}/PySide",
                filter=["*.pdb"],
                recursive=False, logger=log, vars=vars)
        # <install>/lib/site-packages/pysideuic/* -> <setup>/pysideuic
        copydir(
            "{install_dir}/lib/site-packages/pysideuic",
            "{setup_dir}/pysideuic",
            force=False, logger=log, vars=vars)
        # <install>/bin/pyside-uic -> PySide/scripts/uic.py
        makefile(
            "{setup_dir}/PySide/scripts/__init__.py",
            logger=log, vars=vars)
        copyfile(
            "{install_dir}/bin/pyside-uic",
            "{setup_dir}/PySide/scripts/uic.py",
            force=False, logger=log, vars=vars)
        # <install>/bin/*.exe,*.dll -> PySide/
        copydir(
            "{install_dir}/bin/",
            "{setup_dir}/PySide",
            filter=["*.exe", "*.dll"],
            recursive=False, logger=log, vars=vars)
        # <install>/lib/*.lib -> PySide/
        copydir(
            "{install_dir}/lib/",
            "{setup_dir}/PySide",
            filter=["*.lib"],
            recursive=False, logger=log, vars=vars)
        # <install>/share/PySide/typesystems/* -> <setup>/PySide/typesystems
        copydir(
            "{install_dir}/share/PySide/typesystems",
            "{setup_dir}/PySide/typesystems",
            logger=log, vars=vars)
        # <install>/include/* -> <setup>/PySide/include
        copydir(
            "{install_dir}/include",
            "{setup_dir}/PySide/include",
            logger=log, vars=vars)
        # <sources>/pyside-examples/examples/* -> <setup>/PySide/examples
        copydir(
            "{sources_dir}/pyside-examples/examples",
            "{setup_dir}/PySide/examples",
            force=False, logger=log, vars=vars)
        # <ssl_libs>/* -> <setup>/PySide/
        copydir("{ssl_libs_dir}", "{setup_dir}/PySide",
            filter=[
                "libeay32.dll",
                "ssleay32.dll"],
            force=False, logger=log, vars=vars)
        # <qt>/bin/*.dll -> <setup>/PySide
        copydir("{qt_bin_dir}", "{setup_dir}/PySide",
            filter=[
                "*.dll",
                "designer.exe",
                "linguist.exe",
                "lrelease.exe",
                "lupdate.exe",
                "lconvert.exe"],
            ignore=["*d4.dll"],
            recursive=False, logger=log, vars=vars)
        if self.debug:
            # <qt>/bin/*d4.dll -> <setup>/PySide
            copydir("{qt_bin_dir}", "{setup_dir}/PySide",
                filter=["*d4.dll"],
                recursive=False, logger=log, vars=vars)
        # <qt>/plugins/* -> <setup>/PySide/plugins
        copydir("{qt_plugins_dir}", "{setup_dir}/PySide/plugins",
            filter=["*.dll"],
            logger=log, vars=vars)
        # <qt>/imports/* -> <setup>/PySide/imports
        copydir("{qt_imports_dir}", "{setup_dir}/PySide/imports",
            filter=["qmldir", "*.dll"],
            logger=log, vars=vars)
        # <qt>/translations/* -> <setup>/PySide/translations
        copydir("{qt_translations_dir}", "{setup_dir}/PySide/translations",
            filter=["*.qm"],
            logger=log, vars=vars)

def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

setup(
    name = "PySide",
    version = __version__,
    description = ("Python bindings for the Qt cross-platform application and UI framework"),
    long_description = read('README.txt'),
    options = {
        "bdist_wininst": {
            "install_script": "pyside_postinstall.py",
        },
        "bdist_msi": {
            "install_script": "pyside_postinstall.py",
        },
    },
    scripts = [
        "pyside_postinstall.py"
    ],
    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'Environment :: MacOS X',
        'Environment :: X11 Applications :: Qt',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C++',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Topic :: Database',
        'Topic :: Software Development',
        'Topic :: Software Development :: Code Generators',
        'Topic :: Software Development :: Libraries :: Application Frameworks',
        'Topic :: Software Development :: User Interfaces',
        'Topic :: Software Development :: Widget Sets',
    ],
    keywords = 'Qt',
    author = 'PySide Team',
    author_email = 'contact@pyside.org',
    url = 'http://www.pyside.org',
    download_url = 'http://releases.qt-project.org/pyside/%s' % __version__,
    license = 'LGPL',
    packages = ['PySide', 'pysideuic'],
    include_package_data = True,
    zip_safe = False,
    entry_points = {
        'console_scripts': [
            'pyside-uic = PySide.scripts.uic:main',
        ]
    },
    cmdclass = {
        'build': pyside_build,
        'bdist_egg': pyside_bdist_egg,
        'install': pyside_install,
    },
)
