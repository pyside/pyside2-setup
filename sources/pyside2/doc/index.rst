|project|
*************

The |project| product enables the use of Qt5 APIs in Python applications. It
lets Python developers utilize the full potential of Qt, using the |pymodname| module.
The |pymodname| module provides access to the individual Qt modules such as QtCore,
QtGui, and so on. The following is the list of supported Qt5 modules:

Qt Modules
===========

.. list-table::
   :widths: 150, 150
   :align: left

   * - `Qt Core <PySide2/QtCore/index.html>`_
        Provides core non-GUI functionality.
     - `Qt 3D Animation <PySide2/Qt3DAnimation/index.html>`_
        Provides basic elements required to animate 3D objects.
   * - `Qt Gui <PySide2/QtGui/index.html>`_
        Extends QtCore with GUI functionality.
     - `Qt Help <PySide2/QtHelp/index.html>`_
        Provides classes for integrating online documentation in applications.
   * - `Qt Network <PySide2/QtNetwork/index.html>`_
        Offers classes that lets you to write TCP/IP clients and servers.
     - `Qt OpenGL <PySide2/QtOpenGL/index.html>`_
        Offers classes that make it easy to use OpenGL in Qt applications.
   * - `Qt PrintSupport <PySide2/QtPrintSupport/index.html>`_
        Offers classes that make it easy to use OpenGL in Qt applications.
     - `Qt Qml <PySide2/QtQml/index.html>`_
        Python API for Qt QML.
   * - `Qt Charts <PySide2/QtCharts/index.html>`_
        Provides a set of easy to use chart components.
     - `Qt Quick <PySide2/QtQuick/index.html>`_
        Provides classes for embedding Qt Quick in Qt applications.
   * - `Qt DataVisualization <PySide2/QtDataVisualization/index.html>`_
        Provides a way to visualize data in 3D as bar, scatter, and surface graphs.
     - `Qt QuickWidgets <PySide2/QtQuickWidgets/index.html>`_
        Provides the QQuickWidget class for embedding Qt Quick in widget-based applications.
   * - `Qt TextToSpeech <PySide2/QtTextToSpeech/index.html>`_
        Provides API to access text-to-speech engines.
     - `Qt Sql <PySide2/QtSql/index.html>`_
        Helps you provide seamless database integration to your Qt applications.
   * - `Qt Multimedia <PySide2/QtMultimedia/index.html>`_
        Provides low-level multimedia functionality.
     - `Qt MultimediaWidgets <PySide2/QtMultimediaWidgets/index.html>`_
        Provides the widget-based multimedia API.
   * - `Qt MacExtras <PySide2/QtMacExtras/index.html>`_
        Provides classes and functions specific to
        macOS and iOS operating systems.
     - `Qt Svg <PySide2/QtSvg/index.html>`_
        Provides classes for displaying the contents of SVG files.
   * - `Qt UiTools <PySide2/QtUiTools/index.html>`_
        Provides classes to handle forms created with Qt Designer.
     - `Qt Test <PySide2/QtTest/index.html>`_
        Provides classes for unit testing Qt applications and libraries.
   * - `Qt Concurrent <PySide2/QtConcurrent/index.html>`_
        Provides high-level APIs that make it possible
        to write multi-threaded programs without using low-level threading
        primitives such as mutexes, read-write locks, wait conditions, or semaphores.
     - `Qt AxContainer <PySide2/QtAxContainer/index.html>`_
        Provides QAxObject and QAxWidget which act as
        containers for COM objects and ActiveX controls.
   * - `Qt WebChannel <PySide2/QtWebChannel/index.html>`_
        Enables peer-to-peer communication between a server and a client
        (HTML/JavaScript or QML application).
     - `Qt WebSockets <PySide2/QtWebSockets/index.html>`_
        Provides interfaces that enable Qt applications
        to act as a server that can process WebSocket requests, or a client that
        can consume data received from the server, or both.
   * - `Qt Widgets <PySide2/QtWidgets/index.html>`_
        Extends Qt GUI with C++ widget functionality.
     - `Qt WinExtras <PySide2/QtWinExtras/index.html>`_
        Provides classes and functions for using some Windows APIs in a Qt way.
   * - `Qt X11Extras <PySide2/QtX11Extras/index.html>`_
        Provides information about the X display configuration.
     - `Qt Xml <PySide2/QtXml/index.html>`_
        Provides a stream reader and writer for XML documents.
   * - `Qt XmlPatterns <PySide2/QtXmlPatterns/index.html>`_
        Provides support for XPath, XQuery, XSLTi, and XML Schema validation.
     - `Qt 3D Core <PySide2/Qt3DCore/index.html>`_
        Contains functionality to support near-realtime simulation systems.
   * - `Qt 3D Extras <PySide2/Qt3DExtras/index.html>`_
        Provides a set of prebuilt elements to help you get started with Qt 3D.
     - `Qt 3D Input <PySide2/Qt3DInput/index.html>`_
        Provides classes for handling user input in applications using Qt 3D.
   * - `Qt 3D Logic <PySide2/Qt3DLogic/index.html>`_
        Enables synchronizing frames with the Qt 3D backend.
     - `Qt 3D Render <PySide2/Qt3DRender/index.html>`_
        Contains functionality to support 2D and 3D rendering using Qt 3D.
   * - `Qt Positioning <PySide2/QtPositioning/index.html>`_
        Provides positioning information via QML and Python interfaces.
     - `Qt Location <PySide2/QtLocation/index.html>`_
        Helps you create viable mapping solutions using the data available from some of the popular location services.
   * - `Qt Sensors <PySide2/QtSensors/index.html>`_
        Provides access to sensor hardware via QML and Python interfaces and a motion gesture recognition API for devices.
     - `Qt Scxml <PySide2/QtScxml/index.html>`_
        Provides classes to create and use state machines from SCXML files.

|project| also comes with the
:doc:`Shiboken2 <shiboken2:contents>` generator that outputs C++ code
for CPython extensions.

.. toctree::
    :maxdepth: 2

    contents.rst
