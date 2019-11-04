|project|
*************

|project| offers Python bindings for Qt, enabling the use of Qt5 APIs in Python
applications. It lets Python developers utilize the full potential of Qt, using
the |pymodname| module.

The |pymodname| module provides access to the individual
Qt modules such as QtCore, QtGui, and so on. |project| also comes with the
:doc:`Shiboken2 <shiboken2:index>` CPython binding code generator, which can be
used to generate Python bindings for your C or C++ code.

.. toctree::
    :name: mastertoc
    :maxdepth: 2

    contents.rst
    Known issues <https://wiki.qt.io/Qt_for_Python/Considerations>

Qt Modules
===========

Basic modules
-------------
  These are the main modules that will help you build a Widget based UI.

  :mod:`Qt Core <PySide2.QtCore>`
    Provides core non-GUI functionality, like signal and slots, properties, base classes of item models, serialization, etc.
  :mod:`Qt Gui <PySide2.QtGui>`
    Extends QtCore with GUI functionality: Events, windows and screens, OpenGL and raster-based 2D painting, images.
  :mod:`Qt Widgets <PySide2.QtWidgets>`
    Ready to use Widgets for your application, including also graphical elements for your UI.

QML and Qt Quick
----------------
  If you want to use the `Qml Language <https://doc.qt.io/qt-5.qmlapplications>`, these
  modules will help you interact with it from Python.

  :mod:`Qt Qml <PySide2.QtQml>`
    Base Python API to interact with the QML module.
  :mod:`Qt Quick <PySide2.QtQuick>`
    Provides classes for embedding Qt Quick in Qt applications.
  :mod:`Qt QuickWidgets <PySide2.QtQuickWidgets>`
    Provides the QQuickWidget class for embedding Qt Quick in widget-based applications.

Data visualization
------------------

  Charts, diagrams, animations: these modules provide a large amount
  of classes that can help you include these elements in your UI.

  :mod:`Qt Charts <PySide2.QtCharts>`
    Provides a set of easy to use chart components.
  :mod:`Qt DataVisualization <PySide2.QtDataVisualization>`
    Provides a way to visualize data in 3D as bar, scatter, and surface graphs.

Multimedia
-----------

  Audio, video, and hardware interaction: check these modules if you are
  looking for multimedia solutions.

  :mod:`Qt Multimedia <PySide2.QtMultimedia>`
    Provides low-level multimedia functionality.
  :mod:`Qt MultimediaWidgets <PySide2.QtMultimediaWidgets>`
    Provides the widget-based multimedia API.

WebEngine
---------

  If your project is based on a browser or the features around web
  based applications, these modules will help you to interact with them.

  :mod:`Qt WebEngineWidgets <PySide2.QtWebEngineWidgets>`
    Provides widgets that can handle web content.
  :mod:`Qt WebChannel <PySide2.QtWebChannel>`
    Enables peer-to-peer communication between a server and a client
    (HTML/JavaScript or QML application).

All the modules
---------------

  There are many other modules currently supported by |pymodname|,
  here you can find a complete list of them.

  :doc:`Check all the modules <modules>`
   Display a table with all the currently supported Qt modules.
