|project| Quick start
======================

Requirements
------------

Before you can install |project|, first you must install the following software:

 * Python 2.7 or 3.5+,
 * We recommend using a virtual environment, such as
   `venv <https://docs.python.org/3/library/venv.html>`_ or
   `virtualenv <https://virtualenv.pypa.io/en/latest>`_

Installation
------------

Now you are ready to install the |project| packages using ``pip``.
From the terminal, run the following command::

    pip install PySide2 # For the latest version on PyPi

or::

    pip install --index-url=http://download.qt.io/snapshots/ci/pyside/5.14/latest pyside2 --trusted-host download.qt.io

Test your Installation
----------------------

Now that you have |project| installed, you can test your setup by running the following Python
constructs to print version information::

    import PySide2.QtCore

    # Prints PySide2 version
    # e.g. 5.11.1a1
    print(PySide2.__version__)

    # Gets a tuple with each version component
    # e.g. (5, 11, 1, 'a', 1)
    print(PySide2.__version_info__)

    # Prints the Qt version used to compile PySide2
    # e.g. "5.11.2"
    print(PySide2.QtCore.__version__)

    # Gets a tuple with each version components of Qt used to compile PySide2
    # e.g. (5, 11, 2)
    print(PySide2.QtCore.__version_info__)

Create a Simple Application
---------------------------

Your |project| setup is ready. You can explore it further by developing a simple application
that prints "Hello World" in several languages. The following instructions will
guide you through the development process:

1. Create a new file named :code:`hello_world.py`, and add the following imports to it.::

        import sys
        import random
        from PySide2 import QtCore, QtWidgets, QtGui

  The |pymodname| Python module provides access to the Qt APIs as its submodule.
  In this case, you are importing the :code:`QtCore`, :code:`QtWidgets`, and :code:`QtGui` submodules.

2. Define a class named :code:`MyWidget`, which extends QWidget and includes a QPushButton and
   QLabel.::

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

  The MyWidget class has the :code:`magic` member function that randomly chooses an item from the
  :code:`hello` list. When you click the button, the :code:`magic` function is called.

3. Now, add a main function where you instantiate :code:`MyWidget` and :code:`show` it.::

        if __name__ == "__main__":
            app = QtWidgets.QApplication([])

            widget = MyWidget()
            widget.resize(800, 600)
            widget.show()

            sys.exit(app.exec_())

Run your example. Try clicking the button at the bottom to see which greeting you get.

.. image:: pyside-examples/images/screenshot_hello.png
   :alt: Hello World application
