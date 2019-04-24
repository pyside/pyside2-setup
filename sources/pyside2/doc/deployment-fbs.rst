===============
|project| & fbs
===============

`fbs <https://build-system.fman.io>`_ provides a powerful environment for packaging,
creating installers, and signing your application, but also for managing the application's updates.
Since it is based on PyInstaller, it currently supports Linux, macOS, and Windows.

You can read the `official tutorial <https://github.com/mherrmann/fbs-tutorial>`_ for more
details on how to use `fbs`, or check the
`documentation <https://build-system.fman.io/manual/>`_ for a complete set of features and
options.

Preparation
===========

Installing `fbs` can be done via **pip**::

    pip install fbs pyinstaller==3.4

If you are using a virtual environment, remember to activate it before
installing it.

After the installation, you will be able to use the `fbs` executable.

Starting a new project
======================

`fbs` provides nice features that allow you to create a base
project structure by executing the following command::

    fbs startproject

This process will prompt you to answer many questions to configure the details
of your project, like:

 * Application name
 * Author name
 * Qt bindings (PySide2 or PyQt5)
 * Bundle indentified (for macOS)

After the process finishes, you will have a `src/` directory that
will contain the following structure::

    └── src
        ├── build
        │   └── settings
        └── main
            ├── icons
            │   ├── base
            │   ├── linux
            │   └── mac
            └── python

Inside the `settings` directory you can find a couple of `json` files
that you can edit to include more information about your project.

The main file will be under the `python` directory, and its content by default is::

    from fbs_runtime.application_context import ApplicationContext
    from PySide2.QtWidgets import QMainWindow

    import sys

    class AppContext(ApplicationContext):           # 1. Subclass ApplicationContext
        def run(self):                              # 2. Implement run()
            window = QMainWindow()
            version = self.build_settings['version']
            window.setWindowTitle("MyApp v" + version)
            window.resize(250, 150)
            window.show()
            return self.app.exec_()                 # 3. End run() with this line

    if __name__ == '__main__':
        appctxt = AppContext()                      # 4. Instantiate the subclass
        exit_code = appctxt.run()                   # 5. Invoke run()
        sys.exit(exit_code)

The example will show an empty `QMainWindow`, and you can execute it by running::

    fbs run

Freezing the application
========================

Once you verify that the application is properly working,
you can continue with the freezing process::

    fbs freeze

After the process finishes, you will get a message stating the location
of your executable, e.g.::

    Done. You can now run `target/MyApp/MyApp`. If that doesn't work, see
    https://build-system.fman.io/troubleshooting.


Then executing the application will result in the same window
you saw with the `fbs run` command::

    cd target/MyApp/
    ./MyApp

.. note:: This is the case for Linux. For other platforms like macOS, you will need to
          enter the directory: `target/MyApp.app/Contents/MacOS`, and for
          Windows you will find a `MyApp.exe` executable.
