scriptableapplication demonstrates how to make a Qt C++ application scriptable.

It has a class MainWindow inheriting QMainWindow for which bindings are generated
using PySide2's shiboken2 bindings generator.

The header wrappedclasses.h is passed to shiboken2 which generates class
wrappers and headers in a subdirectory which are linked into the application.

pythonutils.cpp has some code which binds the instance of MainWindow
to a variable 'mainWindow' in the global (__main___) namespace.
It is then possible to run Python script snippets like
mainWindow.testFunction1() which trigger the underlying C++ function.

Virtualenv Support
If the application is started from a terminal with an activated python virtual environment, that
environment's packages will be used for the python module import process. In this case, make sure
that the application was built while the virtualenv was active, so that the build system picks up
the correct python shared library.

Windows Notes
The build config of the application (Debug or Release) should match the PySide2 build config,
otherwise the application will not function correctly. In practice this means the only supported
configurations are:
1) qmake release config build of the application + PySide2 setup.py without "--debug" flag +
   python.exe for the PySide2 build process + python36.dll for the linked in shared library +
   release build of Qt.
2) qmake debug config build of the application + PySide2 setup.py WITH "--debug" flag +
   python_d.exe for the PySide2 build process + python36_d.dll for the linked in shared library +
   debug build of Qt.
This is necessary because all the shared libraries in question have to link to the same C++ runtime
library (msvcrt.dll or msvcrtd.dll).
To make the example as self-contained as possible, the shared libraries in use (pyside2.dll,
shiboken2.dll) are hard-linked into the build folder of the application.
