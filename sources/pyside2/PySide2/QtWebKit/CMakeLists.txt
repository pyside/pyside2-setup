project(QtWebKit)

set(QtWebKit_SRC
${QtWebKit_GEN_DIR}/webcore_wrapper.cpp
# module is always needed
${QtWebKit_GEN_DIR}/qtwebkit_module_wrapper.cpp
)

set(QtWebKit_include_dirs   ${QtWebKit_SOURCE_DIR}
                            ${QtWebKit_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5WebKit_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtWidgets_GEN_DIR}
                            ${QtNetwork_GEN_DIR}
                            )
set(QtWebKit_libraries      pyside2
                            ${Qt5WebKit_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebKit_deps QtGui QtNetwork)
create_pyside_module(NAME QtWebKit
                     INCLUDE_DIRS QtWebKit_include_dirs
                     LIBRARIES QtWebKit_libraries
                     DEPS QtWebKit_deps
                     TYPESYSTEM_PATH QtWebKit_SOURCE_DIR
                     SOURCES QtWebKit_SRC)

