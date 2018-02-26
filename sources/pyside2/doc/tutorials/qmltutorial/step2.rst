.. _qmlcomponents:

QML Tutorial 2 - QML Components
*******************************

In this chapter, you are going to add a color picker to change the color of the text.

.. figure:: declarative-tutorial2.png
    :align: center

The color picker is made of six cells with different colors.
To avoid writing the same code multiple times for each cell, create a new ``Cell`` component.
A component provides a way of defining a new type that you can re-use in other QML files.
A QML component is like a black-box that interacts with the outside world through its properties, signals,
and functions, and is generally defined in its own QML file.
The component's filename must always start with a capital letter.

Here is the QML code for ``Cell``:

.. pysideinclude:: helloworld/Cell.qml
    :snippet: 0

Walkthrough
===========

The Cell Component
------------------

.. pysideinclude:: helloworld/Cell.qml
    :snippet: 1

The root element of the component is an Item with the ``id``, *container*.
An Item is the most basic visual element in QML and is often used as a container for other elements.

.. pysideinclude:: helloworld/Cell.qml
    :snippet: 4

Its ``cellColor`` property is accessible from  *outside* the component, allowing you
to instantiate cells with different colors. It is an alias to the existing color property of the rectangle
that composes the cell.

.. pysideinclude:: helloworld/Cell.qml
    :snippet: 5

The Cell has a signal called *clicked* with the *cellColor* parameter of type *color*.
You need this signal to change the color of the text in the main QML file later.

.. pysideinclude:: helloworld/Cell.qml
    :snippet: 2

The cell component is basically a colored rectangle with the ``id``, *rectangle*.

Its ``anchors.fill`` property is a convenient way to set the size of an element.
In this case the rectangle will have the same size as its parent.

.. pysideinclude:: helloworld/Cell.qml
    :snippet: 3

In order to change the color of the text when the cell is clicked, a MouseArea element with
the same size as its parent is used.

A MouseArea enables you to react to mouse events such as clicked, hover, and so on. In this case, when the MouseArea *clicked*
signal is reported, the Cell's *clicked* signal is emitted.

The main QML file
-----------------

In the main QML file, use the ``Cell`` component to create the color picker:

.. pysideinclude:: helloworld/tutorial2.qml
    :snippet: 0

Create the color picker by putting 6 cells with different colors in a grid.

.. pysideinclude:: helloworld/tutorial2.qml
    :snippet: 1

When the *clicked* signal of a cell is triggered, set the color of the text to the *cellColor* passed as a parameter.
You can react to a signal of a component through a handler of the name, *'onSignalName'*.

[Previous :ref:`qmlbasictypes`][Next :ref:`qmlstatesandtransitions`]
