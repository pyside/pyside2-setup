=====================
|project| & cx_Freeze
=====================

`cx_Freeze <https://anthony-tuininga.github.io/cx_Freeze/>`_ lets you
freeze your Python application into executables. The supported
platforms are Linux, macOS, Windows, FreeBSD, among others.

You can read the `official documentation <https://cx-freeze.readthedocs.io/en/latest/index.html>`_
to clarify any further question, and remember to contribute to
the project by `filing issues <https://sourceforge.net/projects/cx-freeze/>`_
if you find any, or contributing to `their development <https://bitbucket.org/anthony_tuininga/cx_freeze/src>`_.

Preparation
===========

Installing `cx_Freeze` can be done using **pip**::

    pip install cx_freeze

If you are using a virtual environment, remember to activate it before
installing `cx_Freeze` into it.

After the installation, you will have the `cxfreeze` binary to deploy
your application.

Freezing an application
=======================

There are three options to work with `cx_Freeze`:

 1. Using the `cxfreeze` script.
 2. Creating `setup.py` script to build the project.
 3. Using the module classes directly (for advanced purposes).

The following sections cover the first two use cases.

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


Using `cxfreeze` executable
---------------------------

Now that we have an application, try freezing it with the following
command::

    cxfreeze hello.py

This command creates a `dist/` directory containing the executable.
and a `lib/` directory containing all the shared libraries.

To launch the application, go to the `dist/` directory and execute
the file::

    cd dist/
    ./main


Using a setuptools script
-------------------------

For this process, you need an additional script called `setup.py`::

    import sys
    from cx_Freeze import setup, Executable

    setup(name = "MyApp",
          version = "0.1",
          description = "My GUI App",
          executables = [Executable("hello.py")])

Now, build the project using it::

    python setup.py build

This step creates a `build/` directory with the following structure::

    build
    └── exe.linux-x86_64-3.7
        └── lib
        └── main

The first directory inside `build/` depends on the platform
you are using, in this case a `x86_64` Linux using Python 3.7.
The structure is the same as previously described, and you can simply
enter the directory and execute the file::

    cd build/exe.linux-x86_64-3.7
    ./main
