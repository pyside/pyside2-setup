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

configure_file("${QtPrintSupport_SOURCE_DIR}/typesystem_printsupport.xml.in"
               "${QtPrintSupport_BINARY_DIR}/typesystem_printsupport.xml" @ONLY)

set(QtPrintSupport_include_dirs ${QtPrintSupport_SOURCE_DIR}
                                ${QtPrintSupport_BINARY_DIR}
                                ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}PrintSupport_INCLUDE_DIRS}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR}
                                ${QtGui_GEN_DIR}
                                ${QtWidgets_GEN_DIR}
                                )
set(QtPrintSupport_libraries    pyside6
                                ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                                ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                                ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                                ${Qt${QT_MAJOR_VERSION}PrintSupport_LIBRARIES}
                                )
set(QtPrintSupport_deps QtWidgets)
create_pyside_module(NAME QtPrintSupport
                     INCLUDE_DIRS QtPrintSupport_include_dirs
                     LIBRARIES QtPrintSupport_libraries
                     DEPS QtPrintSupport_deps
                     TYPESYSTEM_PATH QtPrintSupport_SOURCE_DIR
                     SOURCES QtPrintSupport_SRC
                     TYPESYSTEM_NAME ${QtPrintSupport_BINARY_DIR}/typesystem_printsupport.xml)
