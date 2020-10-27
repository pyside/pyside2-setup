Getting Started on macOS
========================

Requirements
------------

 * Qt package from `here`_ or a custom build of Qt 5.12+ (preferably 5.15)
 * A Python interpreter (version Python 3.6+).
   You can use the one provided by HomeBrew, or you can get
   python from the `official website`_.
 * `XCode`_ 8.2 (macOS 10.11), 8.3.3 (macOS 10.12), 9 (macOS 10.13), 10.1 (macOS 10.14)
 * `CMake`_  version 3.1 or greater
 * Git version 2 or greater
 * `libclang`_ from your system or the prebuilt version from the ``Qt Downloads`` page is
   recommended. libclang10 is required for PySide 5.15.
 * ``sphinx`` package for the documentation (optional).
 * Depending on your OS, the following dependencies might also be required:

  * ``libgl-dev``,
  * ``python-dev``,
  * ``python-distutils``,
  * and ``python-setuptools``.

.. _XCode: https://developer.apple.com/xcode/
.. _here: https://qt.io/download
.. _official website: https://www.python.org/downloads/
.. _CMake: https://cmake.org/download/
.. _libclang: http://download.qt.io/development_releases/prebuilt/libclang/


Building from source
--------------------

Creating a virtual environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``venv`` module allows you to create a local, user-writeable copy of a python environment into
which arbitrary modules can be installed and which can be removed after use::

    python -m venv testenv  # your interpreter could be called 'python3'
    source testenv/bin/activate
    pip install -r requirements.txt  # General dependencies, documentation, and examples.

will create and use a new virtual environment, which is indicated by the command prompt changing.

Setting up CLANG
~~~~~~~~~~~~~~~~

If you don't have libclang already in your system, you can download from the Qt servers::

    wget http://download.qt.io/development_releases/prebuilt/libclang/libclang-release_100-based-mac.7z

Extract the files, and leave it on any desired path, and then set these two required
environment variables::

    7z x libclang-release_100-based-mac.7z
    export CLANG_INSTALL_DIR=$PWD/libclang

Getting PySide2
~~~~~~~~~~~~~~~

Cloning the official repository can be done by::

    git clone --recursive https://code.qt.io/pyside/pyside-setup

Checking out the version that we want to build, e.g. 5.15::

    cd pyside-setup && git checkout 5.15

.. note:: Keep in mind you need to use the same version as your Qt installation

Building PySide2
~~~~~~~~~~~~~~~~

Check your Qt installation path, to specifically use that version of qmake to build PySide2.
e.g. ``/opt/Qt/5.15.0/gcc_64/bin/qmake``.

Build can take a few minutes, so it is recommended to use more than one CPU core::

    python setup.py build --qmake=/opt/Qt/5.15.0/gcc_64/bin/qmake --build-tests --ignore-git --parallel=8

Installing PySide2
~~~~~~~~~~~~~~~~~~

To install on the current directory, just run::

    python setup.py install --qmake=/opt/Qt/5.15.0/gcc_64/bin/qmake --build-tests --ignore-git --parallel=8

Test installation
~~~~~~~~~~~~~~~~~

You can execute one of the examples to verify the process is properly working.
Remember to properly set the environment variables for Qt and PySide2::

    python examples/widgets/widgets/tetrix.py
