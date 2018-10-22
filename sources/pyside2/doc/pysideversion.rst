Printing |project| and Qt version
*********************************

|project| exports its version numbers in a pythonic way. You can print these
numbers using the following python constructs:

  ::

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


Note that the Qt version used to compile |project| may differ from the version used to
run |project|. To print the current running Qt version number, you can use::

    print(PySide2.QtCore.qVersion())
