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

Installing prerequisites
------------------------

Install latest ``pip`` distribution: download `get-pip.py
<https://raw.github.com/pypa/pip/master/contrib/get-pip.py>`_ and run it using
the ``python`` interpreter.

Installing PySide on a Windows System
-------------------------------------

To install PySide on Windows you can choose from the following options:

#. Use pip to install the ``wheel`` binary packages:
   
   ::

      pip install -U PySide --use-wheel -f http://download.qt-project.org/official_releases/pyside/

#. Use setuptools to install the ``egg`` binary packages (deprecated):
   
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

#. Use pip to install the ``wheel`` binary packages:
   
   ::

      $ pip install -U PySide --use-wheel -f http://download.qt-project.org/official_releases/pyside/

#. Use setuptools to install the ``egg`` binary packages (deprecated):
   
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

We do not provide binaries for Linux. Please read the build instructions in section
``Building PySide on a Linux System``.


Building PySide on a Windows System
===================================

Windows: Installing prerequisites
---------------------------------

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

#. Install latest ``pip`` distribution into the Python you
   installed in the first step: download `get-pip.py 
   <https://raw.github.com/pypa/pip/master/contrib/get-pip.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      c:\> c:\Python27\python get-pip.py

#. Install latest `wheel` distribution:

   ::

      c:\> c:\Python27\Scripts\pip install wheel


Windows: Building PySide distribution
-------------------------------------

#. Download and extract `PySide source distribution
   <https://pypi.python.org/packages/source/P/PySide/PySide-1.2.2.tar.gz>`_

#. Switch to the distribution directory:

   ::

      c:\> cd PySide-1.2.2

#. Build the ``wheel`` binary distribution:

   ::

      c:\> c:\Python27\python.exe setup.py bdist_wheel --qmake=c:\Qt\4.8.5\bin\qmake.exe --openssl=c:\OpenSSL32bit\bin


Windows: Building PySide distribution from a Git repository
-----------------------------------------------------------

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


Windows: Installing PySide distribution
---------------------------------------

#. After the successful build, install the distribution with ``pip``:
   
   ::

      c:\> c:\Python27\Scripts\pip install --use-wheel dist\PySide-1.2.2-cp27-none-win32.whl


Windows: Installing PySide distribution into ``virtual`` Python environment
---------------------------------------------------------------------------

#. Install latest ``virtualenv`` distribution:

   ::

      c:\> c:\Python27\Scripts\pip install virtualenv

#. Use ``virtualenv`` to make a workspace:

   ::

      c:\> c:\Python27\Scripts\virtualenv env

#. Switch to the ``env`` directory:

   ::

      c:\> cd env

#. Install the distribution with ``pip``:
   
   ::

      c:\> Scripts\pip install --use-wheel ..\dist\PySide-1.2.2-cp27-none-win32.whl


Building PySide on a Mac OS X System
====================================

Mac OS X is a Unix flavor, partially based upon 
`BSD Unix <http://en.wikipedia.org/wiki/Berkeley_Software_Distribution>`_.

The supported Mac OS X versions created by `Apple <http://www.apple.com/>`_ are

- OS X 10.6 *Snow Leopard*
- OS X 10.7 *Lion*
- OS X 10.8 *Mountain Lion*
- OS X 10.9 *Mavericks*

Mac OS X is a proprietary UNIX flavor of BSD Unix and only partially similar to
Linux. Therefore, the usual packages from Linux distributions cannot be used
without modifications.

There are several known package managers which provide support for Mac OS X, namely

- `MacPorts <http://www.macports.org/>`_
- `Fink <http://www.finkproject.org/>`_
- `Homebrew <http://brew.sh/>`_

The main purpose of all of these projects is to provide the missing Linux packages
for Mac OS X.

Throughout this tutorial, we are only using `Homebrew <http://brew.sh/>`_, because
it appears to be the most light-weight package manager available. All installations
are made to /usr/local/(bin|lib|include|shared) by simple symlinks.

But it should be easy to translate these instructions for the other, heavier package managers.


Mac OS X: Installing prerequisites
----------------------------------

#. Install Package Manager:

   ::
   
      $ ruby -e "$(curl -fsSL https://raw.github.com/Homebrew/homebrew/go/install)"

