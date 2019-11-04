|project| & PyInstaller
#######################

`PyInstaller <https://www.pyinstaller.org/>`_ lets you freeze your python
application into a stand-alone executable.
The supported platforms are Linux, macOS, Windows, FreeBSD, and others.

One of the main goals of `PyInstaller` is to be compatible with 3rd-party
Python modules, for example: |pymodname|.

You can read the `official documentation <https://www.pyinstaller.org/documentation.html>`_
to clarify any further question, and remember to contribute to
`the project <https://github.com/pyinstaller/pyinstaller>`_
by filing issues if you find any, or contributing to their development.

Preparation
===========

Installing `PyInstaller` can be done using **pip**::

    pip install pyinstaller

If you are using a virtual environment, remember to activate it before
installing `PyInstaller` into it.

After the installation, the `pyinstaller` binary will be located in the `bin/`
directory of your virtual environment, or where your Python executable is located.
If that directory is not in your `PATH`, include the whole path when executing `pyinstaller`.

.. warning:: If you already have a PySide2 or Shiboken2 version installed in your
   system path, PyInstaller will pick them instead of your virtual environment
   version.

Freezing an application
=======================

`PyInstaller` has many options that you can use.
To learn more about them you can just run `pyinstaller -h`.

Two main features are the option to package the whole project
(including the shared libraries) into one executable file (`--onefile`),
and to place it in a directory containing the libraries.

Additionally, for Windows you can enable opening a console during the
execution with the option, `-c` (or equivalent `--console` or `--nowindowed`).
Further, you can specify to not open such console window
on macOS and Windows with the option, `-w` (or equivalent `--windowed` or `--noconsole`).

Creating an example
-------------------

Now, consider the following simple script, named `hello.py`::

    import sys
    import random
    from PySide2.QtWidgets import (QApplication, QLabel, QPushButton,
                                   QVBoxLayout, QWidget)
    from PySide2.QtCore import Slot, Qt

    class MyWidget(QWidget):
        def __init__(self):
            QWidget.__init__(self)

            self.hello = ["Hallo Welt", "你好，世界", "Hei maailma",
                "Hola Mundo", "Привет мир"]

            self.button = QPushButton("Click me!")
            self.text = QLabel("Hello World")
            self.text.setAlignment(Qt.AlignCenter)

            self.layout = QVBoxLayout()
            self.layout.addWidget(self.text)
            self.layout.addWidget(self.button)
            self.setLayout(self.layout)

            # Connecting the signal
            self.button.clicked.connect(self.magic)

        @Slot()
        def magic(self):
            self.text.setText(random.choice(self.hello))

    if __name__ == "__main__":
        app = QApplication(sys.argv)

        widget = MyWidget()
        widget.resize(800, 600)
        widget.show()

        sys.exit(app.exec_())


As it has a UI, you will use the `--windowed` option.

The command line to proceed looks like this::

    pyinstaller --name="MyApplication" --windowed hello.py

This process creates a `dist/` and `build/` directory.
The application executable and the required shared libraries are
placed in `dist/MyApplication`.

To run the application you can go to `dist/MyApplication` and
execute the program::

    cd dist/MyApplication/
    ./MyApplication

.. note:: The directory inside `dist/` and the executable will have
          the same name.

If you prefer to have everything bundled into one executable,
without the shared libraries next to it, you can use the option
`--onefile`::

    pyinstaller --name="MyApplication" --windowed --onefile hello.py

This process takes a bit longer, but in the end you will have one
executable in the `dist/` directory::

    cd dist/
    ./MyApplication


Current Caveats To Be Aware Of
==============================


PyInstaller Problem
-------------------

As already mentioned, `PyInstaller` will pick a system installation
of PySide2 or Shiboken2 instead of your virtualenv version without
notice, if it exists. This may not be a problem if those two
versions are the same.

If you are working with different versions, this can result in
frustrating debugging sessions. You could think you are testing the
latest version, but `PyInstaller` could be working with an older
version.

Problem with numpy in Python 2.7.16
-----------------------------------

A recent problem of PyInstaller is the Python 2 release, that is
v2.7.16. This Python version creates a problem that is known from
Python 3 as a `Tcl/Tk` problem. It rarely shows up in Python 3 as
`Tcl/Tk` is seldom used with `PyInstaller.

On Python 2.7.16, this problem is very much visible, as many are
using numpy. For some reason, installing `numpy` creates a
dependency to `Tcl/Tk`, which can be circumvented only by explicitly
excluding `Tcl/Tk` related things by adding this line to the analysis
section of the spec-file::

    excludes=['FixTk', 'tcl', 'tk', '_tkinter', 'tkinter', 'Tkinter'],


Safety Instructions
-------------------

- When using `PyInstaller` with `virtualenv`, make sure that there is no system
  installation of PySide2 or shiboken2.

- Before compiling, use `pip -uninstall pyside2 shiboken2 -y` multiple times, until
  none of the programs are found anymore.

- Pip is usually a good tool. But to be 100 % sure, you should directly remove
  the PySide2 and shiboken2 folders from site-packages.

- Be sure to use the right version of pip. The safest way to really run the right
  pip, is to use the Python that you mean: Instead of the pip command, better use::

    <path/to/your/>python -m pip
