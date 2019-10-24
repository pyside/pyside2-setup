Overview
=========

The |project| project aims to provide a complete port of the PySide module to Qt 5.
The development started on GitHub in May 2015. The project managed to port Pyside to
Qt 5.3, 5. 4 & 5.5.

The `PySide2` module was released mid June 2018 as a Technical Preview (supporting Qt 5.11),
and it has been fully supported since Qt 5.12.

|project| is available under LGPLv3/GPLv2 and commercial license for the following platforms:

+-------------+--------+--------+--------+--------+---------+---------+
|             | Linux           | macOS           | Windows           |
+=============+========+========+========+========+=========+=========+
|             | 32bit  | 64bit  | 32bit  | 64bit  | 32bit   | 64bit   |
+-------------+--------+--------+--------+--------+---------+---------+
| Python 2.7  | No (*) | Yes    | No (*) | Yes    | No (**) | No (**) |
+-------------+--------+--------+--------+--------+---------+---------+
| Python 3.5+ | No (*) | Yes    | No (*) | Yes    | Yes     | Yes     |
+-------------+--------+--------+--------+--------+---------+---------+

 * (*): `No Qt release <https://wiki.qt.io/Qt_5.12_Tools_and_Versions#Software_configurations_for_Qt_5.12.0>`_
 * (**): `MSVC issue with Python 2.7 and Qt <https://wiki.qt.io/Qt_for_Python/Considerations#Missing_Windows_.2F_Python_2.7_release>`_


What does PySide2 look like?
----------------------------

A simple Hello World example in PySide2 looks like this:

::

      import sys
      from PySide2.QtWidgets import QApplication, QLabel


      if __name__ == "__main__":
          app = QApplication(sys.argv)
          label = QLabel("Hello World")
          label.show()
          sys.exit(app.exec_())


Additional overviews
--------------------

These additional topics provide detailed information about
several Qt-specific features:

.. toctree::
   :titlesonly:
   :glob:

   overviews/*

