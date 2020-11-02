.. currentmodule:: PySide6.QtCore
.. _QEnum:

QEnum/QFlag
***********

This class decorator is equivalent to the `Q_ENUM` macro from Qt.
The decorator is used to register an Enum to the meta-object system,
which is available via `QObject.staticMetaObject`.
The enumerator must be in a QObject derived class to be registered.


Example
-------

::

    from enum import Enum, Flag, auto

    from PySide6.QtCore import QEnum, QFlag, QObject

    class Demo(QObject):

        @QEnum
        class Orientation(Enum):
            North, East, South, West = range(4)

        class Color(Flag):
            RED = auto()
            BLUE = auto()
            GREEN = auto()
            WHITE = RED | BLUE | GREEN

        QFlag(Color)    # identical to @QFlag usage


Caution:
--------

QEnum registers a Python Enum derived class.
QFlag treats a variation of the Python Enum, the Flag class.

Please do not confuse that with the Qt QFlags concept. Python does
not use that concept, it has its own class hierarchy, instead.
For more details, see the `Python enum documentation <https://docs.python.org/3/library/enum.html>`_.


Details about Qt Flags:
-----------------------

There are some small differences between Qt flags and Python flags.
In Qt, we have for instance these declarations:

::

    enum    QtGui::RenderHint { Antialiasing, TextAntialiasing, SmoothPixmapTransform,
                                HighQualityAntialiasing, NonCosmeticDefaultPen }
    flags   QtGui::RenderHints

The equivalent Python notation would look like this:

::

    @QFlag
    class RenderHints(enum.Flag)
        Antialiasing = auto()
        TextAntialiasing = auto()
        SmoothPixmapTransform = auto()
        HighQualityAntialiasing = auto()
        NonCosmeticDefaultPen = auto()


As another example, the Qt::AlignmentFlag flag has 'AlignmentFlag' as the enum
name, but 'Alignment' as the type name. Non flag enums have the same type and
enum names.

::

    enum Qt::AlignmentFlag
    flags Qt::Alignment

The Python way to specify this would be

::

    @QFlag
    class Alignment(enum.Flag):
        ...

We are considering to map all builtin enums and flags to Python enums as well
in a later release.

