======
PySide
======

.. contents:: **Table of Contents** 

Introduction
============

PySide is the Python Qt bindings project, providing access the complete Qt 4.8 framework
as well as to generator tools for rapidly generating bindings for any C++ libraries.

The PySide project is developed in the open, with all facilities you'd expect
from any modern OSS project such as all code in a git repository, an open
Bugzilla for reporting bugs, and an open design process. We welcome
any contribution without requiring a transfer of copyright.

The PySide documentation is hosted at `http://pyside.github.io/docs/pyside/
<http://pyside.github.io/docs/pyside/>`_.

Compatibility
=============

PySide requires Python 2.6 or later and Qt 4.6 or better. Qt 5.x is currently not supported.

Installation
============

Installing prerequisities
-------------------------

Install latest `pip` distribution: download `get-pip.py
<https://raw.github.com/pypa/pip/master/contrib/get-pip.py>`_ and run it using
the ``python`` interpreter.

Installing PySide on a Windows System
-------------------------------------

To install PySide on Windows you can choose from the following options:

#. Use pip to install the `wheel` binary packages:
   
   ::

      pip install -U PySide --use-wheel -f http://download.qt-project.org/official_releases/pyside/

#. Use setuptools to install the `egg` binary packages (deprecated):
   
   ::

      easy_install -U PySide

#. Download and install the packages from the `releases page
   <http://qt-project.org/wiki/PySide_Binaries_Windows>`_.

.. note::

  Provided binaries are without any other external dependencies.
  All required Qt libraries, development tools and examples are included.


Installing PySide on a Mac OS X System
--------------------------------------

You need to install or build Qt 4.8 first, see the `Qt Project Documentation
<http://qt-project.org/doc/qt-4.8/install-mac.html>`_.

Alternatively you can use `Homebrew <http://brew.sh/>`_ and install Qt with

   ::
   
      $ brew install qt

To install PySide on Mac OS X you can choose from the following options:

#. Use pip to install the `wheel` binary packages:
   
   ::

      $ pip install -U PySide --use-wheel -f http://download.qt-project.org/official_releases/pyside/

#. Use setuptools to install the `egg` binary packages (deprecated):
   
   ::

      $ easy_install -U PySide

After the installation, the following call must be made manually:

   ::
   
      $ pyside_postinstall.py -install
      
If for some reason the script is not callable, it can alternatively be
run directly by:

   ::
   
      $ python $(which pyside_postinstall.py) -install


Installing PySide on a Linux System
-----------------------------------

We do not provide binaries for a Linux System. Please read the build instructions in section
``Building PySide on a Linux System``.

Building PySide on a Windows System
===================================

Installing prerequisities
-------------------------

#. Install `Python
   <http://www.python.org/download/>`_.

#. Install `Qt 4.8 libraries for Windows VS 2008 edition
   <http://download.qt-project.org/official_releases/qt/4.8/4.8.5/qt-win-opensource-4.8.5-vs2008.exe>`_
   when building against Python 2.6, 2.7 or 3.2.
   Install `Qt 4.8 libraries for Windows VS 2010 edition
   <http://download.qt-project.org/official_releases/qt/4.8/4.8.5/qt-win-opensource-4.8.5-vs2010.exe>`_
   when building against Python 3.3 or 3.4.

#. Install `Cmake
   <http://www.cmake.org/cmake/resources/software.html>`_.

#. Install `Windows SDK v7.0
   <http://www.microsoft.com/en-us/download/details.aspx?id=3138>`_
   when building against Python 2.6, 2.7 or 3.2.
   Install `Windows SDK v7.1
   <http://www.microsoft.com/en-us/download/details.aspx?id=8279>`_
   when building against Python 3.3 or 3.4.

#. Install `Git
   <http://git-scm.com/download/win>`_.

#. (Optional) Install `OpenSSL
   <http://slproweb.com/products/Win32OpenSSL.html>`_.

