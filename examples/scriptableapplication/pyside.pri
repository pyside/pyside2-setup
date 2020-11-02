PYSIDE_CONFIG = $$PWD/../utils/pyside_config.py

# Use provided python interpreter if given.
isEmpty(python_interpreter) {
    python_interpreter = python
}
message(Using python interpreter: $$python_interpreter)

SHIBOKEN_GENERATOR = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken-generator-path)
isEmpty(SHIBOKEN_GENERATOR): error(Unable to locate the shiboken-generator package location)

SHIBOKEN_MODULE = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken-module-path)
isEmpty(SHIBOKEN_MODULE): error(Unable to locate the shiboken package location)

PYSIDE = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside-path)
isEmpty(PYSIDE): error(Unable to locate the PySide package location)

PYTHON_INCLUDE = $$system($$python_interpreter $$PYSIDE_CONFIG --python-include-path)
isEmpty(PYTHON_INCLUDE): error(Unable to locate the Python include headers directory)

PYTHON_LFLAGS = $$system($$python_interpreter $$PYSIDE_CONFIG --python-link-flags-qmake)
isEmpty(PYTHON_LFLAGS): error(Unable to locate the Python library for linking)

SHIBOKEN_INCLUDE = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken-generator-include-path)
isEmpty(SHIBOKEN_INCLUDE): error(Unable to locate the shiboken include headers directory)

PYSIDE_INCLUDE = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside-include-path)
isEmpty(PYSIDE_INCLUDE): error(Unable to locate PySide include headers directory)

SHIBOKEN_LFLAGS = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken-module-qmake-lflags)
isEmpty(SHIBOKEN_LFLAGS): error(Unable to locate the shiboken libraries for linking)

PYSIDE_LFLAGS = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside-qmake-lflags)
isEmpty(PYSIDE_LFLAGS): error(Unable to locate the PySide libraries for linking)

SHIBOKEN_SHARED_LIBRARIES = $$system($$python_interpreter $$PYSIDE_CONFIG --shiboken-module-shared-libraries-qmake)
isEmpty(SHIBOKEN_SHARED_LIBRARIES): error(Unable to locate the used shiboken module shared libraries)

PYSIDE_SHARED_LIBRARIES = $$system($$python_interpreter $$PYSIDE_CONFIG --pyside-shared-libraries-qmake)
isEmpty(PYSIDE_SHARED_LIBRARIES): error(Unable to locate the used PySide shared libraries)

INCLUDEPATH += "$$PYTHON_INCLUDE" $$PYSIDE_INCLUDE $$SHIBOKEN_INCLUDE
LIBS += $$PYTHON_LFLAGS $$PYSIDE_LFLAGS $$SHIBOKEN_LFLAGS
!build_pass:message(INCLUDEPATH is $$INCLUDEPATH)
!build_pass:message(LIBS are $$LIBS)

!build_pass:message(Using $$PYSIDE)

!win32 {
    !build_pass:message(RPATH will include $$PYSIDE and $$SHIBOKEN_MODULE)
    QMAKE_RPATHDIR += $$PYSIDE $$SHIBOKEN_MODULE
}
