Deployment
##########

Deploying or freezing an application is a crucial part of many Python projects.
Most large projects are not based on a single Python file, so
the distribution of these applications becomes more difficult.

Here are a few distribution options that you could use:
 1. Sending a normal zip-file with the application's content.
 2. Building a proper `Python package (wheel) <https://packaging.python.org/>`_.
 3. Freezing the application into a single binary file or a directory.

.. _fbs: https://build-system.fman.io/

.. _pyinstaller: https://www.pyinstaller.org/

.. _cxfreeze: https://anthony-tuininga.github.io/cx_Freeze/

.. _py2exe: http://www.py2exe.org/

.. _py2app: https://py2app.readthedocs.io/en/latest/

If you choose the **third** option, consider using one of these tools:
 * `fbs`_
 * `PyInstaller <pyinstaller>`_
 * `cx_Freeze <cxfreeze>`_
 * `py2exe`_
 * `py2app`_


|project| is a cross-platform framework,
so we would like to focus on solutions that work on the three
major platforms supported by Qt: Linux, macOS, and Windows.
The following table summarizes the platform support for those packaging
tools:

===========   =======   =====   =====   =======
Name          License   Linux   macOS   Windows
===========   =======   =====   =====   =======
fbs           GPL       yes     yes     yes
PyInstaller   GPL       yes     yes     yes
cx_Freeze     MIT       yes     yes     yes
py2exe        MIT       no      no      yes
py2app        MIT       no      yes     no
===========   =======   =====   =====   =======

According to this table, only *fbs*, *cx_Freeze*, and *PyInstaller*
meets our cross-platform requirement.

As these are command-line tools, it could be hard to include
resources to your application, such as images, icons, and
meta-information. This means, you will need special hooks
or scripts to handle them before adding to the package.
In addition to this, these tools does not offer a mechanism
to update your application packages.

To create update packages, use the `PyUpdater <https://www.pyupdater.org/>`_,
which is built around PyInstaller.

The `fbs <https://build-system.fman.io>`_ tool offers a nice UI
that allows the user to install the application step-by-step.

Here you can find a set of tutorials on how to use the previously
described tools.

.. note::

   Deployment is supported only from Qt for Python 5.12.2 and later.

.. toctree::
    :name: mastertoc
    :maxdepth: 2

    deployment-fbs.rst
    deployment-pyinstaller.rst
    deployment-cxfreeze.rst
