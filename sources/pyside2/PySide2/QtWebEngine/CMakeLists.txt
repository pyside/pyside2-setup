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
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_GEN_DIR}
                            )
set(QtWebEngine_libraries pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5WebEngine_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebEngine_deps QtCore)
create_pyside_module(QtWebEngine
                     QtWebEngine_include_dirs
                     QtWebEngine_libraries
                     QtWebEngine_deps
                     QtWebEngine_SOURCE_DIR
                     QtWebEngine_SRC
                     "")

