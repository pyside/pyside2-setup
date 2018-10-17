#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
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

"""
This is a distutils setup-script for the Qt for Python project

To build both shiboken2 and PySide2 simply execute:

    python setup.py build

or

    python setup.py install

to build and install into your current Python installation.

The same setup.py script is used to build all the components of the
project:
  - shiboken2 (the supporting Python module)
  - shiboken2-generator (the bindings generation executable)
  - PySide2
  - pyside2-tools

Preferably, a Qt (build) environment should be used to automatically
pick up the associated `qmake`, but optionally one can specify the
location of `qmake` and `cmake` if it is not in the current PATH with:

    --qmake=/path/to/qt/bin/qmake

and

    --cmake=/path/to/bin/cmake

respectively.

By default, all of the above is built when no special options are
passed to the script. You can use the --build-type parameter to specify
which things should be built:

  --build-type=shiboken2 - build / package only the python module
  --build-type=shiboken2-generator - build / package the generator
                                     executable
  --build-type=pyside2 - build / package the PySide2 bindings and
                        and pyside2-tools
  --build-type=all - the implicit default to build all of the above


When building PySide2, optionally, one can specify the location of the
shiboken2 cmake config path if it is not on the current PATH with:

     --shiboken-config-dir=/path/to/shiboken/cmake/config/dir

This is useful if you did a cmake installation of shiboken2 into
a custom location.

For Windows, if OpenSSL support is required, it's necessary to specify
the directory path that contains the OpenSSL shared libraries
"libeay32.dll" and "ssleay32.dll", for example:

    --openssl=C:\OpenSSL-Win64\bin

This will make sure that the libraries are copied into the PySide2
package and are found by the QtNetwork module.

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
entire project every time:

e.g.:

* First, we create a bdist_wheel from a full PySide2 build:

  python setup.py bdist_wheel --qmake=c:\Qt\5.12\bin\qmake.exe
    --cmake=c:\tools\cmake\bin\cmake.exe
    --openssl=c:\libs\OpenSSL32bit\bin

* Then, we create a bdist_egg reusing the PySide2 build with option
  `--only-package`:

  python setup.py bdist_egg --only-package
    --qmake=c:\Qt\5.12\bin\qmake.exe
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
or "0" (or leave empty) to keep using the internal `qt.conf` file.

DEVELOPMENT OPTIONS:

For development purposes the following options might be of use, when
using `setup.py build`:

  --ignore-git will skip the fetching and checkout steps for
        supermodule and all submodules.
  --limited-api=yes|no  default yes if applicable
        Set or clear the limited API flag. Ignored for Python 2.
  --module-subset allows for specifying the Qt modules to be built.
    A minimal set is: --module-subset=Core,Gui,Test,Widgets
  --package-timestamp allows specifying the timestamp that will be
        used as part of the version number for a snapshot package.
        For example given --package-timestamp=1529646276
        the package version will be 5.x.y.dev1529646276.
  --reuse-build option allows recompiling only the modified sources and
        not the whole world, shortening development iteration time.
  --sanitize-address will build the project with address sanitizer.
  --skip-cmake will reuse the already generated Makefiles (or
        equivalents), instead of invoking, CMake to update the
        Makefiles (note, CMake should be ran at least once to generate
        the files).
  --skip-docs skip the documentation generation.
  --skip-make-install will not run make install (or equivalent) for
        each module built.
  --skip-modules allows for specifying the Qt modules that will be
    skipped during the build process.
    For example: --skip-modules=WebEngineCore,WebEngineWidgets
  --skip-packaging will skip creation of the python package,
    enabled (Linux or macOS only).
  --verbose-build will output the compiler invocation with command line
        arguments, etc.

REQUIREMENTS:

* Python: 2.7 and 3.5+ are supported
* CMake: Specify the path to cmake with `--cmake` option or add cmake
  to the system path.
* Qt: 5.11+ is supported. Specify the path to qmake with
  `--qmake` option or add qmake to the system path.

OPTIONAL:

* OpenSSL:
    Specifying the --openssl option only affects Windows.
    It is a no-op for other platforms.

    Please note that official Windows packages do not ship the
    OpenSSL libraries due to import / export restrictions as
    described in
    http://doc.qt.io/qt-5/ssl.html#import-and-export-restrictions

    You can specify the location of the OpenSSL DLLs with the
    following option:

        --openssl=</path/to/openssl/bin-directory>.

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

    (*) Revised on 2018.10.24

* macOS SDK:
    You can specify which macOS SDK should be used for compilation with
    the option:

        --macos-sysroot=</path/to/sdk>.

    e.g.: "--macos-sysroot=/Applications/Xcode.app/.../Developer/SDKs/MacOSX10.12.sdk/"

* macOS minimum deployment target:
    You can specify a custom macOS minimum deployment target with the
    option:

        --macos-deployment-target=<value>

    e.g.: "--macos-deployment-target=10.10"

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
import sys

# Change the current directory to setup.py's dir.
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
if os.path.dirname(this_file):
    os.chdir(os.path.dirname(this_file))

# Save the original command line arguments to pass them on to the setup
# mechanism.
original_argv = list(sys.argv)

from build_scripts.main import get_package_version, check_allowed_python_version
from build_scripts.setup_runner import SetupRunner

# The __version__ variable is just for PEP compliance, and shouldn't be
# used as a value source. Use get_package_version() instead.
__version__ = get_package_version()

check_allowed_python_version()

setup_runner = SetupRunner(original_argv)
setup_runner.run_setup()
