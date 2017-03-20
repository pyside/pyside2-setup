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

You might consider using a virtual environment as described at
[getting started](https://wiki.qt.io/PySide2_GettingStarted).
You should be able to build:

    cd pyside-setup
    python setup.py install

The setup script will try to find the location of the qmake tool of the Qt
version to be used and the cmake build tool in the path. Non-standard
locations can be specified by the *--qmake=path_to_qmake* or
*--cmake=path_to_cmake* command line options.
