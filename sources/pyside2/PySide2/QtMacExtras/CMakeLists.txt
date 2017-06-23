project(QtMacExtras)

set(QtMacExtras_SRC
${QtMacExtras_GEN_DIR}/qmacpasteboardmime_wrapper.cpp
${QtMacExtras_GEN_DIR}/qmactoolbar_wrapper.cpp
${QtMacExtras_GEN_DIR}/qmactoolbaritem_wrapper.cpp

# module is always needed
${QtMacExtras_GEN_DIR}/qtmacextras_module_wrapper.cpp
)

make_path(QtMacExtras_typesystem_path ${QtMacExtras_SOURCE_DIR}
                                      ${QtCore_SOURCE_DIR}
                                      ${QtCore_BINARY_DIR}
                                      ${QtGui_SOURCE_DIR}
                                      ${QtGui_BINARY_DIR})

set(QtMacExtras_include_dirs ${QtMacExtras_SOURCE_DIR}
                        ${QtMacExtras_BINARY_DIR}
                        ${Qt5MacExtras_INCLUDE_DIRS}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR})

set(QtMacExtras_libraries pyside2
                        ${Qt5MacExtras_LIBRARIES}
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES})

set(QtMacExtras_deps QtCore QtGui)

create_pyside_module(QtMacExtras
                     QtMacExtras_include_dirs
                     QtMacExtras_libraries
                     QtMacExtras_deps
                     QtMacExtras_typesystem_path
                     QtMacExtras_SRC
                     "")
