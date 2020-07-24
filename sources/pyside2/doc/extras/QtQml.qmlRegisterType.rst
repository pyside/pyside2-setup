.. currentmodule:: PySide2.QtQml
.. _qmlRegisterType:

qmlRegisterType
***************

.. py:function:: qmlRegisterType(pytype: type, uri: str, versionMajor: int, versionMinor: int, qmlName: str) -> int

   :param type pytype: Python class
   :param str uri: uri to use while importing the component in QML
   :param int versionMajor: major version
   :param int versionMinor: minor version
   :param str qmlName: name exposed to QML
   :return: int (the QML type id)

   This function registers the Python *type* in the QML system with the
   name *qmlName*, in the library imported from *uri* having the
   version number composed from *versionMajor* and *versionMinor*.

   For example, this registers a Python class 'MySliderItem' as a QML
   type named 'Slider' for version '1.0' of a module called
   'com.mycompany.qmlcomponents':

   ::

       qmlRegisterType(MySliderItem, "com.mycompany.qmlcomponents", 1, 0, "Slider")

   Once this is registered, the type can be used in QML by importing
   the specified module name and version number:

   ::

       import com.mycompany.qmlcomponents 1.0

       Slider { ... }

   Note that it's perfectly reasonable for a library to register types
   to older versions than the actual version of the library.
   Indeed, it is normal for the new library to allow QML written to
   previous versions to continue to work, even if more advanced
   versions of some of its types are available.
