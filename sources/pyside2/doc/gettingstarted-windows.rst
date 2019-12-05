Getting Started on Windows
==========================

The Qt library has to be built with the same version of MSVC as Python and PySide2, this can be
selected when using the online installer.

Requirements
------------

 * Qt package from `here`_ or a custom build of Qt (preferably Qt 5.12 or greater)
 * A Python interpreter (version Python 3.5+).

  * Preferably get python from the `official website`_.

  .. note:: Python 2.7 interpreter is not supported.
      The official Python 2.7 binary package which can be downloaded at
      https://www.python.org/downloads/ is built using MSVC 2007, while
      the Qt libraries are built using MSVC 2015/2017.
      Note that if you build your own custom Python2.7 interpreter with
      an MSVC version equivalent to the one that Qt was built with,
      you can safely build and use Qt for Python against that interpreter.

 * `MSVC2017`_ (or MSVC2019) for Python 3 on Windows,
 * `CMake`_  version 3.1 or greater
 * `Git`_ version 2 or greater
 * `libclang_` from the `precompiled Qt packages`_ is recommended.
 * `OpenSSL`_ (optional for SSL support, Qt must have been configured using the same SSL library)
 * ``virtualenv`` is strongly recommended, but optional.
 * ``sphinx`` package for the documentation (optional).

.. _here: https://qt.io/download
.. _official website: https://www.python.org/downloads/
.. _MSVC2017: https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools
.. _CMake: https://cmake.org/download/
.. _Git: https://git-scm.com/download/win
.. _libclang: http://download.qt.io/development_releases/prebuilt/libclang/
.. _OpenSSL: https://sourceforge.net/projects/openssl/


Building from source on Windows 10
----------------------------------

Creating a virtual environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``virtualenv`` allows you to create a local, user-writeable copy of a python environment into
which arbitrary modules can be installed and which can be removed after use::

    virtualenv testenv
    call testenv\scripts\activate
    pip install sphinx  # optional: documentation
    pip install numpy PyOpenGL  # optional: for examples

will create and use a new virtual environment, which is indicated by the command prompt changing.

Setting up CLANG
~~~~~~~~~~~~~~~~

If you don't have libclang already in your system, you can download from the Qt servers,
e.g. ``libclang-release_60-windows-vs2015_64-clazy.7z``.

Extract the files, and leave it on any desired path, e.g ``c:\``, and then set these two required
environment variables::

    set LLVM_INSTALL_DIR=c:\libclang
    set PATH=C:\libclang\bin;%PATH%

Getting PySide2
~~~~~~~~~~~~~~~

Cloning the official repository can be done by::

    git clone --recursive https://code.qt.io/pyside/pyside-setup

Checking out the version that we want to build, e.g. 5.14::

    cd pyside-setup && git checkout 5.14

.. note:: Keep in mind you need to use the same version as your Qt installation

Building PySide2
~~~~~~~~~~~~~~~~

Check your Qt installation path, to specifically use that version of qmake to build PySide2.
e.g. ``E:\Qt\5.12.0\msvc2015_64\bin\qmake.exe``.

Build can take a few minutes, so it is recommended to use more than one CPU core::

    python setup.py build --qmake=c:\path\to\qmake.exe --openssl=c:\path\to\openssl\bin --build-tests --ignore-git --parallel=8

Installing PySide2
~~~~~~~~~~~~~~~~~~

To install on the current directory, just run::

    python setup.py install --qmake=c:\path\to\qmake.exe  --openssl=c:\path\to\openssl\bin --build-tests --ignore-git --parallel=8

Test installation
~~~~~~~~~~~~~~~~~

You can execute one of the examples to verify the process is properly working.
Remember to properly set the environment variables for Qt and PySide2::

    python examples/widgets/widgets/tetrix.py
