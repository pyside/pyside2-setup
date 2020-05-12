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
import sys
import os
import warnings


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


# Declare options
OPTION = {
    "BUILD_TYPE": option_value("build-type"),
    "INTERNAL_BUILD_TYPE": option_value("internal-build-type"),
    "DEBUG": has_option("debug"),
    "RELWITHDEBINFO": has_option('relwithdebinfo'),
    "QMAKE": option_value("qmake"),
    "QT_VERSION": option_value("qt"),
    "CMAKE": option_value("cmake"),
    "OPENSSL": option_value("openssl"),
    "SHIBOKEN_CONFIG_DIR": option_value("shiboken-config-dir"),
    "ONLYPACKAGE": has_option("only-package"),
    "STANDALONE": has_option("standalone"),
    "MAKESPEC": option_value("make-spec"),
    "IGNOREGIT": has_option("ignore-git"),
    # don't generate documentation
    "SKIP_DOCS": has_option("skip-docs"),
    # don't include pyside2-examples
    "NOEXAMPLES": has_option("no-examples"),
    # number of parallel build jobs
    "JOBS": option_value('parallel', short_option_name='j'),
    # Legacy, not used any more.
    "JOM": has_option('jom'),
    # Do not use jom instead of nmake with msvc
    "NO_JOM": has_option('no-jom'),
    "BUILDTESTS": has_option("build-tests"),
    "MACOS_ARCH": option_value("macos-arch"),
    "MACOS_USE_LIBCPP": has_option("macos-use-libc++"),
    "MACOS_SYSROOT": option_value("macos-sysroot"),
    "MACOS_DEPLOYMENT_TARGET": option_value("macos-deployment-target"),
    "XVFB": has_option("use-xvfb"),
    "REUSE_BUILD": has_option("reuse-build"),
    "SKIP_CMAKE": has_option("skip-cmake"),
    "SKIP_MAKE_INSTALL": has_option("skip-make-install"),
    "SKIP_PACKAGING": has_option("skip-packaging"),
    "SKIP_MODULES": option_value("skip-modules"),
    "MODULE_SUBSET": option_value("module-subset"),
    "RPATH_VALUES": option_value("rpath"),
    "QT_CONF_PREFIX": option_value("qt-conf-prefix"),
    "QT_SRC": option_value("qt-src-dir"),
    "QUIET": has_option('quiet', remove=False),
    "VERBOSE_BUILD": has_option("verbose-build"),
    "SANITIZE_ADDRESS": has_option("sanitize-address"),
    "SNAPSHOT_BUILD": has_option("snapshot-build"),
    "LIMITED_API": option_value("limited-api"),
    "PACKAGE_TIMESTAMP": option_value("package-timestamp"),
    "SHORTER_PATHS": has_option("shorter-paths"),
    # This is used automatically by distutils.command.install object, to
    # specify the final installation location.
    "FINAL_INSTALL_PREFIX": option_value("prefix", remove=False),
    # This is used to identify the template for doc builds
    "DOC_BUILD_ONLINE": has_option("doc-build-online"),
}
_deprecated_option_jobs = option_value('jobs')
if _deprecated_option_jobs:
    _warn_deprecated_option('jobs', 'parallel')
    OPTION["JOBS"] = _deprecated_option_jobs
