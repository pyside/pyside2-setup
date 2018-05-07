.. _samegame4:

QML Advanced Tutorial 4 - Finishing Touches
*******************************************

Adding some flair
=================

In this chapter, you are going to do two things to enhance the game
experience: animate the blocks and add a High Score system.

You should clean up the directory structure, now that there are a
lot of files. Move all the JavaScript and QML files outside of ``samegame.qml``
into a new sub-directory named "content".

In anticipation of the new block animations, ``Block.qml`` file is now renamed
to ``BoomBlock.qml``.

Animating block movement
------------------------

First, you will animate the blocks so that they move in a fluid manner. QML has
a number of methods for adding fluid movement, and in this case you are going to
use the Behavior type to add a SpringAnimation. In ``BoomBlock.qml``, apply a
SpringAnimation behavior to the ``x`` and ``y`` properties so that the
block follows and animate its movement in a spring-like fashion towards the
specified position (whose values are set by ``samegame.js``). Here is the code
added to ``BoomBlock.qml``:

.. pysideinclude:: samegame/samegame4/content/BoomBlock.qml
    :snippet: 1

The ``spring`` and ``damping`` values can be changed to modify the spring-like
effect of the animation.

The ``enabled: spawned`` setting refers to the ``spawned`` value that comes from
the ``createBlock()`` function in ``samegame.js``. This ensures that the
SpringAnimation on ``x`` is only enabled after ``createBlock()`` has set the
block to the correct position. Otherwise, the blocks will slide out of the
corner (0,0) when a game begins, instead of falling from the top in rows.
Try commenting out the line, ``enabled: spawned``, and see the effect for
yourself.

Animating block opacity changes
-------------------------------

Next, add a smooth exit animation. For this, use a Behavior type, which
allows us to specify a default animation when a property change occurs. In this
case, when the ``opacity`` of a Block changes, animate the opacity value so that
it gradually fades in and out, instead of abruptly changing between fully
visible and invisible. To do this, apply a Behavior on the ``opacity`` property
of the ``Image`` item in ``BoomBlock.qml``:

.. pysideinclude:: samegame/samegame4/content/BoomBlock.qml
    :snippet: 2

Note the ``opacity: 0``, which means the block is transparent when it is first
created. You could set the opacity in ``samegame.js`` when we create and
destroy the blocks, but use states instead, as this is useful for the next
animation you are going to add. Initially, add these States to the root
item of ``BoomBlock.qml``:

::

    property bool dying: false
    states: [
        State{ name: "AliveState"; when: spawned == true && dying == false
            PropertyChanges { target: img; opacity: 1 }
        },
        State{ name: "DeathState"; when: dying == true
            PropertyChanges { target: img; opacity: 0 }
        }
    ]

Now blocks will automatically fade in, as ``spawned`` is set to true when
you implemented the block animations. To fade out, set ``dying`` to true
instead of setting opacity to 0 when a block is destroyed (in the
``floodFill()`` function).

Adding particle effects
-----------------------

Finally, add a cool-looking particle effect to the blocks when they are
destroyed. To do this, first add a Particles item in
``BoomBlock.qml``, like this:

.. pysideinclude:: samegame/samegame4/content/BoomBlock.qml
    :snippet: 3

To fully understand this you should read the Particles documentation,
but it's important to note that ``emissionRate`` is set to zero so that
particles are not emitted normally. Also, extend the ``dying`` State,
which creates a burst of particles by calling the ``burst()`` method on the
particles item. The code for the states now look like this:

.. pysideinclude:: samegame/samegame4/content/BoomBlock.qml
    :snippet: 4

Now the gaming experience is pleasing with these animations. With a few
more simple animations for all of the player's actions, it will look even better.
The end result is shown below, with a different set of images to demonstrate
the basic theme:

.. figure:: declarative-adv-tutorial4.gif
    :align: center

The theme change here is produced simply by replacing the block images. This
can be done at runtime by changing the ``source`` property of the Image. You
could go a step further and add a button that toggles between themes with
different images.

Keeping a high scores table
===========================

Another feature you might want to add to the game is a method of storing and
retrieving high scores.

