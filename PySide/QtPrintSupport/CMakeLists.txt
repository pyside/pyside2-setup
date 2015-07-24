project(QtPrintSupport)

set(QtPrintSupport_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qabstractprintdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qpagesetupdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qprintdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qprintengine_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qprinter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qprinterinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qprintpreviewdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qprintpreviewwidget_wrapper.cpp
# always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport/qtprintsupport_module_wrapper.cpp
)

make_path(QtPrintSupport_typesystem_path    ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                                            ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                                            )

set(QtPrintSupport_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtPrintSupport
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5PrintSupport_INCLUDE_DIRS}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${QtCore_BINARY_DIR}/PySide/QtCore
                                ${QtGui_BINARY_DIR}/PySide/QtGui
                                ${QtWidgets_BINARY_DIR}/PySide/QtWidgets
                                )
set(QtPrintSupport_libraries    pyside
                                ${SHIBOKEN_PYTHON_LIBRARIES}
                                ${SHIBOKEN_LIBRARY}
                                ${Qt5Core_LIBRARIES}
                                ${Qt5Gui_LIBRARIES}
                                ${Qt5Widgets_LIBRARIES}
                                ${Qt5PrintSupport_LIBRARIES}
                                )
set(QtPrintSupport_deps QtWidgets)
create_pyside_module(QtPrintSupport
                     QtPrintSupport_include_dirs
                     QtPrintSupport_libraries
                     QtPrintSupport_deps
                     QtPrintSupport_typesystem_path
                     QtPrintSupport_SRC
                     "")
