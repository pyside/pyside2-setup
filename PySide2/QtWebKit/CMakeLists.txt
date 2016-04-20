project(QtWebKit)

set(QtWebKit_SRC
${QtWebKit_GEN_DIR}/webcore_wrapper.cpp
# module is always needed
${QtWebKit_GEN_DIR}/qtwebkit_module_wrapper.cpp
)

make_path(QtWebKit_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                                   ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                                   ${QtWebKit_SOURCE_DIR})

set(QtWebKit_include_dirs   ${QtWebKit_SOURCE_DIR}
                            ${QtWebKit_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5WebKit_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtWidgets_GEN_DIR}
                            ${QtNetwork_GEN_DIR}
                            )
set(QtWebKit_libraries      pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5WebKit_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebKit_deps QtGui QtNetwork)
create_pyside_module(QtWebKit
                     QtWebKit_include_dirs
                     QtWebKit_libraries
                     QtWebKit_deps
                     QtWebKit_typesystem_path
                     QtWebKit_SRC
                     "")

