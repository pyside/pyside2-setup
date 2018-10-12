# PySide2

### Introduction

PySide is the [Python Qt bindings project](http://wiki.qt.io/PySide2), providing
access to the complete Qt 5.x framework as well as to generator tools for rapidly
generating bindings for any C++ libraries.

The PySide project is developed in the open, with all facilities you'd expect
from any modern OSS project such as all code in a git repository and an open
design process. We welcome any contribution conforming to the
[Qt Contribution Agreement](https://www.qt.io/contributionagreement/).


PySide 2 supports Qt5. For building, please read about
[getting started](https://wiki.qt.io/PySide2_GettingStarted).
Then download the sources by running

    git clone https://code.qt.io/pyside/pyside-setup

### Building

#### Dependencies

PySide versions following 5.6 use a C++ parser based on
[Clang](http://clang.org/). The Clang library (C-bindings), version 3.9 or
higher is required for building. Prebuilt versions of it can be downloaded from
[download.qt.io](http://download.qt.io/development_releases/prebuilt/libclang/).

After unpacking the archive, set the environment variable *LLVM_INSTALL_DIR* to
point to the folder containing the *include* and *lib* directories of Clang:

    7z x .../libclang-release_39-linux-Rhel7.2-gcc5.3-x86_64.7z
    export LLVM_INSTALL_DIR=$PWD/libclang

On Windows:

    7z x .../libclang-release_39-windows-vs2015_64.7z
    SET LLVM_INSTALL_DIR=%CD%\libclang

#### Build Instructions

You might consider using a virtual environment as described at
[getting started](https://wiki.qt.io/PySide2_GettingStarted).
You should be able to build:

    cd pyside-setup
    python setup.py install

The setup script will try to find the location of the qmake tool of the Qt
version to be used and the cmake build tool in the path. Non-standard
locations can be specified by the *--qmake=path_to_qmake* or
*--cmake=path_to_cmake* command line options.
