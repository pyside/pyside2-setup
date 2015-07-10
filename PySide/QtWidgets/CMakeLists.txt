project(QtWidgets)


set(QtWidgets_OPTIONAL_SRC )
set(QtWidgets_DROPPED_ENTRIES )
## gone check_qt_class(Qt5Widgets QAbstractPageSetupDialog   QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QAbstractPrintDialog       QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QGtkStyle                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPageSetupDialog           QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintDialog               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintEngine               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintPreviewDialog        QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintPreviewWidget        QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrinter                   QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrinterInfo               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSessionManager            QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSizeGrip                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSystemTrayIcon            QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QMacStyle                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)

set(QtWidgets_SRC
## add the files which are generated, later.
${SPECIFIC_OS_FILES}
# this file is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtwidgets_module_wrapper.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/typesystem_widgets.xml.in"
               "${CMAKE_CURRENT_BINARY_DIR}/typesystem_widgetsi.xml" @ONLY)

# fake these variables for qt5
set(QtWidgets_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtWidgets_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets)

set(QtWidgets_typesystem_path "${QtCore_SOURCE_DIR}${PATH_SEP}${QtCore_BINARY_DIR}${PATH_SEP}${QtGui_SOURCE_DIR}${PATH_SEP}${QtGui_BINARY_DIR}${PATH_SEP}${QtWidgets_SOURCE_DIR}")

set(QtWidgets_include_dirs  ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/
                        ${pyside_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_BINARY_DIR}/PySide/QtCore/
                        ${QtGui_BINARY_DIR}/PySide/QtGui/
                        )
set(QtWidgets_libraries   pyside
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      )
set(QtWidgets_deps "QtGui")

create_pyside_module(QtWidgets
                     QtWidgets_include_dirs
                     QtWidgets_libraries
                     QtWidgets_deps
                     QtWidgets_typesystem_path
                     QtWidgets_SRC
                     ""
                     ${CMAKE_CURRENT_BINARY_DIR}/typesystem_widgets.xml)
