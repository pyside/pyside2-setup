project(QtWebChannel)

set(QtWebChannel_SRC
${QtWebChannel_GEN_DIR}/qwebchannel_wrapper.cpp
${QtWebChannel_GEN_DIR}/qwebchannelabstracttransport_wrapper.cpp
# module is always needed
${QtWebChannel_GEN_DIR}/qtwebchannel_module_wrapper.cpp
)

set(QtWebChannel_include_dirs ${QtWebChannel_SOURCE_DIR}
                        ${QtWebChannel_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5WebChannel_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_GEN_DIR}
                        )
set(QtWebChannel_libraries    pyside2
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5WebChannel_LIBRARIES}
                        ${Qt5Core_LIBRARIES}
                        )
set(QtWebChannel_deps QtCore)

create_pyside_module(NAME QtWebChannel
                     INCLUDE_DIRS QtWebChannel_include_dirs
                     LIBRARIES QtWebChannel_libraries
                     DEPS QtWebChannel_deps
                     TYPESYSTEM_PATH QtWebChannel_SOURCE_DIR
                     SOURCES QtWebChannel_SRC)
