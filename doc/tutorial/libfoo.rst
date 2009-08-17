.. highlight:: cpp

.. _gentut-libfoo:

Creating the foo library
=========================

In this section it will be presented the code and the build instructions for a
very simple Qt4 based library. It will be used as the subject for this tutorial.

The Source Code
---------------

There is only one class on this foo library plus a ``.pro`` file which means
that the build system used will be Trolltech's **qmake**.

Put the files below in a directory called **libfoo**. Be aware that this
directory will be refered by the binding Makefile presented in a next section
of this tutorial. If you want to use other names or paths change the binding
Makefile accordingly. Blind copy'n'paste shortens your life.

**libfoo/foo.h**
::

    #ifndef FOO_H
    #define FOO_H

    #include <QtCore/QtCore>

    class Math : public QObject
    {
        Q_OBJECT
    public:
        Math() {}
        virtual ~Math() {}
        int squared(int x);
    };
    #endif // FOO_H


**libfoo/foo.cpp**
::

    #include "foo.h"

    int Math::squared(int x)
    {
      return x * x;
    }


**libfoo/foo.pro**
::

    TEMPLATE = lib
    TARGET = foo
    DEPENDPATH += .
    INCLUDEPATH += .
    HEADERS += foo.h
    SOURCES += foo.cpp

To build the lib:

::

    $ cd libfoo
    $ qmake
    $ make
