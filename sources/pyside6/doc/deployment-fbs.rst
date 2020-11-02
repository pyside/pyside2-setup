|project| & fbs
####################

`fbs`_ provides a powerful environment for packaging, creating installers, and signing your
application. It also lets you manage updates to your application. Since `fbs` is based on
PyInstaller, it supports Linux, macOS, and Windows.

For more details, see the `fbs tutorial`_ and the `fbs manual`_.

.. _fbs: https://build-system.fman.io/
.. _fbs tutorial: https://github.com/mherrmann/fbs-tutorial
.. _fbs manual: https://build-system.fman.io/manual/

Preparation
===========

Installing `fbs`_ (>= 0.7.6) is done via **pip**::

    pip install fbs

If you're using a virtual environment, remember to activate it before installing `fbs`_.

After the installation, you can use the `fbs`_ executable.

Starting a new project
======================

`fbs`_ provides useful features for you to create a base project structure with the following
command::

    fbs startproject

This command prompts you to answer a few questions to configure the details of your project, like:

 * Application name
 * Author name
 * Qt bindings (PySide6 or PyQt5)
 * Bundle indentified (for macOS)

Afterwards, you have a `src/` directory that contains the following structure::

    └── src
        ├── build
        │   └── settings
        └── main
            ├── icons
            │   ├── base
            │   ├── linux
            │   └── mac
            └── python

Inside the `settings` directory, there are a few JSON files that can be edited to include more
information about your project.

The `main` file is in the `python` directory, and its default content is::

    from fbs_runtime.application_context import ApplicationContext
    from PySide6.QtWidgets import QMainWindow

    import sys

    if __name__ == '__main__':
        appctxt = ApplicationContext()       # 1. Instantiate ApplicationContext
        window = QMainWindow()
        window.resize(250, 150)
        window.show()
        exit_code = appctxt.app.exec_()      # 2. Invoke appctxt.app.exec_()
        sys.exit(exit_code)

This example shows an empty `QMainWindow`. You can run it using the following command::

    fbs run

Freezing the application
========================

Once you've verified that the application is working properly, you can continue with the freezing
process using the following command::

    fbs freeze

After the process completes, you see a message stating the location of your executable. For
example::

    Done. You can now run `target/MyApp/MyApp`. If that doesn't work, see
    https://build-system.fman.io/troubleshooting.


Now, you can try to run the application. The result is the same window as the one you saw with the
`fbs run` command::

    cd target/MyApp/
    ./MyApp

.. note:: This is the case for Linux. For other platforms like macOS, you need to enter the
   directory: `target/MyApp.app/Contents/macOS`. For Windows, you need to find the `MyApp.exe`
   executable.
