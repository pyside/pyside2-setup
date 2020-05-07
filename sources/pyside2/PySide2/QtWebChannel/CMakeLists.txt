project(QtWebChannel)

set(QtWebChannel_SRC
${QtWebChannel_GEN_DIR}/qwebchannel_wrapper.cpp
${QtWebChannel_GEN_DIR}/qwebchannelabstracttransport_wrapper.cpp
# module is always needed
${QtWebChannel_GEN_DIR}/qtwebchannel_module_wrapper.cpp
)

set(QtWebChannel_include_dirs ${QtWebChannel_SOURCE_DIR}
                        ${QtWebChannel_BINARY_DIR}
                        ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}WebChannel_INCLUDE_DIRS}
                        ${libpyside_SOURCE_DIR}
                        ${QtCore_GEN_DIR}
                        )
set(QtWebChannel_libraries    pyside2
                        ${Qt${QT_MAJOR_VERSION}WebChannel_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                        )
set(QtWebChannel_deps QtCore)

create_pyside_module(NAME QtWebChannel
                     INCLUDE_DIRS QtWebChannel_include_dirs
                     LIBRARIES QtWebChannel_libraries
                     DEPS QtWebChannel_deps
                     TYPESYSTEM_PATH QtWebChannel_SOURCE_DIR
                     SOURCES QtWebChannel_SRC)
