# Qt For Python

Qt For Python is the [Python Qt bindings project](http://wiki.qt.io/PySide2),
providing access to the complete Qt framework as well as to generator tools for
rapidly generating bindings for any C++ libraries.

Shiboken is the generator used to build the bindings.

See README.pyside2.md and README.shiboken6.md for details.

## Building

To build both Shiboken and PySide simply execute:

 * `python setup.py build`, or
 * `python setup.py install`

to build and install into your current Python installation.

The same setup.py script is used to build all the components of the project:

 * shiboken6 (the supporting Python module)
 * shiboken6-generator (the bindings generation executable)
 * PySide2

Preferably, a Qt (build) environment should be used to automatically pick up
the associated `qmake`, but optionally one can specify the location of `qmake`
and `cmake` if it is not in the current PATH with:

 * `--qmake=/path/to/qt/bin/qmake`, and
 * `--cmake=/path/to/bin/cmake`

respectively.

By default, all of the above is built when no special options are passed to the
script. You can use the --build-type parameter to specify which things should
be built:

 * `--build-type=shiboken6`, build/package only the python module
 * `--build-type=shiboken6-generator`, build/package the generator executable
 * `--build-type=pyside2`, build/package the PySide2 bindings.
 * `--build-type=all`, the implicit default to build all of the above

When building PySide2, optionally, one can specify the location of the
shiboken6 cmake config path if it is not on the current PATH with:

 * `--shiboken-config-dir=/path/to/shiboken/cmake/config/dir`

This is useful if you did a cmake installation of shiboken6 into a custom
location.

For Windows, if OpenSSL support is required, it's necessary to specify the
directory path that contains the OpenSSL shared libraries `libeay32.dll` and
`ssleay32.dll`, for example:

* `--openssl=C:\OpenSSL-Win64\bin`

This will make sure that the libraries are copied into the PySide2 package and
are found by the QtNetwork module.

## Building Additional Options

On Linux and macOS you can use the option `--standalone` to embed Qt libraries
into the PySide2 package.  The option does not affect Windows, because it is
used implicitly, i.e. all relevant DLLs have to be copied into the PySide2
package anyway, because there is no proper rpath support on the platform.

You can use the option `--rpath=/path/to/lib/path` to specify which rpath
values should be embedded into the PySide2 modules and shared libraries.  This
overrides the automatically generated values when the option is not specified.

You can use the option `--only-package` if you want to create more binary
packages (bdist_wheel, bdist_egg, ...) without rebuilding the entire project
every time:

e.g.:

* First, we create a bdist_wheel from a full PySide2 build:
  ```
  python setup.py bdist_wheel --qmake=c:\Qt\5.12\bin\qmake.exe
        --cmake=c:\tools\cmake\bin\cmake.exe
        --openssl=c:\libs\OpenSSL32bit\bin
  ```
* Then, we create a bdist_egg reusing the PySide2 build with option
  `--only-package`:
  ```
  python setup.py bdist_egg --only-package
        --qmake=c:\Qt\5.15\bin\qmake.exe
        --cmake=c:\tools\cmake\bin\cmake.exe
        --openssl=c:\libs\OpenSSL32bit\bin
  ```

You can use the option `--qt-conf-prefix` to pass a path relative to the
PySide2 installed package, which will be embedded into an auto-generated
`qt.conf` registered in the Qt resource system.  This path will serve as the
PrefixPath for QLibraryInfo, thus allowing to choose where Qt plugins should be
loaded from.  This option overrides the usual prefix chosen by `--standalone`
option, or when building on Windows.

To temporarily disable registration of the internal `qt.conf` file, a new
environment variable called PYSIDE_DISABLE_INTERNAL_QT_CONF is introduced.

You should assign the integer "1" to disable the internal `qt.conf`, or "0" (or
leave empty) to keep using the internal `qt.conf` file.

## Development Options

For development purposes the following options might be of use, when
using `setup.py build`:

 * `--ignore-git`, will skip the fetching and checkout steps for supermodule
   and all submodules.
 * `--limited-api=yes|no`, default yes if applicable. Set or clear the limited
   API flag. Ignored for Python 2.
 * `--module-subset`, allows for specifying the Qt modules to be built.
   A minimal set is: `--module-subset=Core,Gui,Test,Widgets`.
 * `--package-timestamp`, allows specifying the timestamp that will be used as
   part of the version number for a snapshot package.
   For example given `--package-timestamp=1529646276` the package version will
   be `5.x.y.dev1529646276`.
 * `--reuse-build`, option allows recompiling only the modified sources and not
   the whole world, shortening development iteration time.
 * `--sanitize-address`, will build the project with address sanitizer.
 * `--skip-cmake`, will reuse the already generated Makefiles (or equivalents),
   instead of invoking, CMake to update the Makefiles (note, CMake should be
   ran at least once to generate the files).
 * `--skip-docs`, skip the documentation generation.
 * `--skip-make-install`, will not run make install (or equivalent) for each
   module built.
 * `--skip-modules`, allows for specifying the Qt modules that will be skipped
   during the build process.
   For example: `--skip-modules=WebEngineCore,WebEngineWidgets`
 * `--skip-packaging`, will skip creation of the python package, enabled (Linux
   or macOS only).
 * `--verbose-build`, will output the compiler invocation with command line
   arguments, etc.

## Requirements

 * Python 3.6+ are supported,
 * CMake: Specify the path to cmake with `--cmake` option or add cmake to the
   system path.
 * Qt 5.12+ is supported. Specify the path to qmake with `--qmake` option or
   add qmake to the system path.

### Optional

#### OpenSSL:

Specifying the `--openssl` option only affects Windows. It is a no-op for other
platforms.

Please note that official Windows packages do not ship the OpenSSL libraries
due to import/export restrictions as described in
http://doc.qt.io/qt-5/ssl.html#import-and-export-restrictions

You can specify the location of the OpenSSL DLLs with the following option:
`--openssl=</path/to/openssl/bin-directory>`.

You can download
[OpenSSL for Windows here](http://slproweb.com/products/Win32OpenSSL.html)

Official Qt packages do not link to the SSL library directly, but rather try to
find the library at runtime.

On Windows, official Qt builds will try to pick up OpenSSL libraries at
application path, system registry, or in the PATH environment variable.

On macOS, official Qt builds use SecureTransport (provided by OS) instead of
OpenSSL.

On Linux, official Qt builds will try to pick up the system OpenSSL library.

> **Note**: this means that Qt packages that directly link to the OpenSSL
> shared libraries, are not currently compatible with standalone PySide2
> packages.

#### macOS SDK:

You can specify which macOS SDK should be used for compilation with the option
`--macos-sysroot=</path/to/sdk>`, for example:
```
--macos-sysroot=/Applications/Xcode.app/.../Developer/SDKs/MacOSX10.12.sdk/
```

#### macOS minimum deployment target:

You can specify a custom macOS minimum deployment target with the option
`--macos-deployment-target=<value>`, for example:
```
--macos-deployment-target=10.10
```

If the option is not set, the minimum deployment target of the used Qt library
will be used instead. Thus it is not necessary to use the option without a good
reason.

If a new value is specified, it has to be higher or equal to both Python's and
Qt's minimum deployment targets.

Description: macOS allows specifying a minimum OS version on which a binary
will be able to run. This implies that an application can be built on a machine
with the latest macOS version installed, with latest Xcode version and SDK
version and the built application can still run on an older OS version.


## CMake super project

For development convenience, a CMake super project is included in the root of
the repository.

The super project can be built using standalone CMake, or using an IDE's CMake
integration (Qt Creator for example).

Nevertheless the default build process is done via setup.py, in which case each
of the sub-projects are built and installed separately, as mentioned, the super
project is just for development convenience.

## IDE (Qt Creator) case

When using an IDE, just open the root CMakeLists.txt file as a new project, and
make sure to specify the following things:

 * `LLVM_INSTALL_DIR`, the environment variable should point to your libclang
   library location
 * `Qt`, either select a Qt Kit when configuring the project, or make sure that
   the qmake binary is present in the PATH environment variable.
 * `Python`, the PATH environment variable should also point to the Python
   interpreter which you wish to use for building the projects (can either be
   a system interpreter, or a virtualenv one for example)

Once that is done, just re-run CMake, so that it picks up the new environment
values.  If needed, all other cache variables defined by the project files can
be re-adjusted (for example FORCE_LIMITED_API).

## Command line CMake case

When building using the command line CMake binary, make sure to invoke it in
a separate build directory, and not in the root source directory.

Make sure you have the correct environment variables set up, as described in
the previous section.

The invocation would then look like:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
# make or nmake or msbuild or jom
```
