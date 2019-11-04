# PySide2

### Introduction

PySide2 is the official Python module from the
[Qt for Python project](http://wiki.qt.io/Qt_for_Python),
which provides access to the complete Qt 5.12+ framework.

The Qt for Python project is developed in the open, with all facilities you'd expect
from any modern OSS project such as all code in a git repository and an open
design process. We welcome any contribution conforming to the
[Qt Contribution Agreement](https://www.qt.io/contributionagreement/).

### Installation

Since the release of the [Technical Preview](https://blog.qt.io/blog/2018/06/13/qt-python-5-11-released/)
it is possible to install via `pip`, both from Qt's servers
and [PyPi](https://pypi.org/project/PySide2/):

    pip install PySide2

#### Dependencies

PySide2 versions following 5.12 use a C++ parser based on
[Clang](http://clang.org/). The Clang library (C-bindings), version 6.0 or
higher is required for building. Prebuilt versions of it can be downloaded from
[download.qt.io](http://download.qt.io/development_releases/prebuilt/libclang/).

After unpacking the archive, set the environment variable *LLVM_INSTALL_DIR* to
point to the folder containing the *include* and *lib* directories of Clang:

    7z x .../libclang-release_60-linux-Rhel7.2-gcc5.3-x86_64-clazy.7z
    export LLVM_INSTALL_DIR=$PWD/libclang

On Windows:

    7z x .../libclang-release_60-windows-vs2015_64-clazy.7z
    SET LLVM_INSTALL_DIR=%CD%\libclang

### Building from source

For building PySide2 from scratch, please read about
[getting started](https://wiki.qt.io/Qt_for_Python/GettingStarted).
This process will include getting the code:

    git clone https://code.qt.io/pyside/pyside-setup
    cd pyside-setup
    git branch --track 5.12 origin/5.12
    git checkout 5.12

then install the dependencies, and following the instructions per platform.
A common build command will look like:

    python setup.py install --qmake=<path/to/qmake/> --parallel=8 --build-tests

You can obtain more information about the options to build PySide
and Shiboken in [our wiki](https://wiki.qt.io/Qt_for_Python/).

### Documentation and Bugs

You can find more information about the PySide2 module API in the
[official Qt for Python documentation](https://doc.qt.io/qtforpython/).

If you come across any issue, please file a bug report at our
[JIRA tracker](https://bugreports.qt.io/projects/PYSIDE) following
our [guidelines](https://wiki.qt.io/Qt_for_Python/Reporting_Bugs).

### Community

Check *#qt-pyside*, our official IRC channel on FreeNode,
or contact us via our [mailing list](http://lists.qt-project.org/mailman/listinfo/pyside).

### Licensing

PySide2 is available under both Open Source (LGPLv3/GPLv2) and commercial license.
Using PyPi is the recommended installation source, because the content of the wheels is valid for both cases.
For more information, refer to the [Qt Licensing page](https://www.qt.io/licensing/).
