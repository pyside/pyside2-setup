.. _pyside-api:

|project| Modules
=================

Basic modules
-------------

These are the main modules that help you build a Widget-based UI.

+---------------------------------------+--------------------------------------------------------+
| :mod:`Qt Core <PySide2.QtCore>`       | Provides core non-GUI functionality, like signal and   |
|                                       | slots, properties, base classes of item models,        |
|                                       | serialization, and more.                               |
+---------------------------------------+--------------------------------------------------------+
| :mod:`Qt GUI <PySide2.QtGui>`         | Extends QtCore with GUI functionality: Events, windows |
|                                       | and screens, OpenGL and raster-based 2D painting, as   |
|                                       | well as images.                                        |
+---------------------------------------+--------------------------------------------------------+
| :mod:`Qt Widgets <PySide2.QtWidgets>` | Provides ready to use Widgets for your application,    |
|                                       | including graphical elements for your UI.              |
+---------------------------------------+--------------------------------------------------------+

QML and Qt Quick
----------------

Use these modules to interact with the `QML Language <https://doc.qt.io/qt-5.qmlapplications>`,
from Python.

+-------------------------------------------------+----------------------------------------------+
| :mod:`Qt QML <PySide2.QtQml>`                   | The base Python API to interact with the     |
|                                                 | module.                                      |
+-------------------------------------------------+----------------------------------------------+
| :mod:`Qt Quick <PySide2.QtQuick>`               | Provides classes to embed Qt Quick in Qt     |
|                                                 | applications.                                |
+-------------------------------------------------+----------------------------------------------+
| :mod:`Qt QuickWidgets <PySide2.QtQuickWidgets>` | Provides the QQuickWidget class to embed Qt  |
|                                                 | Quick in widget-based applications.          |
+-------------------------------------------------+----------------------------------------------+

Data visualization
------------------

Charts, diagrams, animations: these modules provide classes to help you include these elements in
your UI.

+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt Charts <PySide2.QtCharts>`                        | Provides a set of easy to use     |
|                                                            | chart components.                 |
+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt DataVisualization <PySide2.QtDataVisualization>`  | Provides a way to visualize data  |
|                                                            | in 3D as bar, scatter, or surface |
|                                                            | graphs.                           |
+------------------------------------------------------------+-----------------------------------+

Multimedia
-----------

Audio, video, and hardware interaction: use these modules for multimedia solutions.

+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt Multimedia <PySide2.QtMultimedia>`                | Provides low-level multimedia     |
|                                                            | functionality.                    |
+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt MultimediaWidgets <PySide2.QtMultimediaWidgets>`  | Provides the widget-based         |
|                                                            | multimedia API.                   |
+------------------------------------------------------------+-----------------------------------+

WebEngine
---------

If your project is based on a browser or the features around Web-based applications, use these
modules to interact with them.

+---------------------------------------------------------+--------------------------------------+
| :mod:`Qt WebEngineWidgets <PySide2.QtWebEngineWidgets>` | Provides widgets to handle Web       |
|                                                         | content.                             |
+---------------------------------------------------------+--------------------------------------+
| :mod:`Qt WebChannel <PySide2.QtWebChannel>`             | Enables peer-to-peer communication   |
|                                                         | between a server and a client        |
|                                                         | (HTML/JavaScript or QML application).|
+---------------------------------------------------------+--------------------------------------+

All the modules
---------------

There are many other modules currently supported by |pymodname|, here you can find a complete list
of them.

  :doc:`Check all the modules <modules>`
