#PySide2

| Windows | Linux |
|---------|-------|
| [![AppVeyor](https://img.shields.io/appveyor/ci/techtonik/pyside2-setup.svg)](https://ci.appveyor.com/project/techtonik/pyside2-setup) | [![Travis CI](https://img.shields.io/travis/PySide/pyside2-setup.svg)](https://travis-ci.org/PySide/pyside2-setup) |


###Introduction




PySide is the Python Qt bindings project, providing access the complete Qt 5.x framework
as well as to generator tools for rapidly generating bindings for any C++ libraries.

The PySide project is developed in the open, with all facilities you'd expect
from any modern OSS project such as all code in a git repository and an open design process. We welcome
any contribution without requiring a transfer of copyright.


PySide 2 supports Qt5. For building, please read about [getting the dependencies](https://github.com/PySide/pyside2/wiki/Dependencies). Then download the sources by running `git clone --recursive https://github.com/PySide/pyside2-setup.git`.

###Building

####Windows
On Windows, once you have gotten the dependencies and the source, `cd pyside2-setup.git` to enter the directory and then:
```
python setup.py install --qmake=\path\to\bin\qmake --cmake=\path\to\bin\cmake --openssl=\path\to\openssl\bin
```

####Linux

You should be able to build:

```
python setup.py install --qmake=/usr/bin/qmake --cmake=/usr/bin/cmake --openssl=/usr/bin/openssl
```