Follow the on-screen instructions to make adjustions, especially run
 
   ::
        
      $ brew doctor
      
Also see the `homebrew homepage <http://brew.sh/>`_ for further information

#. Install `Xcode <https://itunes.apple.com/en/app/xcode/id497799835?mt=12>`_ (optional):

Follow the on-screen instructions. If you selected any extensions to be installed,
wait for their completion before you proceed.

.. note::

    If you are using Mavericks, you can also use the Xcode Command Line Tools without actually installing Xcode
    (not tested, see this article: `How to Install Command Line Tools in OS X Mavericks (Without Xcode)
    <http://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/>`_).

#. Install the Xcode command Line Tools:

   After Xcode installation has finished, you can open a command shell and issue
   
   ::
   
      $ xcode-select --install
      
   This will open a dialog window with further instructions.
   After the command line tools are installed, you will not need to use Xcode again
   in order to set up PySide.

#. Install build dependencies:

   ::

      $ brew install python cmake qt

Remark: This installs ``Homebrew`` Python, which is fine for you as a single user.
If you are considering to build installers for external users, see the section
``About PySide Distributions``.

#. Install latest ``pip`` distribution into the Python you
   installed in the first step: download `get-pip.py 
   <https://raw.github.com/pypa/pip/master/contrib/get-pip.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      $ wget https://raw.github.com/pypa/pip/master/contrib/get-pip.py
      $ sudo python2.7 get-pip.py

.. note::
    
  There are situations with older Python versions, where the above procedure does not work.
  You can then use this last-resort work-around (tested)::
  
      $ wget https://bitbucket.org/pypa/setuptools/raw/bootstrap/ez_setup.py
      $ sudo python2.7 ez_setup.py
      $ sudo easy_install pip
      $ sudo pip install setuptools -U
      $ sudo pip install wheel -U


#. Install latest ``wheel`` distribution:

   ::

      $ sudo pip2.7 install wheel


Mac OS X: About PySide Distribution
-----------------------------------

If you want to build PySide for your own use, the above instructions are ok.

But when you are considering to build PySide for other versions or other users, you need
to be aware of the following caveat:

- Mac OS X has the concept of a ``MACOSX_DEPLOYMENT_TARGET``

- The current deployment targets which work with PySide are 10.6 to 10.9 .

- All binary installers from https://www.python.org are built with the sessing

    ::
    
        $ export MACOSX_DEPLOYMENT_TARGET=10.6  # Snow Leopard

- The default setting for the deployment target of an extension (like PySide)
  is always set to the value that was present at the build time of CPython.
  You can set the deployment target higher than that, but not below the
  OS X version that was set with your Python installation.
  
- Current distributions like Homebrew set the deployment target to the same
  value as the OS version they are built with. (I.E. 10.9 for Mavericks).
  
- Example: A PySide, built on Mavericks, will therefore not run on a Python that was built
  for Mountain Lion.

Recommendation:

- Use Homebrew's simplicity for your own machine. Do not use it for distributing.

- Use one of the `Python.org Distributions <https://www.python.org/downloads/>`_
  or 
  
- build your own Python, either from a tar archive (
  `Python 2.7 <https://www.python.org/ftp/python/2.7.6/Python-2.7.6.tgz>`_ or
  `Python 3.4 <https://www.python.org/ftp/python/3.4.0/Python-3.4.0.tgz>`_), or from a
  `Mercurial repository <https://docs.python.org/devguide/>`_ with an explicit setting of
  ``MACOSX_DEPLOYMENT_TARGET``.

Mac OS X: Building PySide distribution
--------------------------------------

#. Download ``PySide`` source distribution:

   ::

      $ wget https://pypi.python.org/packages/source/P/PySide/PySide-1.2.2.tar.gz

#. Extract the source distribution:

   ::

      $ tar -xvzf PySide-1.2.2.tar.gz

#. Switch to the distribution directory:

   ::

      $ cd PySide-1.2.2

#. Build the ``wheel`` binary distribution:

   ::

      $ python2.7 setup.py bdist_wheel


Mac OS X: Building PySide distribution from a Git repository
------------------------------------------------------------

#. Clone ``PySide`` setup scripts from git repository:

   ::

      $ git clone https://github.com/PySide/pyside-setup.git pyside-setup

