.. _building_linux:

Building PySide on a Linux System (Ubuntu 12.04 - 14.04)
========================================================

Installing prerequisites
------------------------

#. Install build dependencies:

   ::

      $ sudo apt-get install build-essential git cmake libqt4-dev libphonon-dev python2.7-dev libxml2-dev libxslt1-dev qtmobility-dev

#. Install latest ``pip`` distribution into the Python you
   installed in the first step: download `get-pip.py 
   <https://bootstrap.pypa.io/get-pip.py>`_ and run it using
   the ``python`` interpreter of your Python 2.7 installation using a
   command prompt:

   ::

      $ wget https://bootstrap.pypa.io/get-pip.py
      $ sudo python2.7 get-pip.py

#. Install latest ``wheel`` distribution:

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

#. Build the ``wheel`` binary distribution:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4

#. Optionally you can build standalone version of distribution with embedded Qt libs:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4 --standalone


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

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4 --version=1.2.2

#. Optionally you can build standalone version of distribution with embedded Qt libs:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4 --version=1.2.2 --standalone

#. To build the development version of ``PySide`` distribution, ignore the --version parameter:

   ::

      $ python2.7 setup.py bdist_wheel --qmake=/usr/bin/qmake-qt4


Installing PySide distribution
------------------------------

#. After the successful build, install the distribution with ``pip``. The file name may vary depending on your platform so look into the ``dist`` directory for the correct name:

   ::

      $ ls dist
      PySide-1.2.2-cp27-none-linux-x86_64.whl
      $ sudo pip2.7 install dist/PySide-1.2.2-cp27-none-linux-x86_64.whl


Installing PySide distribution into ``virtual`` Python environment
------------------------------------------------------------------

#. Install latest ``virtualenv`` distribution:

   ::

      $ sudo pip2.7 virtualenv

#. Use ``virtualenv`` to make a workspace:

   ::

      $ virtualenv-2.7 env

#. Switch to the ``env`` directory:

   ::

      $ cd env

#. Install the distribution with ``pip`` (wheel binary file name may vary by platform):

   ::

      $ bin/pip2.7 install ../dist/PySide-1.2.2-cp27-none-linux-x86_64.whl
