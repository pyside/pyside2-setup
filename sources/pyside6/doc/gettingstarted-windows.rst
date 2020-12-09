Getting Started on Windows
==========================

The Qt library has to be built with the same version of MSVC as Python and PySide, this can be
selected when using the online installer.

Requirements
------------

 * Qt package from `here`_ or a custom build of Qt (preferably 6.0)
 * A Python interpreter (version Python 3.6+). Preferably get it from the `official website`_.
 * `MSVC2017`_ (or MSVC2019) for Python 3 on Windows,
 * `CMake`_  version 3.1 or greater
 * `Git`_ version 2 or greater
 * `libclang`_ prebuilt version from the ``Qt Downloads`` page is recommended. We recommend
   libclang10 for 6.0+.
 * `OpenSSL`_ (optional for SSL support, Qt must have been configured using the same SSL library).
 * ``venv`` or ``virtualenv`` is strongly recommended, but optional.
 * ``sphinx`` package for the documentation (optional).

.. note:: Python 3.8.0 was missing some API required for PySide/Shiboken so it's not possible
    to use it for a Windows build.


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

The ``venv`` module allows you to create a local, user-writeable copy of a python environment into
which arbitrary modules can be installed and which can be removed after use::

    python -m venv testenv
    call testenv\Scripts\activate
    pip install -r requirements.txt  # General dependencies, documentation, and examples.

will create and use a new virtual environment, which is indicated by the command prompt changing.

Setting up CLANG
~~~~~~~~~~~~~~~~

If you don't have libclang already in your system, you can download from the Qt servers,
for example, ``libclang-release_100-based-windows-vs2019_64.7z``.

Extract the files, and leave it on any desired path, for example, ``c:``, and then set these two
required environment variables::

    set LLVM_INSTALL_DIR=c:\libclang
    set PATH=C:\libclang\bin;%PATH%

Getting PySide
~~~~~~~~~~~~~~

Cloning the official repository can be done by::

    git clone --recursive https://code.qt.io/pyside/pyside-setup

Checking out the version that we want to build, for example,  6.0::

    cd pyside-setup && git checkout 6.0

.. note:: Keep in mind you need to use the same version as your Qt installation

Building PySide
~~~~~~~~~~~~~~~

Check your Qt installation path, to specifically use that version of qmake to build PySide.
for example, ``E:\Qt\6.0.0\msvc2019_64\bin\qmake.exe``.

Build can take a few minutes, so it is recommended to use more than one CPU core::

    python setup.py build --qmake=c:\path\to\qmake.exe --openssl=c:\path\to\openssl\bin --build-tests --ignore-git --parallel=8

Installing PySide
~~~~~~~~~~~~~~~~~

To install on the current directory, just run::

    python setup.py install --qmake=c:\path\to\qmake.exe  --openssl=c:\path\to\openssl\bin --build-tests --ignore-git --parallel=8

Test installation
~~~~~~~~~~~~~~~~~

You can execute one of the examples to verify the process is properly working.
Remember to properly set the environment variables for Qt and PySide::

    python examples/widgets/widgets/tetrix.py
