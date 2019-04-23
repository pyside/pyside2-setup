Frequently Asked Questions
==========================

**When was PySide2 adopted by The Qt Company?**
  During April 2016 `The Qt Company <https://qt.io>`_ decided to properly support the port
  (`see details <https://groups.google.com/forum/#!topic/pyside-dev/pqwzngAGLWE>`_).

**PySide? Qt for Python? what is the name?**
  The name of the project is Qt for Python and the name of the module is PySide2.

**Why PySide2 and not just PySide?**
  Since PySide was developed for Qt4, when the port was made to support Qt5,
  the name is changed to PySide2 to imply that it was a newer version.

**Where I can find information about the old PySide project?**
  The old wiki page of the project is available on PySide, but the project is deprecated
  and there is no official support for it. We highly recommend not to use it.

**My project is using PySide, how hard would it be to adapt it to PySide2?**
  The changes are the same as between Qt4 and Qt5, and for PySide users it mostly means
  adapting the import statements since many classes were moved from QtGui to QtWidgets.
  Qt 5 is highly compatible with Qt 4. It is possible for developers of Qt 4 applications to
  seamlessly move to Qt 5 with their current functionality and gradually develop new things,
  leveraging all the great items Qt 5 makes possible.

**Does PySide2 support Android and iOS development / deployment?**
  At the moment there is no support for mobile platforms.

**Does PySide2 have support for embedded Linux (Raspberry Pi, i.MX6 etc)?**
  Not at the moment.

**There are three wheels (pyside2, shiboken2, and shiboken2_generator), what is the different between them?**

  Before the official release, everything was in one big wheel, but it made sense to split
  the projects in three different wheels:

   * **pyside2**: contains all the PySide2 modules to use the Qt framework.
     Also depends on the shiboken2 module.
   * **shiboken2**: contains the shiboken2 module with helper functions for PySide2.
   * **shiboken2_generator**: contains the generator binary that can work with a C++ project
     and a typesystem to generate Python bindings.
     Take into account that if you want to generate bindings for a Qt/C++ project,
     the linking to the Qt shared libraries will be missing, and you will need to do this by hand.
     We recommend to build PySide2 from scratch to have everything properly linked.

**Why shiboken2_generator is not installed automatically?**
  It's not necessary to install it to use PySide2.
  The package is the result of the wheel splitting process.
  To use the generator, it's recommended to build it from scratch to have the proper Qt-linking.
