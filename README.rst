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

Compatibility
=============

PySide requires Python 2.6 or later and Qt 4.6 or better.

Installation
============

Installing prerequisities
-------------------------

Install latest `setuptools` distribution: download `ez_setup.py
<https://bitbucket.org/pypa/setuptools/raw/bootstrap/ez_setup.py>`_ and run it using
the ``python`` interpreter.

Installing PySide on a Windows System
-------------------------------------

There are two options to install PySide on Windows:

#. Download and install the packages from the `releases page
   <http://qt-project.org/wiki/PySide_Binaries_Windows>`_.

#. Use setuptools:
   
   ::

      c:\> c:\Python27\Scripts\easy_install PySide

Installing PySide on a UNIX System
----------------------------------

There are no prebuild setuptools distributions available for UNIX System.
To build and install setuptools compatible distributions for UNIX System,
please read the instructions in section ``Building PySide on a UNIX System``.

Using pip vs easy_install
-------------------------

Pip can install only from source (it does not support binary distributions) and allways rebuilds the distribution
before the distribution is installed to system. For that reason the recommended tool to install the ``PySide``
is easy_install.

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
   when building against Python 3.3.

#. Install `Cmake
   <http://www.cmake.org/cmake/resources/software.html>`_.

#. Install `Windows SDK v7.0
   <http://www.microsoft.com/en-us/download/details.aspx?id=3138>`_
   when building against Python 2.6, 2.7 or 3.2.
   Install `Windows SDK v7.1
   <http://www.microsoft.com/en-us/download/details.aspx?id=8279>`_
   when building against Python 3.3.

#. Install `Git
   <http://git-scm.com/download/win>`_.

#. (Optional) Install `OpenSSL
   <http://slproweb.com/products/Win32OpenSSL.html>`_.

#. Install latest `setuptools` distribution into the Python you
   installed in the first step: download `ez_setup.py
   <https://bitbucket.org/pypa/setuptools/raw/bootstrap/ez_setup.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      c:\> c:\Python27\python ez_setup.py

Building PySide distribution
----------------------------

#. Clone ``PySide`` setup scripts from git repository:

   ::

      c:\> git clone https://github.com/PySide/pyside-setup.git pyside-setup

#. Switch to the ``pyside-setup`` directory:

   ::

      c:\> cd pyside-setup

#. Build ``PySide`` windows installer:

   ::

      c:\> c:\Python27\python.exe setup.py bdist_wininst --version=1.2.0 --qmake=c:\Qt\4.8.5\bin\qmake.exe --openssl=c:\OpenSSL32bit\bin

#. To build the development version of ``PySide`` windows installer, ignore the --version parameter:

   ::

      c:\> c:\Python27\python.exe setup.py bdist_wininst --qmake=c:\Qt\4.8.5\bin\qmake.exe --openssl=c:\OpenSSL32bit\bin

Installing PySide distribution
------------------------------

#. After the successful build, install the distribution with easy_install:
   
   ::

      c:\> c:\Python27\Scripts\easy_install dist\PySide-1.2.0.win32-py2.7.exe

Building PySide on a UNIX System (Ubuntu 12.04 LTS)
===================================================

Installing prerequisities
-------------------------

#. Install Python 2.7 header files and a static library:
    
   ::

      $ sudo apt-get install python2.7-dev
   
#. Install Qt 4.8 libraries:
    
   ::

      $ sudo apt-get install qt-sdk
   
#. Install cmake:
    
   ::

      $ sudo apt-get install cmake

#. Install git:
    
   ::

      $ sudo apt-get install git
   
#. Install latest `setuptools` distribution into the Python you
   installed in the first step: download `ez_setup.py
   <https://bitbucket.org/pypa/setuptools/raw/bootstrap/ez_setup.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      $ sudo python2.7 ez_setup.py

Building PySide distribution
----------------------------

#. Clone ``PySide`` setup scripts from git repository:

   ::

      $ git clone https://github.com/PySide/pyside-setup.git pyside-setup

#. Switch to the ``pyside-setup`` directory:

   ::

      $ cd pyside-setup

#. Build ``PySide`` distribution:

   ::

      $ python2.7 setup.py bdist_egg --version=1.2.0

#. Optionally you can build standalone version of distribution with embedded Qt libs:

   ::

      $ python2.7 setup.py bdist_egg --standalone --version=1.2.0

#. To build the development version of ``PySide`` distribution, ignore the --version parameter:

   ::

      $ python2.7 setup.py bdist_egg

Installing PySide distribution
------------------------------

#. After the successful build, install the distribution with easy_install
   and run the post-install script:
   
   ::

      $ sudo easy_install-2.7 dist/PySide-1.2.0-py2.7.egg
      $ sudo python2.7 pyside_postinstall.py -install

PySide Setup Script command line options
========================================

Usage on Windows System
-----------------------
    
   ::

      c:\> c:\Python27\python.exe setup.py [distribution_type] [options]

Usage on UNIX System
--------------------
    
   ::

      python2.7 setup.py [distribution_type] [options]

Distribution types
------------------

``bdist_wininst``
    Create standalone windows installer with embedded Qt libs and development tools.
    This distribution type can be installed with ``easy_install``.

``bdist_egg``
    Create egg binary distribution.
    This distribution type can be installed with ``easy_install``.

``install``
    Install package to site packages folder.

``develop``
    Install package in ``development mode``, such that it's available on
    ``sys.path``, yet can still be edited directly from its source folder.

``sdist``
    Create full source distribution with included sources of PySide Setup Scripts,
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

``--cmake``
    Specify the path to cmake.
    Useful when the cmake is not in path.

``--standalone``
    When enabled, all required Qt libs will be included in PySide distribution.
    This option is allways enabled on Windows System.
    On Linux it's disabled by default.

``--version``
    Specify what version of PySide distribution to build.
    This option is available only when the setup scripts are cloned from git repository.

``--list-versions``
    List available versions of PySide distributions.

``--ignore-git``
    Don't pull sources from git repository.

``--make-spec``
    Specify the cmake makefile generator type.
    Available values are ``msvc`` on Windows System and ``make`` on UNIX System.

``--no-examples``
    Don't include PySide examples in PySide distribution

``--jobs``
    Specify the number of parallel build jobs

``--jom``
    Use jom instead of nmake with msvc

``--build-tests``
    Enable building the tests

Feedback and getting involved
=============================

- Mailing list: http://lists.qt-project.org/mailman/listinfo/pyside
- Issue tracker: https://bugreports.qt-project.org/browse/PYSIDE
- Code Repository: http://qt.gitorious.org/pyside