#. Switch to the ``pyside-setup`` directory:

   ::

      $ cd pyside-setup

#. Build ``PySide`` distribution:

   ::

      $ python2.7 setup.py bdist_wheel --version=1.2.2

   ..  commented out, working on this
        #. Optionally you can build standalone version of distribution with embedded Qt libs:
        
           ::
        
              $ python2.7 setup.py bdist_wheel --version=1.2.2 --standalone

#. To build the development version of ``PySide`` distribution, ignore the --version parameter:

   ::

      $ python2.7 setup.py bdist_wheel


Mac OS X: Installing PySide distribution
----------------------------------------

#. After the successful build, install the distribution with ``pip``:
   
   ::

      $ sudo pip2.7 install --use-wheel dist/PySide-1.2.2-cp27-none-linux-x86_64.whl

#. Run the post-install script to finish the package configuration:
   
   ::

      $ sudo python2.7 pyside_postinstall.py -install


Mac OS X: Installing PySide distribution into ``virtual`` Python environment
----------------------------------------------------------------------------

#. Install latest ``virtualenv`` distribution:

   ::

      $ sudo pip2.7 virtualenv

#. Use ``virtualenv`` to make a workspace:

   ::

      $ virtualenv-2.7 env

#. Activate the virtual Python in the ``env`` directory:

   ::

      $ source env/bin/activate

#. Install the distribution with ``pip``:
   
   ::

      (env) $ pip install --use-wheel ../dist/PySide-1.2.2-cp27-none-linux-x86_64.whl

#. Run the post-install script to finish the package configuration:
   
   ::

      (env) $ pyside_postinstall.py -install
      
#. Leave the virtual environment (optional):

   ::
   
      (env) $ deactivate
      $ 


Building PySide on a Linux System (Ubuntu 12.04 - 14.04)
========================================================

Linux: Installing prerequisites
-------------------------------

#. Install build dependencies:

   ::

      $ sudo apt-get install build-essential git cmake libqt4-dev libphonon-dev python2.7-dev libxml2-dev libxslt1-dev qtmobility-dev

#. Install latest ``pip`` distribution into the Python you
   installed in the first step: download `get-pip.py 
   <https://raw.github.com/pypa/pip/master/contrib/get-pip.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      $ wget https://raw.github.com/pypa/pip/master/contrib/get-pip.py
      $ sudo python2.7 get-pip.py

#. Install latest ``wheel`` distribution:

   ::

      $ sudo pip2.7 install wheel


Linux: Building PySide distribution
-----------------------------------

#. Download ``PySide`` source distribution:

   ::

      $ wget https://pypi.python.org/packages/source/P/PySide/PySide-1.2.2.tar.gz

#. Extract the source distribution:

   ::

      $ tar -xvzf PySide-1.2.2.tar.gz

#. Switch to the distribution directory:

   ::

      $ cd PySide-1.2.2

#. Build the ``wheel`` binary distribution:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4

#. Optionally you can build standalone version of distribution with embedded Qt libs:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4 --standalone


Linux: Building PySide distribution from a Git repository
---------------------------------------------------------

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


Linux: Installing PySide distribution
-------------------------------------

#. After the successful build, install the distribution with ``pip``:
   
   ::

      $ sudo pip2.7 install --use-wheel dist/PySide-1.2.2-cp27-none-linux-x86_64.whl

#. Run the post-install script to finish the package configuration:
   
   ::

      $ sudo python2.7 pyside_postinstall.py -install


Linux: Installing PySide distribution into ``virtual`` Python environment
-------------------------------------------------------------------------

#. Install latest ``virtualenv`` distribution:

   ::

      $ sudo pip2.7 virtualenv

#. Use ``virtualenv`` to make a workspace:

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

Usage on a Linux/Mac OS X System
--------------------------------
    
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
  But adding multiple targets on the same command line works, so you can build eggs
  and wheels with one compilation.

``--cmake``
    Specify the path to cmake.
    Useful when the cmake is not in path.

``--standalone``
    When enabled, all required Qt libs will be included in PySide distribution.
    This option is allways enabled on Windows.
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
    Available values are ``msvc`` on Windows and ``make`` on Linux/Mac OS X.

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
