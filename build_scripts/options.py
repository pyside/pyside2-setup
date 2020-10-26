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
import distutils.log as log
from distutils.spawn import find_executable
import sys
import os
import warnings

from .qtinfo import QtInfo


_AVAILABLE_MKSPECS = ["ninja", "msvc", "mingw"] if sys.platform == "win32" else ["ninja", "make"]


# Global options not which are not part of the commands
ADDITIONAL_OPTIONS = """
Additional options:
  --limited-api                        Use Limited API [yes/no]
  ---macos-use-libc++                  Use libc++ on macOS
  --snapshot-build                     Snapshot build
  --package-timestamp                  Package Timestamp
"""


def _warn_multiple_option(option):
    warnings.warn('Option "{}" occurs multiple times on the command line.'.format(option))


def _warn_deprecated_option(option, replacement=None):
    w = 'Option "{}" is deprecated and may be removed in a future release.'.format(option)
    if replacement:
        w = '{}\nUse "{}" instead.'.format(w, replacement)
    warnings.warn(w)


class Options(object):
    def __init__(self):

        # Dictionary containing values of all the possible options.
        self.dict = {}

    def has_option(self, name, remove=True):
        """ Returns True if argument '--name' was passed on the command
        line. """
        option = '--{}'.format(name)
        count = sys.argv.count(option)
        remove_count = count
        if not remove and count > 0:
            remove_count -= 1
        for i in range(remove_count):
            sys.argv.remove(option)
        if count > 1:
            _warn_multiple_option(option)
        return count > 0

    def option_value(self, name, short_option_name=None, remove=True):
        """
        Returns the value of a command line option or environment
        variable.

        :param name: The name of the command line option or environment
         variable.

        :param remove: Whether the option and its value should be
         removed from sys.argv. Useful when there's a need to query for
         the value and also pass it along to setuptools for example.

        :return: Either the option value or None.
        """
        option = '--' + name
        short_option = '-' + short_option_name if short_option_name else None
        single_option_prefix = option + '='
        value = None
        for index in reversed(range(len(sys.argv))):
            arg = sys.argv[index]
            if arg == option or short_option and arg == short_option:
                if value:
                    _warn_multiple_option(option)
                else:
                    if index + 1 >= len(sys.argv):
                        raise RuntimeError("The option {} requires a value".format(option))
                    value = sys.argv[index + 1]

                if remove:
                    sys.argv[index:index + 2] = []

            elif arg.startswith(single_option_prefix):
                if value:
                    _warn_multiple_option(option)
                else:
                    value = arg[len(single_option_prefix):]

                if remove:
                    sys.argv[index:index + 1] = []

        if value is None:
            value = os.getenv(name.upper().replace('-', '_'))

        self.dict[name] = value
        return value


options = Options()


def has_option(*args, **kwargs):
    return options.has_option(*args, **kwargs)


def option_value(*args, **kwargs):
    return options.option_value(*args, **kwargs)


def _jobs_option_value():
    """Option value for parallel builds."""
    value = option_value('parallel', short_option_name='j')
    if value:
        return '-j' + value if not value.startswith('-j') else value
    return ''


# Declare options which need to be known when instantiating the DistUtils
# commands.
OPTION = {
    "BUILD_TYPE": option_value("build-type"),
    "INTERNAL_BUILD_TYPE": option_value("internal-build-type"),
    # number of parallel build jobs
    "JOBS": _jobs_option_value(),
    # Legacy, not used any more.
    "JOM": has_option('jom'),
    "MACOS_USE_LIBCPP": has_option("macos-use-libc++"),
    "QUIET": has_option('quiet', remove=False),
    "SNAPSHOT_BUILD": has_option("snapshot-build"),
    "LIMITED_API": option_value("limited-api"),
    "PACKAGE_TIMESTAMP": option_value("package-timestamp"),
    # This is used automatically by distutils.command.install object, to
    # specify the final installation location.
    "FINAL_INSTALL_PREFIX": option_value("prefix", remove=False)
    # This is used to identify the template for doc builds
}
_deprecated_option_jobs = option_value('jobs')
if _deprecated_option_jobs:
    _warn_deprecated_option('jobs', 'parallel')
    OPTION["JOBS"] = _deprecated_option_jobs


