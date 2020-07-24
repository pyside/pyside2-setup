.. currentmodule:: PySide2.QtQml
.. _qmlRegisterSingletonType:

qmlRegisterSingletonType
************************

.. py:function:: qmlRegisterSingletonType(pytype: type, uri: str, versionMajor: int, versionMinor: int, typeName: str) -> int

   :param type pytype: Python class
   :param str uri: uri to use while importing the component in QML
   :param int versionMajor: major version
   :param int versionMinor: minor version
   :param str typeName: name exposed to QML
   :return: int (the QML type id)

   This function registers a Python type as a singleton in the QML system.

.. py:function:: qmlRegisterSingletonType(pytype: type, uri: str, versionMajor: int, versionMinor: int, typeName: str, callback: object) -> int

   :param type pytype: Python class
   :param str uri: uri to use while importing the component in QML
   :param int versionMajor: major version
   :param int versionMinor: minor version
   :param str typeName: name exposed to QML
   :param object callback: Python callable (to handle Python type)
   :return: int (the QML type id)

   This function registers a Python type as a singleton in the QML system using
   the provided callback (which gets a QQmlEngine as a parameter) to generate
   the singleton.


.. py:function:: qmlRegisterSingletonType(uri: str, versionMajor: int, versionMinor: int, typeName: str, callback: object) -> int

   :param str uri: uri to use while importing the component in QML
   :param int versionMajor: major version
   :param int versionMinor: minor version
   :param str typeName: name exposed to QML
   :param object callback: Python callable (to handle QJSValue)
   :return: int (the QML type id)

   This function registers a QJSValue as a singleton in the QML system using
   the provided callback (which gets a QQmlEngine as a parameter) to
   generate the singleton.
