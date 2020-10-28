.. _faq:

:orphan:

Frequently Asked Questions
==========================

**When did The Qt Company adopt PySide2?**
  In April 2016 `The Qt Company <https://qt.io>`_ decided to properly support the port. For more
  information, see `<https://groups.google.com/forum/#!topic/pyside-dev/pqwzngAGLWE>`_.

**Why use PySide2 and not PySide?**
  Since PySide was developed for Qt 4, we now use PySide2 to imply that it is for a newer version,
  after it was ported to support Qt 5.

**Where I can find information about the old PySide project?**
  The project's old wiki page is available on PySide, but the project is now deprecated and not
  supported.

**There are three wheels (pyside2, shiboken6, and shiboken6_generator), what's the difference?**

  Before the official release, everything was in one big wheel, so it made sense to split these
  into separate wheels, each for the major projects currently in development:

   * **pyside2**: contains all the PySide2 modules to use the Qt framework; also depends on the
     shiboken6 module.
   * **shiboken6**: contains the shiboken6 module with helper functions for PySide2.
   * **shiboken6_generator**: contains the generator binary that can work with a C++ project and a
     typesystem to generate Python bindings.
     If you want to generate bindings for a Qt/C++ project, there won't be any linking to the Qt
     shared libraries; you need to do this by hand. We recommend building PySide2 from scratch
     to have everything properly linked.

**Why is the shiboken6_generator not installed automatically?**
  It's not necessary to install the shiboken6_generator to use PySide2. The package is a result of
  the wheel splitting process. To use the generator, it's recommended to build it from scratch to
  have the proper Qt linking.
