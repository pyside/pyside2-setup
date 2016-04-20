project(QtWebChannel)

set(QtWebChannel_SRC
${QtWebChannel_GEN_DIR}/qwebchannel_wrapper.cpp
${QtWebChannel_GEN_DIR}/qwebchannelabstracttransport_wrapper.cpp
# module is always needed
${QtWebChannel_GEN_DIR}/qtwebchannel_module_wrapper.cpp
)

make_path(QtWebChannel_typesystem_path ${QtCore_SOURCE_DIR} ${QtCore_BINARY_DIR} ${QtWebChannel_SOURCE_DIR})

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

create_pyside_module(QtWebChannel
                     QtWebChannel_include_dirs
                     QtWebChannel_libraries
                     QtWebChannel_deps
                     QtWebChannel_typesystem_path
                     QtWebChannel_SRC
                     "")
