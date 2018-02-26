.. _qmlstatesandtransitions:

QML Tutorial 3 - States and Transitions
***************************************

In this chapter, you are guided through the steps to make the example a little more dynamic by
introducing states and transitions. For example, moving the text to the bottom of the screen,
rotate, and change its color when clicked.

.. figure:: declarative-tutorial3_animation.gif
    :align: center

Here is the QML code for such a behavior:

.. pysideinclude:: helloworld/tutorial3.qml
    :snippet: 0

Walkthrough
===========

.. pysideinclude:: helloworld/tutorial3.qml
    :snippet: 2

First, create a new *down* state for the text element.
Pressing the MouseArea activates this new state and releasing it deactivates the state.

The *down* state includes a set of property changes from the implicit *default state*
(the items as they were initially defined in the QML).
Specifically, set the ``y`` property of the text to ``160``, rotation to ``180``, and ``color`` to red.

.. pysideinclude:: helloworld/tutorial3.qml
    :snippet: 3

To make the application even better, add a transiton between the two states so that switching between these
two states look smooth and nice.

The ``from`` and ``to`` properties of the Transition element define the states between which the transition will run.
In this case, you want a transition from the default state to the *down* state.

To have a similar transition effect when changing back from the *down* state to the default state,
set the ``reversible`` property to ``true``. This is equivalent to writing two transitions.

The ParallelAnimation element makes sure that the two types of animations (number and color) start at the same time.
You could also run them one after the other by using SequentialAnimation instead.

[Previous :ref:`qmlcomponents`]