#. Install latest `pip` distribution into the Python you
   installed in the first step: download `get-pip.py 
   <https://raw.github.com/pypa/pip/master/contrib/get-pip.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      c:\> c:\Python27\python get-pip.py

#. Install latest `wheel` distribution:

   ::

      c:\> c:\Python27\Scripts\pip install wheel

Building PySide distribution
----------------------------

#. Download and extract `PySide source distribution
   <https://pypi.python.org/packages/source/P/PySide/PySide-1.2.2.tar.gz>`_

#. Switch to the distribution directory:

   ::

      c:\> cd PySide-1.2.2

#. Build the `wheel` binary distribution:

   ::

      c:\> c:\Python27\python.exe setup.py bdist_wheel --qmake=c:\Qt\4.8.5\bin\qmake.exe --openssl=c:\OpenSSL32bit\bin

Building PySide distribution from git repository
------------------------------------------------

#. Clone ``PySide`` setup scripts from git repository:

   ::

      c:\> git clone https://github.com/PySide/pyside-setup.git pyside-setup

#. Switch to the ``pyside-setup`` directory:

   ::

      c:\> cd pyside-setup

#. Build the `wheel` binary distribution:

   ::

      c:\> c:\Python27\python.exe setup.py bdist_wheel --version=1.2.2 --qmake=c:\Qt\4.8.5\bin\qmake.exe --openssl=c:\OpenSSL32bit\bin

#. To build the development version of ``PySide`` distribution, ignore the --version parameter:

   ::

      c:\> c:\Python27\python.exe setup.py bdist_wheel --qmake=c:\Qt\4.8.5\bin\qmake.exe --openssl=c:\OpenSSL32bit\bin

Installing PySide distribution
------------------------------

#. After the successful build, install the distribution with ``pip``:
   
   ::

      c:\> c:\Python27\Scripts\pip install --use-wheel dist\PySide-1.2.2-cp27-none-win32.whl

Installing PySide distribution into ``virtual`` Python environment
------------------------------------------------------------------

#. Install latest `virtualenv` distribution:

   ::

      c:\> c:\Python27\Scripts\pip install virtualenv

#. Use `virtualenv` to make a workspace:

   ::

      c:\> c:\Python27\Scripts\virtualenv env

#. Switch to the ``env`` directory:

   ::

      c:\> cd env

#. Install the distribution with ``pip``:
   
   ::

      c:\> Scripts\pip install --use-wheel ..\dist\PySide-1.2.2-cp27-none-win32.whl

Building PySide on a Linux System (Ubuntu 12.04 - 14.04)
========================================================

Installing prerequisities
-------------------------

#. Install build dependencies:

   ::

      $ sudo apt-get install build-essential git cmake libqt4-dev libphonon-dev python2.7-dev libxml2-dev libxslt1-dev qtmobility-dev