class DistUtilsCommandMixin(object):
    """Mixin for the DistUtils build/install commands handling the options."""

    _finalized = False

    mixin_user_options = [
        ('debug', None, 'Build with debug information'),
        ('relwithdebinfo', None, 'Build in release mode with debug information'),
        ('only-package', None, 'Package only'),
        ('standalone', None, 'Standalone build'),
        ('ignore-git', None, 'Do update subrepositories'),
        ('skip-docs', None, 'Skip documentation build'),
        ('no-examples', None, 'Do not build examples'),
        ('no-jom', None, 'Do not use jom (MSVC)'),
        ('build-tests', None, 'Build tests'),
        ('use-xvfb', None, 'Use Xvfb for testing'),
        ('reuse-build', None, 'Reuse existing build'),
        ('skip-cmake', None, 'Skip CMake step'),
        ('skip-make-install', None, 'Skip install step'),
        ('skip-packaging', None, 'Skip packaging step'),
        ('verbose-build', None, 'Verbose build'),
        ('sanitize-address', None, 'Build with address sanitizer'),
        ('shorter-paths', None, 'Use shorter paths'),
        ('doc-build-online', None, 'Build online documentation'),
        ('qmake=', None, 'Path to qmake'),
        ('qt=', None, 'Qt version'),
        ('cmake=', None, 'Path to CMake'),
        ('openssl=', None, 'Path to OpenSSL libraries'),
        ('shiboken-config-dir=', None, 'shiboken configuration directory'),
        ('make-spec=', None, 'Qt make-spec'),
        ('macos-arch=', None, 'macOS architecture'),
        ('macos-sysroot=', None, 'macOS sysroot'),
        ('macos-deployment-target=', None, 'macOS deployment target'),
        ('skip-modules=', None, 'Qt modules to be skipped'),
        ('module-subset=', None, 'Qt modules to be built'),
        ('rpath=', None, 'RPATH'),
        ('qt-conf-prefix=', None, 'Qt configuration prefix'),
        ('qt-src-dir=', None, 'Qt source directory')]

    def __init__(self):
        self.debug = False
        self.relwithdebinfo = False
        self.only_package = False
        self.standalone = False
        self.ignore_git = False
        self.skip_docs = False
        self.no_examples = False
        self.no_jom = False
        self.build_tests = False
        self.use_xvfb = False
        self.reuse_build = False
        self.skip_cmake = False
        self.skip_make_install = False
        self.skip_packaging = False
        self.verbose_build = False
        self.sanitize_address = False
        self.snapshot_build = False
        self.shorter_paths = False
        self.doc_build_online = False
        self.qmake = None
        self.qt = '5'
        self.cmake = None
        self.openssl = None
        self.shiboken_config_dir = None
        self.make_spec = None
        self.macos_arch = None
        self.macos_sysroot = None
        self.macos_deployment_target = None
        self.skip_modules = None
        self.module_subset = None
        self.rpath = None
        self.qt_conf_prefix = None
        self.qt_src_dir = None

    def mixin_finalize_options(self):
        # Bail out on 2nd call to mixin_finalize_options() since that is the
        # build command following the install command when invoking
        # setup.py install
        if not DistUtilsCommandMixin._finalized:
            DistUtilsCommandMixin._finalized = True
            self._do_finalize()

    def _do_finalize(self):
        if not self._determine_defaults_and_check():
            sys.exit(-1)
        OPTION['DEBUG'] = self.debug
        OPTION['RELWITHDEBINFO'] = self.relwithdebinfo
        OPTION['ONLYPACKAGE'] = self.only_package
        OPTION['STANDALONE'] = self.standalone
        OPTION['IGNOREGIT'] = self.ignore_git
        OPTION['SKIP_DOCS'] = self.skip_docs
        OPTION['NOEXAMPLES'] = self.no_examples
        OPTION['BUILDTESTS'] = self.build_tests
        OPTION['NO_JOM'] = self.no_jom
        OPTION['XVFB'] = self.use_xvfb
        OPTION['REUSE_BUILD'] = self.reuse_build
        OPTION['SKIP_CMAKE'] = self.skip_cmake
        OPTION['SKIP_MAKE_INSTALL'] = self.skip_make_install
        OPTION['SKIP_PACKAGING'] = self.skip_packaging
        OPTION['VERBOSE_BUILD'] = self.verbose_build
        if self.verbose_build:
            log.set_verbosity(1)
        OPTION['SANITIZE_ADDRESS'] = self.sanitize_address
        OPTION['SHORTER_PATHS'] = self.shorter_paths
        OPTION['DOC_BUILD_ONLINE'] = self.doc_build_online
        # make qtinfo.py independent of relative paths.
        qmake_abs_path = os.path.abspath(self.qmake)
        OPTION['QMAKE'] = qmake_abs_path
        OPTION['QT_VERSION'] = self.qt
        QtInfo().setup(qmake_abs_path, self.qt)
        OPTION['CMAKE'] = os.path.abspath(self.cmake)
        OPTION['OPENSSL'] = self.openssl
        OPTION['SHIBOKEN_CONFIG_DIR'] = self.shiboken_config_dir
        OPTION['MAKESPEC'] = self.make_spec
        OPTION['MACOS_ARCH'] = self.macos_arch
        OPTION['MACOS_SYSROOT'] = self.macos_sysroot
        OPTION['MACOS_DEPLOYMENT_TARGET'] = self.macos_deployment_target
        OPTION['SKIP_MODULES'] = self.skip_modules
        OPTION['MODULE_SUBSET'] = self.module_subset
        OPTION['RPATH_VALUES'] = self.rpath
        OPTION['QT_CONF_PREFIX'] = self.qt_conf_prefix
        OPTION['QT_SRC'] = self.qt_src_dir

    def _determine_defaults_and_check(self):
        if not self.cmake:
            self.cmake = find_executable("cmake")
        if not self.cmake:
            print("cmake could not be found.")
            return False
        if not os.path.exists(self.cmake):
            print("'{}' does not exist.".format(self.cmake))
            return False

        if not self.qmake:
            self.qmake = find_executable("qmake")
            if not self.qmake:
                self.qmake = find_executable("qmake-qt5")
        if not self.qmake:
            print("qmake could not be found.")
            return False
        if not os.path.exists(self.qmake):
            print("'{}' does not exist.".format(self.qmake))
            return False

        if not self.make_spec:
            self.make_spec = _AVAILABLE_MKSPECS[0]
        if self.make_spec not in _AVAILABLE_MKSPECS:
            print('Invalid option --make-spec "{}". Available values are {}'.format(self.make_spec,
                                                                                    _AVAILABLE_MKSPECS))
            return False

        if OPTION["JOBS"] and sys.platform == 'win32' and self.no_jom:
            print("Option --jobs can only be used with jom on Windows.")
            return False

        return True
