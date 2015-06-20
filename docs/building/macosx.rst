.. _building_macosx:

Building PySide on a Mac OS X System
====================================

Mac OS X is a Unix flavor, partially based upon 
`BSD Unix <http://en.wikipedia.org/wiki/Berkeley_Software_Distribution>`_.

The supported Mac OS X versions created by `Apple <http://www.apple.com/>`_ are

- OS X 10.6 *Snow Leopard*
- OS X 10.7 *Lion*
- OS X 10.8 *Mountain Lion*
- OS X 10.9 *Mavericks*
- OS X 10.10 *Yosemite*

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


Installing prerequisites
------------------------

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
   <https://bootstrap.pypa.io/get-pip.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      $ wget https://bootstrap.pypa.io/get-pip.py
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


About PySide Distribution
-------------------------

If you want to build PySide for your own use, the above instructions are ok.

But when you are considering to build PySide for other versions or other users, you need
to be aware of the following caveat:

- Mac OS X has the concept of a ``MACOSX_DEPLOYMENT_TARGET``

- The current deployment targets which work with PySide are 10.6 to 10.9 .

- All binary installers from https://www.python.org are built with the setting

::

   $ export MACOSX_DEPLOYMENT_TARGET=10.6  # Snow Leopard

- The default setting for the deployment target of an extension (like PySide)
  is always inherited from the Python used for building.
  You can set the deployment target higher than that, but not below the
  OS X version that was set during building your Python installation.
  
- Current distributions like Homebrew set the deployment target to the same
  value as the OS version they are built with. (I.E. 10.9 for Mavericks).
  
- Example: A PySide, built on Mavericks, will therefore not run on a Python that was built
  for Mountain Lion.

Recommendation:

- Use Homebrew's simplicity for your own machine. Do not use it for distributing.

- Use one of the `Python.org Distributions <https://www.python.org/downloads/>`_
  or 
  
- Build your own Python, either from a tar archive (
  `Python 2.7 <https://www.python.org/ftp/python/2.7.6/Python-2.7.6.tgz>`_ or
  `Python 3.4 <https://www.python.org/ftp/python/3.4.0/Python-3.4.0.tgz>`_), or from a
  `Mercurial repository <https://docs.python.org/devguide/>`_ with an explicit setting of
  ``MACOSX_DEPLOYMENT_TARGET``.

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

#. Build the ``wheel`` binary distribution:

   ::

      $ python2.7 setup.py bdist_wheel


Building PySide distribution from a Git repository
--------------------------------------------------

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


Installing PySide distribution
------------------------------

#. After the successful build, install the distribution with ``pip``:

   ::

      $ sudo pip2.7 install dist/PySide-1.2.2-cp27-none-linux-x86_64.whl


Installing PySide distribution into ``virtual`` Python environment
------------------------------------------------------------------

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

      (env) $ pip install ../dist/PySide-1.2.2-cp27-none-linux-x86_64.whl

#. Leave the virtual environment (optional):

   ::

      (env) $ deactivate
      $ 
