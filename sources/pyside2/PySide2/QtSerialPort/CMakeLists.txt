project(QtSerialPort)

set(QtSerialPort_OPTIONAL_SRC )
set(QtSerialPort_DROPPED_ENTRIES )

set(QtSerialPort_SRC
    ${QtSerialPort_GEN_DIR}/qserialport_wrapper.cpp
    ${QtSerialPort_GEN_DIR}/qserialportinfo_wrapper.cpp
# module is always needed
    ${QtSerialPort_GEN_DIR}/qtserialport_module_wrapper.cpp
)

set(QtSerialPort_include_dirs ${QtSerialPort_SOURCE_DIR}
                              ${QtSerialPort_BINARY_DIR}
                              ${Qt5Core_INCLUDE_DIRS}
                              ${Qt5SerialPort_INCLUDE_DIRS}
                              ${libpyside_SOURCE_DIR}
                              ${QtCore_GEN_DIR})

set(QtSerialPort_libraries pyside2
                        ${Qt5SerialPort_LIBRARIES})

set(QtSerialPort_deps QtCore)

create_pyside_module(NAME QtSerialPort
                     INCLUDE_DIRS QtSerialPort_include_dirs
                     LIBRARIES QtSerialPort_libraries
                     DEPS QtSerialPort_deps
                     TYPESYSTEM_PATH QtSerialPort_SOURCE_DIR
                     SOURCES QtSerialPort_SRC
                     DROPPED_ENTRIES QtSerialPort_DROPPED_ENTRIES)
