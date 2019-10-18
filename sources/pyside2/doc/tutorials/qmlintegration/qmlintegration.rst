########################
QML Integration Tutorial
########################

This tutorial provides a quick walk-through of a python application that loads, and interacts with
a QML file.  QML is a declarative language that lets you design UIs faster than a traditional
language, such as C++.  The QtQml and QtQuick modules provides the necessary infrastructure for
QML-based UIs.

In this tutorial, you will learn how to integrate Python with a QML application through a context
property.  This mechanism will help us to understand how to use Python as a backend for certain
signals from the UI elements in the QML interface.  Additionally, you will learn how to provide
a modern look to your QML application using one of the features from Qt Quick Controls 2.

The tutorial is based on an application that allow you to set many text properties, like increasing
the font size, changing the color, changing the style, and so on.  Before you begin, install the
`PySide2 <https://pypi.org/project/PySide2/>`_ Python packages.

The following step-by-step process will guide you through the key elements of the QML based
application and PySide2 integration:

#. First, let's start with the following QML-based UI:

   .. image:: textproperties_default.png

   The design is based on a `GridLayout`, containing two `ColumnLayout`.
   Inside the UI you will find many `RadioButton`, `Button`, and a `Slider`.

#. With the QML file in place, you can load it from Python:

   .. literalinclude:: main.py
      :linenos:
      :lines: 98-108
      :emphasize-lines: 103,107

   Notice that we specify the name of the context property, **con**,
   and also we explicitly load our QML file.

#. Define the `Bridge` class, containing all the logic for the context property:

   .. literalinclude:: main.py
      :linenos:
      :lines: 51-91

#. Now, go back to the QML file and connect the signals to the slots defined in the `Bridge` class:

   .. literalinclude:: view.qml
      :linenos:
      :lines: 85-93
      :emphasize-lines: 89-91

   The properties *Italic*, *Bold*, and *Underline* are mutually
   exclusive, this means only one can be active at any time.
   To achieve  this each time we select one of these options, we
   check the three properties via the context property as you can
   see in the above snippet.
   Only one of the three will return *True*, while the other two
   will return *False*, that is how we make sure only one is being
   applied to the text.

#. Each slot verifies if the selected option contains the text associated
   to the property:

   .. literalinclude:: main.py
      :linenos:
      :lines: 79-84
      :emphasize-lines: 82,84

   Returning *True* or *False* allows you to activate and deactivate
   the properties of the QML UI elements.

   It is also possible to return other values that are not *Boolean*,
   like the slot in charge of returning the font size:

   .. literalinclude:: main.py
      :linenos:
      :lines: 64-70

#. Now, for changing the look of our application, you have two options:

   1. Use the command line: execute the python file adding the option, `--style`::

       python main.py --style material

   2. Use a `qtquickcontrols2.conf` file:

      .. literalinclude:: qtquickcontrols2.conf
         :linenos:

      Then add it to your `.qrc` file:

      .. literalinclude:: style.qrc
         :linenos:

      Generate the *rc* file running, `pyside2-rcc style.qrc > style_rc.py`
      And finally import it from your `main.py` script.

   .. literalinclude:: main.py
      :linenos:
      :lines: 41-48
      :emphasize-lines: 48

      You can read more about this configuration file
      `here <https://doc.qt.io/qt-5/qtquickcontrols2-configuration.html>`_.

   The final look of your application will be:

   .. image:: textproperties_material.png

You can :download:`view.qml <view.qml>` and
:download:`main.py <main.py>` to try this example.
