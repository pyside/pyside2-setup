.. currentmodule:: PySide2.QtQml
.. _QmlElement:

QmlElement
**********

.. py:decorator:: QmlElement

   This decorator registers a class it is attached to for use in QML, using
   global variables to specify the import name and version.

   ::
      QML_IMPORT_NAME = "com.library.name"
      QML_IMPORT_MAJOR_VERSION = 1
      QML_IMPORT_MINOR_VERSION = 0 # Optional

      @QmlElement
      class ClassForQml(QObject):
          # ...

   Afterwards the class may be used in QML:

   ::
      import com.library.name 1.0

      ClassForQml {
         // ...
      }
