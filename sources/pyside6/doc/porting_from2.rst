Porting applications from PySide2 to PySide6
********************************************

Qt for Python 6.0.0 initially ships with the essential
`Modules <https://doc.qt.io/qt-6/qtmodules.html>`_ and some
add-ons (Qt Concurrent, Qt Help, Qt OpenGL, Qt Print Support
Qt Quick Widgets, Qt SQL, Qt SVG, Qt UI Tools and Qt XML).
More modules will follow in subsequent releases as they
are added to Qt.

The first thing to do when porting applications is to replace the
import statements:

::

    from PySide2.QtWidgets import QApplication...
    from PySide2 import QtCore

needs to be changed to:

::

    from PySide6.QtWidgets import QApplication...
    from PySide6 import QtCore


Some classes are in a different module now, for example
``QAction`` and ``QShortcut`` have been moved from ``QtWidgets`` to ``QtGui``.

Then, the code base needs to be checked for usage of deprecated API and adapted
accordingly. More information can be found in the
`Porting to Qt 6 <https://doc.qt.io/qt-6/portingguide.html>`_ Guide
and the `Qt 6.0 Documentation <https://doc.qt.io/qt-6/index.html>`_ .
