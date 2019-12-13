|project| & fbs
####################

``fbs`` provides a powerful environment for packaging,
creating installers, and signing your application. It also lets you manage updates to
your application. As it is based on PyInstaller, it supports Linux, macOS, and Windows.

You can read the `fbs tutorial`_ for more details about how to use
`fbs`, or check out the `fbs manual`_ for a complete set of features
and options.

.. _fbs tutorial: https://github.com/mherrmann/fbs-tutorial

.. _fbs manual: https://build-system.fman.io/manual/

Preparation
===========

Installing `fbs` (>= 0.7.6) can be done via **pip**::

    pip install fbs

If you are using a virtual environment, remember to activate it before
installing it.

After the installation, you will be able to use the `fbs` executable.

Starting a new project
======================

`fbs` provides nice features that lets you create a base
project structure by executing the following command::

    fbs startproject

This command prompts you to answer a few questions to configure the details
of your project, like:

 * Application name
 * Author name
 * Qt bindings (PySide2 or PyQt5)
 * Bundle indentified (for macOS)

After it finishes, you will have a `src/` directory that
contains the following structure::

    └── src
        ├── build
        │   └── settings
        └── main
            ├── icons
            │   ├── base
            │   ├── linux
            │   └── mac
            └── python

Inside the `settings` directory, you will find a couple of `json` files
that can be edited to include more information about your project.

The `main` file will be under the `python` directory, and its content
by default is::

    from fbs_runtime.application_context import ApplicationContext
    from PySide2.QtWidgets import QMainWindow

    import sys

    if __name__ == '__main__':
        appctxt = ApplicationContext()       # 1. Instantiate ApplicationContext
        window = QMainWindow()
        window.resize(250, 150)
        window.show()
        exit_code = appctxt.app.exec_()      # 2. Invoke appctxt.app.exec_()
        sys.exit(exit_code)

This example shows an empty `QMainWindow`. You can run it using the
following command::

    fbs run

Freezing the application
========================

Once you verify that the application is properly working,
you can continue with the freezing process using the following
command::

    fbs freeze

After the process finishes, you will get a message stating the location
of your executable. For example::

    Done. You can now run `target/MyApp/MyApp`. If that doesn't work, see
    https://build-system.fman.io/troubleshooting.


You can now try running the application, which will result in the same
window that you saw with the `fbs run` command::

    cd target/MyApp/
    ./MyApp

.. note:: This is the case for Linux. For other platforms like macOS,
   you need to enter the directory: `target/MyApp.app/Contents/macOS`,
   and for Windows find the `MyApp.exe` executable.
