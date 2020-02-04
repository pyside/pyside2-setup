|project| Getting Started
==========================

This page is focused on building |project| from source, if you just want to install |pymodname|
with ``pip`` you need to run::

    pip install pyside2

for more details, refer to our `Quick Start`_ guide. Additionally, you can
check the `FAQ`_ related to the project.

.. _Quick Start: quickstart.html
.. _FAQ: faq.html

General Requirements
--------------------

 * **Python**: 3.5+ and 2.7
 * **Qt:** 5.12+ is recommended
 * **libclang:** The libclang library, recommended: version 6 for PySide2 5.12.
   Prebuilt versions of it can be `downloaded here`_.
 * **CMake:** 3.1+ is needed.

.. _downloaded here: http://download.qt.io/development_releases/prebuilt/libclang/

Guides per platform
-------------------

You can refer to the following pages for platform specific instructions:

 * `Windows`_,
 * `macOS`_,
 * `Linux`_,
 * Mobile platforms (iOS/Android) **(no support)**
 * Embedded platforms **(no official support)**

   .. note:: Most Linux-based embedded OS provide PySide2 with their official
             package manager (e.g. `Raspbian`_ and `ArchlinuxARM`_).

.. _Windows: gettingstarted-windows.html
.. _macOS: gettingstarted-macOS.html
.. _Linux: gettingstarted-linux.html
.. _Raspbian: https://www.raspbian.org/
.. _ArchlinuxARM: https://archlinuxarm.org/

A normal building command will look like this::

    python setup.py install --qmake=/path/to/qmake \
                            --ignore-git \
                            --debug \
                            --build-tests \
                            --parallel=8 \
                            --make-spec=ninja \
                            --verbose-build \
                            --module-subset=Core,Gui,Widgets

Which will build and install the project with **debug** symbols, including the **tests**,
using **ninja** (instead of make), and considering only the **module subset** of QtCore, QtGUI
and QtWidgets.

Other important options to consider are:
 * ``--cmake``, to specify the path to the cmake binary,
 * ``--reuse-build``, to rebuild only the modified files,
 * ``--openssl=/path/to/openssl/bin``, to use a different path for OpenSSL,
 * ``--standalone``, to copy over the Qt libraries into the final package
   to make it work on other machines.

Testing the installation
-------------------------

Once the installation finishes, you will be able to execute any of our examples::

    python examples/widgets/widgets/tetrix.py

Running Tests
--------------

Using the ``--build-tests`` option will enable us to run all the auto tests inside the project::

    python testrunner.py test > testlog.txt

.. note:: On Windows, don't forget to have qmake in your path
   (``set PATH=E:\Path\to\Qt\5.14\msvc2017_64\bin;%PATH%``)

You can also run a specific test (for example ``qpainter_test``) by running::

    ctest -R qpainter_test --verbose

Building the documentation
---------------------------

The documentation is being generated using **qdoc** to get the API information, and also **sphinx**
for the local Python related notes.

The system required ``libxml2`` and `libxslt``, also on the Python environment, ``sphinx`` and
``graphviz`` need to be installed before running the installation process::

    pip install graphviz sphinx

After installing ``graphviz`, the ``dot`` command needs to be in PATH, otherwise,
the process will fail. Installing ``graphviz`` system-wide is also an option.

Since the process rely on a Qt installation, you need to specify where the ``qtbase`` directory
you will use with your ``qmake`` is located::

    export QT_SRC_DIR=/path/to/qtbase

Once the build process finishes, you can go to the generated ``*_build/*_release/pyside2``
directory, and run::

    make apidoc

Finally, you will get a ``html`` directory containing all the generated documentation.
