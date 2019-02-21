#########################
QML Application Tutorial
#########################

This tutorial provides a quick walk-through of a python application
that loads a QML file. QML is a declarative language that lets you
design UIs faster than a traditional language, such as C++. The
QtQml and QtQuick modules provides the necessary infrastructure for
QML-based UIs.

In this tutorial, you'll also learn how to provide data from Python
as a QML context property, which is then consumed by the ListView
defined in the QML file.

Before you begin, install the following prerequisites:

* The `PySide2 <https://pypi.org/project/PySide2/>`_ Python packages.
* Qt Creator v4.9 beta1 or later from
  `http://download.qt.io
  <http://download.qt.io/snapshots/qtcreator/4.9/4.9.0-beta1/>`_.


The following step-by-step instructions guide you through application
development process using Qt Creator:

#. Open Qt Creator and select **File > New File or Project..** menu item
   to open following dialog:

   .. image:: newpyproject.png

#. Select **Qt for Python - Empty** from the list of application templates
   and select **Choose**.

   .. image:: pyprojname.png

#. Give a **Name** to your project, choose its location in the
   filesystem, and select **Finish** to create an empty ``main.py``
   and ``main.pyproject``.

   .. image:: pyprojxplor.png

   This should create a ``main.py`` and ```main.pyproject`` files
   for the project.

#. Download :download:`view.qml<view.qml>` and :download:`logo.png <logo.png>`
   and move them to your project folder.

#. Double-click on ``main.pyproject`` to open it in edit mode, and append
   ``view.qml`` and ``logo.png`` to the **files** list. This is how your
   project file should look after this change:

   .. code::

    {
        "files": ["main.py", "view.qml", "logo.png"]
    }

#. Now that you have the necessary bits for the application, import the
   Python modules in your ``main.py``, and download country data and
   format it:

   .. literalinclude:: main.py
      :linenos:
      :lines: 40-60
      :emphasize-lines: 12-20

#. Now, set up the application window using
   :ref:`PySide2.QtGui.QGuiApplication<qguiapplication>`, which manages the application-wide
   settings.

   .. literalinclude:: main.py
      :linenos:
      :lines: 40-65
      :emphasize-lines: 23-25

   .. note:: Setting the resize policy is important if you want the
      root item to resize itself to fit the window or vice-a-versa.
      Otherwise, the root item will retain its original size on
      resizing the window.

#. You can now expose the ``data_list`` variable as a QML context
   property, which will be consumed by the QML ListView item in ``view.qml``.

   .. literalinclude:: main.py
      :linenos:
      :lines: 40-70
      :emphasize-lines: 27-30

#. Load the ``view.qml`` to the ``QQuickView`` and call ``show()`` to
   display the application window.

   .. literalinclude:: main.py
      :linenos:
      :lines: 40-79
      :emphasize-lines: 33-39

#. Finally, execute the application to start the event loop and clean up.

   .. literalinclude:: main.py
      :linenos:
      :lines: 40-
      :emphasize-lines: 41-43

#. Your application is ready to be run now. Select **Projects** mode to
   choose the Python version to run it.

   .. image:: projectsmode.png

Run the application by using the ``CTRL+R`` keyboard shortcut to see if it
looks like this:

.. image:: qmlapplication.png

You could also watch the following video tutorial for guidance to develop
this application:

.. raw:: html

    <div style="position: relative; padding-bottom: 56.25%; height: 0;
    overflow: hidden; max-width: 100%; height: auto;">
        <iframe src="https://www.youtube.com/embed/JxfiUx60Mbg" frameborder="0"
        allowfullscreen style="position: absolute; top: 0; left: 0;
        width: 100%; height: 100%;">
        </iframe>
    </div>

********************
Related information
********************

* `QML Reference <https://doc.qt.io/qt-5/qmlreference.html>`_
* :doc:`../qmltutorial/index`
* :doc:`../qmladvancedtutorial/index`
