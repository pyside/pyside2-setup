Getting started
===============

Building from source
--------------------

This step is focused on building Shiboken from source, both the Generator and Python module.
Please notice that these are built when you are building PySide from source too, so there is no
need to continue if you already have a built PySide.

General Requirements
^^^^^^^^^^^^^^^^^^^^

 * **Python**: 3.5+ and 2.7
 * **Qt:** 5.12+ is recommended
 * **libclang:** The libclang library, recommended: version 6 for Shiboken2 5.12.
   Prebuilt versions of it can be `downloaded here`_.
 * **CMake:** 3.1+ is needed.

.. _downloaded here: http://download.qt.io/development_releases/prebuilt/libclang/

Simple build
^^^^^^^^^^^^

If you need only Shiboken Generator, a simple build run would look like this::

    python setup.py install --qmake=/path/to/qmake \
                            --build-tests \
                            --parallel=8 \
                            --verbose-build \
                            --internal-build-type=shiboken2-generator

The same can be used for the module, changing the value of ``internal-build-type`` to
``shiboken2-module``.

Using the wheels
----------------

Installing ``pyside2`` or ``shiboken2`` from pip **does not** install ``shiboken2_generator``,
because the wheels are not on PyPi.

You can get the ``shiboken2_generator`` wheels from Qt servers, and you can still install it
via ``pip``::

    pip install \
        --index-url=http://download.qt.io/official_releases/QtForPython/ \
        --trusted-host download.qt.io \
        shiboken2 pyside2 shiboken2_generator


The ``whl`` package cannot automatically discover in your system the location for:

* Clang installation,
* ``qmake`` location with the same version as the one described in the wheel,
* Qt libraries with the same package version.

So using this process requires you to manually modify the variables:

* ``CLANG_INSTALL_DIR`` must be set to where the libraries are,
* ``PATH`` must include the location for a ``qmake`` with the same Qt version as the package,
* ``LD_LIBRARY_PATH`` including the Qt libraries and Clang libraries paths.
