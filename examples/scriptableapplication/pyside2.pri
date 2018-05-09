PYSIDE2 = $$system(python $$PWD/pyside2_config.py --pyside2)
isEmpty(PYSIDE2): error(Unable to locate the PySide2 package location)

PYTHON_INCLUDE = $$system(python $$PWD/pyside2_config.py --python-include)
isEmpty(PYTHON_INCLUDE): error(Unable to locate the Python include headers directory)

PYTHON_LFLAGS = $$system(python $$PWD/pyside2_config.py --python-link)
isEmpty(PYTHON_LFLAGS): error(Unable to locate the Python library for linking)

PYSIDE2_INCLUDE = $$system(python $$PWD/pyside2_config.py --pyside2-include)
isEmpty(PYSIDE2_INCLUDE): error(Unable to locate the PySide2 include headers directory)

PYSIDE2_LFLAGS = $$system(python $$PWD/pyside2_config.py --pyside2-link)
isEmpty(PYSIDE2_LFLAGS): error(Unable to locate the PySide2 libraries for linking)

PYSIDE2_SHARED_LIBRARIES = $$system(python $$PWD/pyside2_config.py --pyside2-shared-libraries)
isEmpty(PYSIDE2_SHARED_LIBRARIES): error(Unable to locate the used PySide2 shared libraries)

INCLUDEPATH += "$$PYTHON_INCLUDE" $$PYSIDE2_INCLUDE
LIBS += $$PYTHON_LFLAGS $$PYSIDE2_LFLAGS
!build_pass:message(INCLUDEPATH is $$INCLUDEPATH)
!build_pass:message(LIBS are $$LIBS)

!build_pass:message(Using $$PYSIDE2)

!win32 {
    QMAKE_RPATHDIR += $$PYSIDE2
}