To do this, show a dialog when the game is over to request the player's name
and add it to a High Scores table. This requires a few changes to
``Dialog.qml``. In addition to a ``Text`` item, it now has a ``TextInput``
child item for receiving keyboard text input:

.. pysideinclude:: samegame/samegame4/content/Dialog.qml
    :snippet: 2
    :prepend: Rectangle {
              ...
    :append: ...
             }


Also, add a ``showWithInput()`` function. The text input will only be visible if
this function is called instead of ``show()``. When the dialog is closed, it
emits a ``closed()`` signal, and other items can retrieve the text entered by
the user through the ``inputText`` property:

.. pysideinclude:: samegame/samegame4/content/Dialog.qml
    :snippet: 1
    :prepend: Rectangle {
              ...
    :append: ...
             }

Now the dialog can be used in ``samegame.qml``:

.. pysideinclude:: samegame/samegame4/samegame.qml
    :snippet: 0

When the dialog emits the ``closed`` signal, we call the new ``saveHighScore()``
function in ``samegame.js``, to store the high score locally in an SQL database
and also send the score to an online database if possible.

The ``nameInputDialog`` is activated in the ``victoryCheck()`` function in
``samegame.js``:

.. pysideinclude:: samegame/samegame4/content/samegame.js
    :snippet: 4
    :prepend: function vitoryCheck() {
              ...

Storing high scores offline
---------------------------

Now, you need to implement the functionality to actually save the High Scores table.

Here is the ``saveHighScore()`` function in ``samegame.js``:

.. pysideinclude:: samegame/samegame4/content/samegame.js
    :snippet: 2

First, call ``sendHighScore()`` to send the high scores to an online database.

Then, use the Offline Storage API to maintain a persistent SQL database, unique
to this application. Create an offline storage database for the high scores
using ``openDatabase()``, then prepare the data and SQL query that we want to use
to save it. The offline storage API uses SQL queries for data manipulation and
retrieval. The ``db.transaction()`` uses three SQL queries:
   * To initialize the database, if necessary.
   * To add high scores to the database.
   * To retrieve the high score records.

To use the returned records, turn it into a string with one line per row, and show
a dialog containing that string.

This is one way of storing and displaying high scores locally, but certainly
not the only way. A more complex alternative would be to create a high score
dialog component, and pass it the results for processing and display (instead
of reusing the ``Dialog``). This allows for a more themeable dialog that could
present the high scores in a better way. If you are using QML-based UI for a
Python application, you can also pass the score to a function that stores it
locally in a variety of ways. This can be a simple format without SQL, or in
another SQL database.

Storing high scores online
--------------------------

You've seen how you can store high scores locally, but it is also easy to
integrate a web-enabled high score storage into your application. The
implementation we've done here is very simple: the high score data is posted to
a php script running on a server somewhere, and that server then stores it and
displays it to visitors. You could also request an XML or QML file, which
contains and displays the scores, but that's beyond the scope of this tutorial.
The php script used here is available in the ``examples`` directory.

If the player entered their name, you can send the data to an online database
service. The following code snippet from ``samegame.js`` demonstrates this well:

.. pysideinclude:: samegame/samegame4/content/samegame.js
    :snippet: 1

The XMLHttpRequest in this code is the same as the ``XMLHttpRequest()`` as you'll
find in standard browser JavaScript, and can be used in the same way to
dynamically get XML or QML from the web service to display the high scores. We don't worry about the response in this case - we just post the high
score data to the web server. If it had returned a QML file (or a URL to a QML file) you could instantiate it in much the same
way as you did with the blocks.

An alternate way to access and submit web-based data would be to use QML items designed for this purpose. XmlListModel
makes it very easy to fetch and display XML based data such as RSS in a QML application (see the Flickr demo for an example).


That's it!
==========

By following this tutorial you've seen how you can write a fully functional application in QML:

* Build your application with QML items.
* Add application logic with JavaScript code.
* Add animations with Behaviors and states.
* Store persistent application data using, for example, the Offline Storage API or XMLHttpRequest.

There is so much more to learn about QML that we haven't been able to cover in this tutorial. Check out all the
demos and examples and the documentation to see all the things you can do with QML!

[Previous :ref:`samegame3`]
