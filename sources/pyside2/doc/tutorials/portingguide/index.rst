Porting a C++ Application to Python
*************************************

Qt for Python lets you use Qt APIs in a Python application.
So the next question is: What does it take to port an
existing C++ application? Try porting a Qt C++ application
to Python to understand this.

Before you start, ensure that all the prerequisites for
Qt for Python are met. See
:doc:`Getting Started <../../gettingstarted>` for more
information. In addition, familiarize yourself with the
basic differences between Qt in C++ and in Python.

Basic differences
==================

This section highlights some of the basic differences
between C++ and Python, and how Qt differs between these
two contexts.

C++ vs Python
--------------

* In the interest of code reuse, both C++ and Python
  provide ways for one file of code to use facilities
  provided by another. In C++, this is done using the
  ``#include`` directive to access the API definition of
  the reused code. The Python equivalent is an ``import``
  statement.
* The constructor of a C++ class shares the name of its
  class and automatically calls the constructor of any
  base-classes (in a predefined order) before it runs.
  In Python, the ``__init__()`` method is the constructor
  of the class, and it can explicitly call base-class
  constructors in any order.
* C++ uses the keyword, ``this``, to implicitly refer to
  the current object. In python, you need to explicitly
  mention the current object as the first parameter
  to each instance method of the class; it is conventionally
  named ``self``.
* And more importantly, forget about curly braces, {}, and
  semi-colon, ;.
* Precede variable definitions with the ``global`` keyword,
  only if they need global scope.

.. code:: python

    var = None
    def func(key, value = None):
      """Does stuff with a key and an optional value.

      If value is omitted or None, the value from func()'s
      last call is reused.
      """
      global var
      if value is None:
          if var is None:
              raise ValueError("Must pass a value on first call", key, value)
          value = var
      else:
          var = value
      doStuff(key, value)

In this example, ``func()`` would treat ``var`` as a local
name without the ``global`` statement.  This would lead to
a ``NameError`` in the ``value is None`` handling, on
accessing ``var``. For more information about this, see
`Python refernce documentation <python refdoc>`_.

.. _python refdoc: https://docs.python.org/3/reference/simple_stmts.html#the-global-statement

.. tip:: Python being an interpreted language, most often
   the easiest way is to try your idea in the interperter.
   You could call the ``help()`` function in the
   interpreter on any built-in function or keyword in
   Python. For example, a call to ``help(import)`` should
   provide documentation about the ``import`` statment

Last but not the least, try out a few examples to
familiarize yourself with the Python coding style and
follow the guidelines outlined in the
`PEP8 - Style Guide <pep8>`_.

.. _pep8: <https://www.python.org/dev/peps/pep-0008/#naming-conventions>

.. code-block:: python

    import sys

    from PySide2.QtWidgets import QApplication, QLabel

    app = QApplication(sys.argv)
    label = QLabel("Hello World")
    label.show()
    sys.exit(app.exec_())

.. note:: Qt provides classes that are meant to manage
   the application-specific requirements depending on
   whether the application is console-only
   (QCoreApplication), GUI with QtWidgets (QApplication),
   or GUI without QtWidgets (QGuiApplication). These
   classes load necessary plugins, such as the GUI
   libraries required by an application. In this case, it is
   QApplication that is initialized first as the application
   has a GUI with QtWidgets.

Qt in the C++ and Python context
---------------------------------

Qt behaves the same irrespective of whether it is used
in a C++ or a Python application. Considering that C++
and Python use different language semantics, some
differences between the two variants of Qt are inevitable.
Here are a few important ones that you must be aware of:

* **Qt Properties**: ``Q_PROPERTY`` macros are used in C++ to add a
  public member variable with getter and setter functions. Python's
  alternative for this is the ``@property`` decorator before the
  getter and setter function definitions.
* **Qt Signals and Slots**: Qt offers a unique callback mechanism,
  where a signal is emitted to notify the occurrence of an event, so
  that slots connected to this signal can react to it. In C++,
  the class definition must define the slots under the
  ``public Q_SLOTS:`` and signals under ``Q_SIGNALS:``
  access specifier. You connect these two using one of the
  several variants of the QObject::connect() function. Python's
  equivalent for this is the `@Slot`` decorator just before the
  function definition. This is necessary to register the slots
  with the QtMetaObject.
* **QString, QVariant, and other types**

  - Qt for Python does not provide access to QString and
    QVariant. You must use Python's native types instead.
  - QChar and QStringRef are represented as Python strings,
    and QStringList is converted to a list of strings.
  - QDate, QDateTime, QTime, and QUrl's __hash__() methods
    return a string representation so that identical dates
    (and identical date/times or times or URLs) have
    identical hash values.
  - QTextStream's bin(), hex(), and oct() functions are
    renamed to bin_(), hex_(), and oct_() respectively. This
    should avoid name conflicts with Python's built-in
    functions.

* **QByteArray**: A QByteArray is treated as a list of
  bytes without encoding. The equivalent type in Python
  varies; Python 2 uses "str" type, whereas Python 3 uses
  "bytes". To avoid confusion, a QString is represented as
  an encoded human readable string, which means it is
  a "unicode" object in Python 2, and a "str" in Python 3.

Here is the improved version of the Hello World example,
demonstrating some of these differences:

.. literalinclude:: hello_world_ex.py
   :linenos:
   :lines: 40-

.. note:: The ``if`` block is just a good practice when
   developing a Python application. It lets the Python file
   behave differently depending on whether it is imported
   as a module in another file or run directly. The
   ``__name__`` variable will have different values in
   these two scenarios. It is ``__main__`` when the file is
   run directly, and the module's file name
   (``hello_world_ex`` in this case) when imported as a
   module. In the later case, everything defined in the
   module except the ``if`` block is available to the
   importing file.

Notice that the QPushButton's ``clicked`` signal is
connected to the ``magic`` function to randomly change the
QLabel's ``text`` property. The `@Slot`` decorator marks
the methods that are slots and informs the QtMetaObject about
them.

Porting a Qt C++ example
=========================

Qt offers several C++ examples to showcase its features and help
beginners learn. You can try porting one of these C++ examples to
Python. The
`books SQL example <https://code.qt.io/cgit/qt/qtbase.git/tree/examples/sql/books>`_
is a good starting point as it does not require you to write UI-specific code in
Python, but can use its ``.ui`` file instead.

The following chapters guides you through the porting process:

.. toctree::
   :glob:
   :titlesonly:

   chapter1/chapter1
   chapter2/chapter2
   chapter3/chapter3
