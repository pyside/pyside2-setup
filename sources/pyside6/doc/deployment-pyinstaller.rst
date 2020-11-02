|project| & PyInstaller
#######################

`PyInstaller <https://www.pyinstaller.org/>`_ lets you freeze your python application into a
stand-alone executable. This installer supports Linux, macOS, Windows, and more; and is also
compatible with 3rd-party Python modules, such as |pymodname|.

For more details, see the `official documentation <https://www.pyinstaller.org/documentation.html>`_.

Preparation
===========

Install the `PyInstaller` via **pip** with the following command::

    pip install pyinstaller

If you're using a virtual environment, remember to activate it before installing `PyInstaller`.

After installation, the `pyinstaller` binary is located in your virtual environment's `bin/`
directory, or where your Python executable is located. If that directory isn't in your `PATH`,
include the whole path when you run `pyinstaller`.

.. warning:: If you already have a PySide6 or Shiboken6 version installed in your
   system path, PyInstaller uses them instead of your virtual environment version.

Freeze an application
=======================

`PyInstaller` has many options that you can use. To list them all, run `pyinstaller -h`.

There are two main features:

 * the option to package the whole project (including shared libraries) into one executable file
   (`--onefile`)
 * the option to place it in a directory containing the libraries

Additionally, on Windows when the command is running, you can open a console with the `-c` option
(or `--console` or `--nowindowed` equivalent).

Otherwise, you can specify to not open such a console window on macOS and Windows with the `-w`
option (or `--windowed` or `--noconsole` equivalent).

Create an example
-----------------

Now, consider the following script, named `hello.py`::

    import sys
    import random
    from PySide6.QtWidgets import (QApplication, QLabel, QPushButton,
                                   QVBoxLayout, QWidget)
    from PySide6.QtCore import Slot, Qt

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


Since it has a UI, you use the `--windowed` option.

The command line to proceed looks like this::

    pyinstaller --name="MyApplication" --windowed hello.py

This process creates two directories: `dist/` and `build/`. The application executable and the
required shared libraries are placed in `dist/MyApplication`.

To run the application, go to `dist/MyApplication` and run the program::

    cd dist/MyApplication/
    ./MyApplication

.. note:: The directory inside `dist/` and the executable have the same name.

Use the `--onefile` option if you prefer to have everything bundled into one executable, without
the shared libraries next to it::

    pyinstaller --name="MyApplication" --windowed --onefile hello.py

This process takes a bit longer, but in the end you have one executable in the `dist/` directory::

    cd dist/
    ./MyApplication


Some Caveats
============


PyInstaller Issue
-----------------

As mentioned before, if available, `PyInstaller` picks a system installation of PySide6 or
Shiboken6 instead of your `virtualenv` version without notice. This is negligible if those
two versions are the same.

If you're working with different versions, this can result in frustrating debugging sessions
when you think you are testing the latest version, but `PyInstaller` is working with an older
version.


Safety Instructions
-------------------

- When using `PyInstaller` with `virtualenv`, make sure that there is no system
  installation of PySide6 or shiboken6.

- Before compiling, use `pip -uninstall pyside6 shiboken6 -y` multiple times, until
  none of the programs are found anymore.

- Pip is usually a good tool. But to be 100 % sure, you should directly remove
  the PySide6 and shiboken6 folders from site-packages.

- Be sure to use the right version of pip. The safest way to really run the right
  pip, is to use the Python that you mean: Instead of the pip command, better use::

    <path/to/your/>python -m pip
