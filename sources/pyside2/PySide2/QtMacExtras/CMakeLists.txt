project(QtMacExtras)

set(QtMacExtras_SRC
${QtMacExtras_GEN_DIR}/qmacpasteboardmime_wrapper.cpp
${QtMacExtras_GEN_DIR}/qmactoolbar_wrapper.cpp
${QtMacExtras_GEN_DIR}/qmactoolbaritem_wrapper.cpp

# module is always needed
${QtMacExtras_GEN_DIR}/qtmacextras_module_wrapper.cpp
)

set(QtMacExtras_include_dirs ${QtMacExtras_SOURCE_DIR}
                        ${QtMacExtras_BINARY_DIR}
                        ${Qt${QT_MAJOR_VERSION}MacExtras_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${libpyside_SOURCE_DIR})

set(QtMacExtras_libraries pyside2
                        ${Qt${QT_MAJOR_VERSION}MacExtras_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES})

set(QtMacExtras_deps QtCore QtGui)

create_pyside_module(NAME QtMacExtras
                     INCLUDE_DIRS QtMacExtras_include_dirs
                     LIBRARIES QtMacExtras_libraries
                     DEPS QtMacExtras_deps
                     TYPESYSTEM_PATH QtMacExtras_SOURCE_DIR
                     SOURCES QtMacExtras_SRC)
