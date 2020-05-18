project(QtWebKit)

set(QtWebKit_SRC
${QtWebKit_GEN_DIR}/webcore_wrapper.cpp
# module is always needed
${QtWebKit_GEN_DIR}/qtwebkit_module_wrapper.cpp
)

set(QtWebKit_include_dirs   ${QtWebKit_SOURCE_DIR}
                            ${QtWebKit_BINARY_DIR}
                            ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}WebKit_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtWidgets_GEN_DIR}
                            ${QtNetwork_GEN_DIR}
                            )
set(QtWebKit_libraries      pyside2
                            ${Qt${QT_MAJOR_VERSION}WebKit_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                            )
set(QtWebKit_deps QtGui QtNetwork)
create_pyside_module(NAME QtWebKit
                     INCLUDE_DIRS QtWebKit_include_dirs
                     LIBRARIES QtWebKit_libraries
                     DEPS QtWebKit_deps
                     TYPESYSTEM_PATH QtWebKit_SOURCE_DIR
                     SOURCES QtWebKit_SRC)

