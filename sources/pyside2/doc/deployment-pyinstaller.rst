=======================
|project| & PyInstaller
=======================

`PyInstaller <https://www.pyinstaller.org/>`_ allows you to freeze your python
application into a stand-alone executable.
The supported platforms are Linux, macOS, Windows, FreeBSD, and others.

One of the main goals of `PyInstaller` is to be compatible with 3rd-party
Python modules, e.g.: |pymodname|.

You can read the `official documentation <https://www.pyinstaller.org/documentation.html>`_
to clarify any further question, and remember to contribute to
`the project <https://github.com/pyinstaller/pyinstaller>`_
by filing issues if you find any, or contributing to their development.

Preparation
===========

Installing `PyInstaller` can be done via **pip**::

    pip install pyinstaller

If you are using a virtual environment, remember to activate it before
installing `PyInstaller` into it.

After the installation, the `pyinstaller` binary will be located in the `bin/`
directory of your virtual environment, or where your Python executable is located.

If that directory is not in your `PATH`, you need to include the whole path
when executing `pyinstaller`.

.. warning:: If you already have PySide2 or Shiboken2 installed in your system, PyInstaller will pick them
             instead of your virtual environment ones.

Freezing an application
=======================

`PyInstaller` has many options that you can use.
To learn more about them you can just run `pyinstaller -h`.

Two main features are the option to package the whole project
(including the shared libraries) into one executable file (`--onefile`),
and to prepare a directory that will contain
an executable next to all the used libraries.

Additionally, for Windows you can enable opening a console during the
execution with the option `-c` (or equivalent `--console` or `--nowindowed`).
Further, you can specify to not open such console window
on macOS and Windows with the option `-w` (or equivalent `--windowed` or `--noconsole`).

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


Since it has a UI, we will use the `--windowed` option.

The command line to proceed will look like this::

    pyinstaller --name="MyApplication" --windowed hello.py

This process will create a `dist/` and `build/` directory.
The executable and all the shared libraries required by your application
will be placed inside `dist/MyApplication`.

To execute the frozen application you can go inside `dist/MyApplication` and
execute the program::

    cd dist/MyApplication/
    ./MyApplication

.. note:: The directory inside `dist/` and the executable will have the same name.

If you prefer to have everything bundled into one executable, i.e.:
no shared libraries next to the executable, you can use the option
`--onefile`::

    pyinstaller --name="MyApplication" --windowed --onefile hello.py

This process will take a bit longer, but in the end you will discover
an executable inside the `dist/` directory that you can execute::

    cd dist/
    ./MyApplication


Current Caveats To Be Aware Of
==============================


PyInstaller Problem
-------------------

As already mentioned, `PyInstaller` will pick a system installation of PySide2 or
Shiboken2 instead of your virtualenv version without notice, if it exists.
This may be no problem if those PySide2 or shiboken2 versions are the same.

If you are working with different versions, this can result in frustrating sessions,
when you think you are testing a new version, but `PyInstaller`
is silently working with a different, older version.


Problem with numpy in Python 2.7.16
-----------------------------------

A recent problem of PyInstaller is the appearance of Python 2.7.16 .
This Python version creates a problem that is known from Python 3 as a `Tcl/Tk` problem.
This does rarely show up in Python 3 because `Tcl/Tk` is seldom used with `PyInstaller.

On Python 2.7.16, this problem is very much visible, since many people are using numpy.
For some reason, installing `numpy` creates a dependency of `Tcl/Tk`, which can
be circumvented only by explicitly excluding `Tcl/Tk` related things by adding
this line to the analysis section of the spec-file::

    excludes=['FixTk', 'tcl', 'tk', '_tkinter', 'tkinter', 'Tkinter'],


Safety Instructions
-------------------

o When using `PyInstaller` with `virtualenv`, make sure that there is no system
  installation of PySide2 or shiboken2.

o Before compiling, use `pip -uninstall pyside2 shiboken2 -y` multiple times, until
  none of the programs is found anymore.

o Pip is usually a good tool. But to be 100 % sure, you should directly remove
  the PySide2 and shiboken2 folders from site-packages.

o Be sure to use the right version of pip. The safest way to really run the right
  pip, use the Python that you mean: Instead of the pip command, better use::

    <path/to/your/>python -m pip
