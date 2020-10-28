PYSIDE_CONFIG = $$PWD/../utils/pyside2_config.py

# Use provided python interpreter if given.
isEmpty(python_interpreter) {
    python_interpreter = python
}
message(Using python interpreter: $$python_interpreter)

SHIBOKEN6_GENERATOR = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken6-generator-path)
isEmpty(SHIBOKEN6_GENERATOR): error(Unable to locate the shiboken6-generator package location)

SHIBOKEN6_MODULE = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken6-module-path)
isEmpty(SHIBOKEN6_MODULE): error(Unable to locate the shiboken6 package location)

PYSIDE2 = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside2-path)
isEmpty(PYSIDE2): error(Unable to locate the PySide2 package location)

PYTHON_INCLUDE = $$system($$python_interpreter $$PYSIDE_CONFIG --python-include-path)
isEmpty(PYTHON_INCLUDE): error(Unable to locate the Python include headers directory)

PYTHON_LFLAGS = $$system($$python_interpreter $$PYSIDE_CONFIG --python-link-flags-qmake)
isEmpty(PYTHON_LFLAGS): error(Unable to locate the Python library for linking)

SHIBOKEN6_INCLUDE = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken6-generator-include-path)
isEmpty(SHIBOKEN6_INCLUDE): error(Unable to locate the shiboken include headers directory)

PYSIDE2_INCLUDE = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside2-include-path)
isEmpty(PYSIDE2_INCLUDE): error(Unable to locate the PySide2 include headers directory)

SHIBOKEN6_LFLAGS = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken6-module-qmake-lflags)
isEmpty(SHIBOKEN6_LFLAGS): error(Unable to locate the shiboken libraries for linking)

PYSIDE2_LFLAGS = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside2-qmake-lflags)
isEmpty(PYSIDE2_LFLAGS): error(Unable to locate the PySide2 libraries for linking)

SHIBOKEN6_SHARED_LIBRARIES = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken6-module-shared-libraries-qmake)
isEmpty(SHIBOKEN6_SHARED_LIBRARIES): error(Unable to locate the used shiboken6 module shared libraries)

PYSIDE2_SHARED_LIBRARIES = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside2-shared-libraries-qmake)
isEmpty(PYSIDE2_SHARED_LIBRARIES): error(Unable to locate the used PySide2 shared libraries)

INCLUDEPATH += "$$PYTHON_INCLUDE" $$PYSIDE2_INCLUDE $$SHIBOKEN6_INCLUDE
LIBS += $$PYTHON_LFLAGS $$PYSIDE2_LFLAGS $$SHIBOKEN6_LFLAGS
!build_pass:message(INCLUDEPATH is $$INCLUDEPATH)
!build_pass:message(LIBS are $$LIBS)

!build_pass:message(Using $$PYSIDE2)

!win32 {
    !build_pass:message(RPATH will include $$PYSIDE2 and $$SHIBOKEN6_MODULE)
    QMAKE_RPATHDIR += $$PYSIDE2 $$SHIBOKEN6_MODULE
}
