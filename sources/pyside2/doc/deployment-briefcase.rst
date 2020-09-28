|project| & Briefcase
#######################

`Briefcase <https://briefcase.readthedocs.io>`_  is a packaging tool that lets you create a standalone package for a Python application. It supports the following installer formats:

 * .app application bundle for macOS
 * MSI installer for Windows
 * AppImage for Linux

For more details, see the `official documentation <https://briefcase.readthedocs.io/en/latest/index.html>`_.

Preparation
===========

Install `Briefcase` using the following **pip** command::

    pip install briefcase

You also need : docker on linux, `WixToolset`_ on windows,

If you're using a virtual environment, remember to activate it before installing `Briefcase`.

After installation, the `briefcase` binary is located in your virtual environment's `bin/`
directory, or where your Python executable is located.

You can either create a brand new project using the briefcase assistant or setup your own.

.. _`WixToolset`: https://wixtoolset.org/

Use Briefcase Assistant
=======================

Run the following command and answer the questions to get started::

    briefcase new

Ensure that `PySide2` is chosen as the `GUI toolkit choice`.
Your PySide2 application is now configured. You can jump to `Build the package`_.


Set up your project
===================

Create a pyproject.toml
-----------------------

At the root level of your project, create a `pyproject.toml` file::

        [tool.briefcase]
        project_name = "MyPySideApp"
        bundle = "com.example"
        version = "0.0.1"
        url = "https://somwhere/on/the/net"
        license = "GNU General Public License v3 (GPLv3)"
        author = 'MyName Firstname'
        author_email = "cool@mailexample.com"

        [tool.briefcase.app.mypysideapp]
        formal_name = "A Cool App"
        description = "The coolest app ever"
        icon = "src/mypysideapp/resources/appicon" # Briecase will choose the right extension depending the os (png,ico,...)
        sources = ['src/mypysideapp']
        requires = ['pyside2==5.15.0',
                    'pony>=0.7.11,<0.8',
                    'dickens==1.0.1',
                    'Pillow==7.1.2',
                    'mako==1.1.2',
                    'beautifulsoup4']


        [tool.briefcase.app.mypysideapp.macOS]
        requires = []

        [tool.briefcase.app.mypysideapp.linux]
        requires = []
        system_requires = []

        [tool.briefcase.app.mypysideapp.windows]
        requires = []


Write some code
----------------

Let's say your project tree is like this::

    pyproject.toml
    setup.cfg
    pytest.ini
    src/

        mypysideapp/
            resources/
                appicon.png
                appicon.ico
            __init__.py
            __main__.py
            app.py


Content of `__main__.py`::

    import sys
    from PySide2.QtWidgets import QApplication
    from mypysideapp.app import MyWidget

    if __name__ == "__main__":
        app = QApplication(sys.argv)

        widget = MyWidget()
        widget.resize(800, 600)
        widget.show()

        sys.exit(app.exec_())


Content of  `app.py`::

    import random
    from PySide2.QtWidgets import (QLabel, QPushButton,
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


Build the package
==================

Initialize the package
------------------------

Just run::

    briefcase create

Run the following command to initialize the building the packages for Windows, Linux, and macOS.
It creates a subdirectory each for the different platforms.
This step takes longer as it adds the packages listed in `requires` sections in the `pyproject.toml` file.

Build the application
---------------------
::

    briefcase build

You'll get::

    macOS/A Cool App/A Cool App.app
    or
    linux/A Cool App-x86_64-0.0.1.AppImage
    or
    windows\A Cool App


Run the application
-------------------
::

    briefcase run

.. note:: You can run your project in `dev` mode (your source code not packaged) with `briefcase dev`


Build the installer (only Windows and macOS)
---------------------------------------------

macOS::

    briefcase package --no-sign

It's possible to sign, see the `documentation <https://briefcase.readthedocs.io/en/latest/how-to/code-signing/index.html>`_. You get `macOS/A Cool App-0.0.1.dmg`

Windows::

    briefcase package

You get `windows\A_Cool_App-0.0.1.msi`
