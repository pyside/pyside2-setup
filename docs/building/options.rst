.. _building_options:

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

``--parallel``
    Specify the number of parallel build jobs

``--jom``
    Use `jom <http://qt-project.org/wiki/jom>`_ instead of nmake with msvc

``--build-tests``
    Enable building the tests
