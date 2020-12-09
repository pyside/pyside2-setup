Getting Started on Linux
==========================

Requirements
------------

 * Qt package from `here`_ or a custom build of Qt (preferably 6.0)
 * A Python interpreter (version Python 3.6+).
   You can either use the one provided by your OS, or get it
   from the `official website`_.
 * GCC
 * `CMake`_  version 3.1 or greater
 * Git version 2 or greater
 * `libclang`_ from your system or the prebuilt version from the ``Qt Downloads`` page is
   recommended. libclang10 is required for 6.0+.
 * ``sphinx`` package for the documentation (optional).
 * Depending on your linux distribution, the following dependencies might also be required:

    * ``libgl-dev``,
    * ``python-dev``,
    * ``python-distutils``,
    * and ``python-setuptools``.

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

    python -m venv testenv
    source testenv/bin/activate
    pip install -r requirements.txt  # General dependencies, documentation, and examples.

will create and use a new virtual environment, which is indicated by the command prompt changing.

Setting up CLANG
~~~~~~~~~~~~~~~~

If you don't have libclang already in your system, you can download from the Qt servers::

    wget https://download.qt.io/development_releases/prebuilt/libclang/libclang-release_100-based-linux-Rhel7.6-gcc5.3-x86_64.7z

Extract the files, and leave it on any desired path, and then set these two required
environment variables::

    7z x libclang-release_100-based-linux-Rhel7.6-gcc5.3-x86_64.7z
    export CLANG_INSTALL_DIR=$PWD/libclang

Getting PySide
~~~~~~~~~~~~~~

Cloning the official repository can be done by::

    git clone --recursive https://code.qt.io/pyside/pyside-setup

Checking out the version that we want to build, for example 6.0::

    cd pyside-setup && git checkout 6.0

.. note:: Keep in mind you need to use the same version as your Qt installation.
          Additionally, ``git checkout -b 6.0  --track origin/6.0`` could be a better option
          in case you want to work on it.

Building PySide
~~~~~~~~~~~~~~~

Check your Qt installation path, to specifically use that version of qmake to build PySide.
for example, ``/opt/Qt/6.0.0/gcc_64/bin/qmake``.

Build can take a few minutes, so it is recommended to use more than one CPU core::

    python setup.py build --qmake=/opt/Qt/6.0.0/gcc_64/bin/qmake --build-tests --ignore-git --parallel=8

Installing PySide
~~~~~~~~~~~~~~~~~

To install on the current directory, just run::

    python setup.py install --qmake=/opt/Qt/6.0.0/gcc_64/bin/qmake --build-tests --ignore-git --parallel=8

Test installation
~~~~~~~~~~~~~~~~~

You can execute one of the examples to verify the process is properly working.
Remember to properly set the environment variables for Qt and PySide::

    python examples/widgets/widgets/tetrix.py
