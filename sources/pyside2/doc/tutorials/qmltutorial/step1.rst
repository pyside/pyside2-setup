.. _qmlbasictypes:

QML Tutorial 1 - Basic Types
****************************

This first program is a very simple "Hello world" example that introduces some basic QML concepts.
The following image is a screenshot of this program.

.. figure:: declarative-tutorial1.png
    :align: center

Here is the QML code for the application:

.. pysideinclude:: helloworld/tutorial1.qml
    :snippet: 0

Walkthrough
===========

Import
------

First, you need to import the types that are required for this example. Most QML files import the built-in QML
types (like Rectangle, Image, ...) that come with Qt, using the following statement:

.. pysideinclude:: helloworld/tutorial1.qml
    :snippet: 3

Rectangle element
-----------------

.. pysideinclude:: helloworld/tutorial1.qml
    :snippet: 1

Declare a root element using the Rectangle type, which is one of the basic building blocks to create an application in QML.
Give it an ``id`` so that you can refer to it later. For example, call it "page", and also set its ``width``,
``height``, and ``color`` properties.

Text element
------------

.. pysideinclude code/tutorial1.qml
    :snippet: 2

Add a Text element as the child of the Rectangle element to display the text, 'Hello world!'.

Use its ``y`` property to position it at 30 pixels from the top of its parent.

The ``anchors.horizontalCenter`` property refers to the horizontal center of an element.
In this case, specify that the text element must be horizontally centered in the *page* element.

The ``font.pointSize`` and ``font.bold properties`` are related to fonts and use the dot notation.


Viewing the example
-------------------

To view what you have created, run the ``qmlscene`` tool (located in the ``bin directory`` of your Qt installation) with your
QML filename as the first argument. For example, to run the Tutorial 1 example from the install
location, you would type:

::

    > [QT_INSTALL_DIR]\bin\qmlscene tutorial1.qml

[Previous :ref:`qmltutorial`][Next :ref:`qmlcomponents`]
