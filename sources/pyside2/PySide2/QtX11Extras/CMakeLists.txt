project(QtX11Extras)

set(QtX11Extras_SRC
${QtX11Extras_GEN_DIR}/qx11info_wrapper.cpp
# module is always needed
${QtX11Extras_GEN_DIR}/qtx11extras_module_wrapper.cpp
)

configure_file("${QtX11Extras_SOURCE_DIR}/QtX11Extras_global.post.h.in"
               "${QtX11Extras_BINARY_DIR}/QtX11Extras_global.post.h" @ONLY)

set(QtX11Extras_include_dirs ${QtX11Extras_SOURCE_DIR}
                        ${QtX11Extras_BINARY_DIR}
                        ${Qt${QT_MAJOR_VERSION}X11Extras_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${libpyside_SOURCE_DIR})

set(QtX11Extras_libraries pyside2
                        ${Qt${QT_MAJOR_VERSION}X11Extras_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES})

set(QtX11Extras_deps QtCore QtGui)

create_pyside_module(NAME QtX11Extras
                     INCLUDE_DIRS QtX11Extras_include_dirs
                     LIBRARIES QtX11Extras_libraries
                     DEPS QtX11Extras_deps
                     TYPESYSTEM_PATH QtX11Extras_SOURCE_DIR
                     SOURCES QtX11Extras_SRC)
