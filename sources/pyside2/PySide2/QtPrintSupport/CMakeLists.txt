project(QtPrintSupport)

set(QtPrintSupport_SRC
${QtPrintSupport_GEN_DIR}/qabstractprintdialog_wrapper.cpp
${QtPrintSupport_GEN_DIR}/qpagesetupdialog_wrapper.cpp
${QtPrintSupport_GEN_DIR}/qprintdialog_wrapper.cpp
${QtPrintSupport_GEN_DIR}/qprintengine_wrapper.cpp
${QtPrintSupport_GEN_DIR}/qprinter_wrapper.cpp
${QtPrintSupport_GEN_DIR}/qprinterinfo_wrapper.cpp
${QtPrintSupport_GEN_DIR}/qprintpreviewdialog_wrapper.cpp
${QtPrintSupport_GEN_DIR}/qprintpreviewwidget_wrapper.cpp
# module is always needed
${QtPrintSupport_GEN_DIR}/qtprintsupport_module_wrapper.cpp
)

set(QtPrintSupport_include_dirs ${QtPrintSupport_SOURCE_DIR}
                                ${QtPrintSupport_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Gui_INCLUDE_DIRS}
                                ${Qt5Widgets_INCLUDE_DIRS}
                                ${Qt5PrintSupport_INCLUDE_DIRS}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${QtCore_GEN_DIR}
                                ${QtGui_GEN_DIR}
                                ${QtWidgets_GEN_DIR}
                                )
set(QtPrintSupport_libraries    pyside2
                                ${SHIBOKEN_PYTHON_LIBRARIES}
                                ${SHIBOKEN_LIBRARY}
                                ${Qt5Core_LIBRARIES}
                                ${Qt5Gui_LIBRARIES}
                                ${Qt5Widgets_LIBRARIES}
                                ${Qt5PrintSupport_LIBRARIES}
                                )
set(QtPrintSupport_deps QtWidgets)
create_pyside_module(NAME QtPrintSupport
                     INCLUDE_DIRS QtPrintSupport_include_dirs
                     LIBRARIES QtPrintSupport_libraries
                     DEPS QtPrintSupport_deps
                     TYPESYSTEM_PATH QtPrintSupport_SOURCE_DIR
                     SOURCES QtPrintSupport_SRC)