#. Install latest `pip` distribution into the Python you
   installed in the first step: download `get-pip.py 
   <https://raw.github.com/pypa/pip/master/contrib/get-pip.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      $ wget https://raw.github.com/pypa/pip/master/contrib/get-pip.py
      $ sudo python2.7 get-pip.py

#. Install latest `wheel` distribution:

   ::

      $ sudo pip2.7 install wheel

Building PySide distribution
----------------------------

#. Download ``PySide`` source distribution:

   ::

      $ wget https://pypi.python.org/packages/source/P/PySide/PySide-1.2.2.tar.gz

#. Extract the source distribution:

   ::

      $ tar -xvzf PySide-1.2.2.tar.gz

#. Switch to the distribution directory:

   ::

      $ cd PySide-1.2.2

#. Build the `wheel` binary distribution:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4

#. Optionally you can build standalone version of distribution with embedded Qt libs:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4 --standalone

Building PySide distribution from git repository
------------------------------------------------

#. Clone ``PySide`` setup scripts from git repository:

   ::

      $ git clone https://github.com/PySide/pyside-setup.git pyside-setup

#. Switch to the ``pyside-setup`` directory:

   ::

      $ cd pyside-setup

#. Build ``PySide`` distribution:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4 --version=1.2.2

#. Optionally you can build standalone version of distribution with embedded Qt libs:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4 --version=1.2.2 --standalone

#. To build the development version of ``PySide`` distribution, ignore the --version parameter:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4

Installing PySide distribution
------------------------------

#. After the successful build, install the distribution with ``pip``:
   
   ::

      $ sudo pip2.7 install --use-wheel dist/PySide-1.2.2-cp27-none-linux-x86_64.whl

#. Run the post-install script to finish the package configuration:
   
   ::

      $ sudo python2.7 pyside_postinstall.py -install

Installing PySide distribution into ``virtual`` Python environment
------------------------------------------------------------------

#. Install latest `virtualenv` distribution:

   ::

      $ sudo pip2.7 virtualenv

#. Use `virtualenv` to make a workspace:

   ::

      $ virtualenv-2.7 env

#. Switch to the ``env`` directory:

   ::

      $ cd env

#. Install the distribution with ``pip``:
   
   ::

      $ bin/pip2.7 install --use-wheel ../dist/PySide-1.2.2-cp27-none-linux-x86_64.whl

#. Run the post-install script to finish the package configuration:
   
   ::

      $ bin/python bin/pyside_postinstall.py -install


PySide Setup Script command line options
========================================

Usage on a Windows System
-------------------------
    
   ::

      c:\> c:\Python27\python.exe setup.py [distribution_type] [options]

Usage on a Linux System
-----------------------
    
   ::

      python2.7 setup.py [distribution_type] [options]

Distribution types
------------------

``bdist_wheel``
    Create a wheel binary distribution.
    This distribution type can be installed with ``pip``.

``bdist_egg``
    Create an egg binary distribution.
    This distribution type can be installed with ``easy_install``.

``bdist_wininst``
    Create a standalone windows installer with embedded Qt libs and development tools.
    This distribution type can be installed with ``easy_install``.

``install``
    Install package to site packages folder.

``develop``
    Install package in ``development mode``, such that it's available on
    ``sys.path``, yet can still be edited directly from its source folder.

``sdist``
    Create a full source distribution with included sources of PySide Setup Scripts,
    PySide, Shiboken, PySide Tools and PySide Examples.
    Can be used to build binary distribution in offline mode.

Options
-------

``--qmake``
    Specify the path to qmake.
    Useful when the qmake is not in path or more than one Qt versions are installed.

``--openssl``
    Specify the path to OpenSSL libs.

``--only-package``
    Skip rebuilding everything and create distribution from prebuilt binaries.
    Before using this option first time, the full distribution build is required.

.. note::

  This option is broken on Mac OS X and fails to produce a usable distribution.


``--cmake``
    Specify the path to cmake.
    Useful when the cmake is not in path.

``--standalone``
    When enabled, all required Qt libs will be included in PySide distribution.
    This option is allways enabled on Windows System.
    On Linux it's disabled by default.

.. note::

  This option does not work on Mac OS X, yet.


``--version``
    Specify what version of PySide distribution to build.
    This option is available only when the setup scripts are cloned from git repository.

``--list-versions``
    List available versions of PySide distributions.

``--ignore-git``
    Don't pull sources from git repository.

``--make-spec``
    Specify the cmake makefile generator type.
    Available values are ``msvc`` on Windows System and ``make`` on a Linux/OS X System.

``--no-examples``
    Don't include PySide examples in PySide distribution

``--jobs``
    Specify the number of parallel build jobs

``--jom``
    Use `jom <http://qt-project.org/wiki/jom>`_ instead of nmake with msvc

``--build-tests``
    Enable building the tests

Feedback and getting involved
=============================

- Mailing list: http://lists.qt-project.org/mailman/listinfo/pyside
- Issue tracker: https://bugreports.qt-project.org/browse/PYSIDE
- Code Repository: http://qt.gitorious.org/pyside
