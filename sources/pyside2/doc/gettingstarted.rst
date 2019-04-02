===============
Getting Started
===============

To develop with |project|, you must install Python, Clang, and |project|.

Preparing for the Installation
==============================

Before you can install |project|, you must install the following software:

* Python 3.5+ or 2.7
* libclang 5.0+ (for Qt 5.11) or 6.0+ (for Qt 5.12)
* Recommended: a virtual environment, such as `venv <https://docs.python.org/3/library/venv.html>`_ or `virtualenv <https://virtualenv.pypa.io/en/stable/installation>`_

Installing |project|
====================

After you have installed the required software, you are ready to install the |project|
packages using the pip wheel. Run the following command from your command
prompt to install::

    pip install PySide2 # For the latest version on PyPi

or::

    pip install --index-url=http://download.qt.io/snapshots/ci/pyside/5.12/latest pyside2 --trusted-host download.qt.io

Testing the Installation
========================

Now that you have |project| installed, you can test your setup by running the following Python
constructs to print version information:

.. include:: pysideversion.rst
   :start-line: 5
   :end-line: 32

Creating a Simple Application
=============================

Your |project| setup is ready, so try exploring it further by developing a simple application
that prints "Hello World" in several languages. The following instructions will
guide you through the development process:

* Create a new file named :code:`hello_world.py`, and add the following imports to it.

  ::

        import sys
        import random
        from PySide2 import QtCore, QtWidgets, QtGui

  The |pymodname| Python module provides access to the Qt APIs as its submodule.
  In this case, you are importing the :code:`QtCore`, :code:`QtWidgets`, and :code:`QtGui` submodules.

* Define a class named :code:`MyWidget`, which extends QWidget and includes a QPushButton and QLabel.

  ::

        class MyWidget(QtWidgets.QWidget):
            def __init__(self):
                super().__init__()

                self.hello = ["Hallo Welt", "Hei maailma", "Hola Mundo", "Привет мир"]

                self.button = QtWidgets.QPushButton("Click me!")
                self.text = QtWidgets.QLabel("Hello World")
                self.text.setAlignment(QtCore.Qt.AlignCenter)

                self.layout = QtWidgets.QVBoxLayout()
                self.layout.addWidget(self.text)
                self.layout.addWidget(self.button)
                self.setLayout(self.layout)

                self.button.clicked.connect(self.magic)


            def magic(self):
                self.text.setText(random.choice(self.hello))

  The MyWidget class has the :code:`magic` member function that
  randomly chooses an item from the list :code:`hello`. This function
  is called when you click the button.

* Now, add a main function where you instantiate :code:`MyWidget` and
  :code:`show` it.

  ::

        if __name__ == "__main__":
            app = QtWidgets.QApplication([])

            widget = MyWidget()
            widget.resize(800, 600)
            widget.show()

            sys.exit(app.exec_())

Your example is ready to be run. Try clicking the button at the bottom
and see which greeting you get.
