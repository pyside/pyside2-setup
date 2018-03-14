.. _samegame3:

QML Advanced Tutorial 3 - Implementing the Game Logic
*****************************************************

Making a playable game
======================

Now that all the UI components are ready, add the game logic that
dictates how a player interacts with the blocks and plays the game,
until it is won or lost.

To achieve this, add the following functions to ``samegame.js``:


* ``handleClick(x,y)``
* ``floodFill(xIdx,yIdx,type)``
* ``shuffleDown()``
* ``victoryCheck()``
* ``floodMoveCheck(xIdx, yIdx, type)``

As this is a tutorial about QML, not game design, only the ``handleClick()``
and ``victoryCheck()`` functions that interface directly with the QML items
are discussed.

.. Note:: Although the game logic here is written in JavaScript,
   it could have been written in Python and then exposed to QML.

Enabling mouse click interaction
================================

To make it easier for the JavaScript code to interface with the QML items,
an Item called ``gameCanvas`` is added to ``samegame.qml``. It replaces the
background as the item which contains the blocks. It also accepts mouse input
from the user.  Here is the item code:

.. pysideinclude:: samegame/samegame3/samegame.qml
    :snippet: 1

The ``gameCanvas`` item is of the same size as the board, and has a ``score``
property and a MouseArea to handle mouse clicks. The blocks are now created as
its children, and its dimensions are used to determine the board size
so that the application scales to the available screen size. As the item's size
is bound to a multiple of ``blockSize``, ``blockSize`` is moved into ``samegame.qml``
from ``samegame.js``, as a QML property.

.. Note:: The ``blockSize`` can still be accessed from the script.

When clicked, the MouseArea calls ``handleClick()`` in ``samegame.js``, which
determines whether the player's click should cause any blocks to be removed,
and updates ``gameCanvas.score`` with the current score if necessary. Here is
the ``handleClick()`` function:

.. pysideinclude:: samegame/samegame3/samegame.js
    :snippet: 1

.. Note:: If ``score`` was a global variable in the ``samegame.js`` file,
   you would not be able to bind to it. You can only bind to QML properties.

Updating the score
==================

When the player clicks a block and triggers \c handleClick(), \c handleClick()
also calls \c victoryCheck() to update the score and to check whether the
player has completed the game. Here is the \c victoryCheck() code:

.. pysideinclude:: samegame/samegame3/samegame.js
    :snippet: 2

This updates the ``gameCanvas.score`` value and displays a "Game Over" dialog
if the game is finished.

The Game Over dialog is created using a ``Dialog`` item that is defined in
``Dialog.qml``. Here is the ``Dialog.qml`` code:

.. pysideinclude:: samegame/samegame3/Dialog.qml
    :snippet: 0

Notice how it is designed to be usable imperatively from the script file, via
the functions and signals. And this is how it is used in the main
``samegame.qml`` file:

.. pysideinclude:: samegame/samegame3/samegame.qml
    :snippet: 2

Give the dialog a ``z`` value of 100 to ensure it is displayed on top of our
other components. The default ``z`` value for an item is 0.


A dash of color
---------------

It's not much fun to play Same Game if all the blocks are of the same color, so
the ``createBlock()`` function in ``samegame.js`` randomly changes the color
to create a different type of block (for either red, green or blue) each time
it is called. ``Block.qml`` has also changed so that each block contains a
different image depending on its type:

.. pysideinclude:: samegame/samegame3/Block.qml
    :snippet: 0


A working game
==============

You now have a working game! The blocks can be clicked, the player can
score, and the game can end (and then you can start a new one).
Here is a screenshot of what has been accomplished so far:

.. figure:: declarative-adv-tutorial3.png
    :align: center

This is what ``samegame.qml`` looks like now:

.. pysideinclude:: samegame/samegame3/samegame.qml
    :snippet: 0

The game works, but it's a little boring right now. Where are the smooth
animated transitions? Where are the high scores?
If you were a QML expert, you could have written these in the first
iteration, but in this tutorial they've been saved until the next chapter
- where your application becomes alive!

[Previous :ref:`samegame2`] [Next :ref:`samegame4`]
