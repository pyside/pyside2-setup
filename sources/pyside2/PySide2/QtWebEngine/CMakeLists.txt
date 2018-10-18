project(QtWebEngine)

set(QtWebEngine_SRC
${QtWebEngine_GEN_DIR}/qtwebengine_wrapper.cpp
# module is always needed
${QtWebEngine_GEN_DIR}/qtwebengine_module_wrapper.cpp
)

set(QtWebEngine_include_dirs
                            ${QtWebEngine_SOURCE_DIR}
                            ${QtWebEngine_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            )
set(QtWebEngine_libraries pyside2
                            ${Qt5WebEngine_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebEngine_deps QtCore)
create_pyside_module(NAME QtWebEngine
                     INCLUDE_DIRS QtWebEngine_include_dirs
                     LIBRARIES QtWebEngine_libraries
                     DEPS QtWebEngine_deps
                     TYPESYSTEM_PATH QtWebEngine_SOURCE_DIR
                     SOURCES QtWebEngine_SRC)

