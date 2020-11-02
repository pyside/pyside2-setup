.. _pyside-api:

|project| Modules
=================

Basic modules
-------------

These are the main modules that help you build a Widget-based UI.

+---------------------------------------+--------------------------------------------------------+
| :mod:`Qt Core <PySide6.QtCore>`       | Provides core non-GUI functionality, like signal and   |
|                                       | slots, properties, base classes of item models,        |
|                                       | serialization, and more.                               |
+---------------------------------------+--------------------------------------------------------+
| :mod:`Qt GUI <PySide6.QtGui>`         | Extends QtCore with GUI functionality: Events, windows |
|                                       | and screens, OpenGL and raster-based 2D painting, as   |
|                                       | well as images.                                        |
+---------------------------------------+--------------------------------------------------------+
| :mod:`Qt Widgets <PySide6.QtWidgets>` | Provides ready to use Widgets for your application,    |
|                                       | including graphical elements for your UI.              |
+---------------------------------------+--------------------------------------------------------+

QML and Qt Quick
----------------

Use these modules to interact with the `QML Language <https://doc.qt.io/qt-5.qmlapplications>`,
from Python.

+-------------------------------------------------+----------------------------------------------+
| :mod:`Qt QML <PySide6.QtQml>`                   | The base Python API to interact with the     |
|                                                 | module.                                      |
+-------------------------------------------------+----------------------------------------------+
| :mod:`Qt Quick <PySide6.QtQuick>`               | Provides classes to embed Qt Quick in Qt     |
|                                                 | applications.                                |
+-------------------------------------------------+----------------------------------------------+
| :mod:`Qt QuickWidgets <PySide6.QtQuickWidgets>` | Provides the QQuickWidget class to embed Qt  |
|                                                 | Quick in widget-based applications.          |
+-------------------------------------------------+----------------------------------------------+

Data visualization
------------------

Charts, diagrams, animations: these modules provide classes to help you include these elements in
your UI.

+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt Charts <PySide6.QtCharts>`                        | Provides a set of easy to use     |
|                                                            | chart components.                 |
+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt DataVisualization <PySide6.QtDataVisualization>`  | Provides a way to visualize data  |
|                                                            | in 3D as bar, scatter, or surface |
|                                                            | graphs.                           |
+------------------------------------------------------------+-----------------------------------+

Multimedia
-----------

Audio, video, and hardware interaction: use these modules for multimedia solutions.

+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt Multimedia <PySide6.QtMultimedia>`                | Provides low-level multimedia     |
|                                                            | functionality.                    |
+------------------------------------------------------------+-----------------------------------+
| :mod:`Qt MultimediaWidgets <PySide6.QtMultimediaWidgets>`  | Provides the widget-based         |
|                                                            | multimedia API.                   |
+------------------------------------------------------------+-----------------------------------+

WebEngine
---------

If your project is based on a browser or the features around Web-based applications, use these
modules to interact with them.

+---------------------------------------------------------+--------------------------------------+
| :mod:`Qt WebEngineWidgets <PySide6.QtWebEngineWidgets>` | Provides widgets to handle Web       |
|                                                         | content.                             |
+---------------------------------------------------------+--------------------------------------+
| :mod:`Qt WebChannel <PySide6.QtWebChannel>`             | Enables peer-to-peer communication   |
|                                                         | between a server and a client        |
|                                                         | (HTML/JavaScript or QML application).|
+---------------------------------------------------------+--------------------------------------+

All the modules
---------------

There are many other modules currently supported by |pymodname|, here you can find a complete list
of them.

  :doc:`Check all the modules <modules>`
