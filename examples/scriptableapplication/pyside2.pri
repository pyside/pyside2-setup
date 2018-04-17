PYTHON_INCLUDE = $$system(python $$PWD/pyside2_config.py --python-include)
isEmpty(PYTHON_INCLUDE): error(Unable to locate Python)
PYTHON_LFLAGS = $$system(python $$PWD/pyside2_config.py --python-link)

PYSIDE2 = $$system(python $$PWD/pyside2_config.py --pyside2)
isEmpty(PYSIDE2): error(Unable to locate PySide2)
PYSIDE2_INCLUDE = $$system(python $$PWD/pyside2_config.py --pyside2-include)
PYSIDE2_LFLAGS = $$system(python $$PWD/pyside2_config.py --pyside2-link)
PYSIDE2_SHARED_LIBRARIES = $$system(python $$PWD/pyside2_config.py --pyside2-shared-libraries)
CLANG_BIN_DIR = $$system(python $$PWD/pyside2_config.py --clang-bin-dir)

INCLUDEPATH += "$$PYTHON_INCLUDE" $$PYSIDE2_INCLUDE
LIBS += $$PYTHON_LFLAGS $$PYSIDE2_LFLAGS
LIBPATH += $$PYTHON_LFLAGS $$PYSIDE2_LFLAGS
!build_pass:message(INCLUDEPATH is $$INCLUDEPATH)
!build_pass:message(LIBS are $$LIBS)
!build_pass:message(LIBPATH is $$LIBPATH)

!build_pass:message(Using $$PYSIDE2)

!win32 {
    QMAKE_RPATHDIR += $$PYSIDE2
}
