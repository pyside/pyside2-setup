project(QtNetwork)

set(QtNetwork_OPTIONAL_SRC )
set(QtNetwork_DROPPED_ENTRIES )
check_qt_class(QtNetwork QSslCertificate    QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslCipher         QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslConfiguration  QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslError          QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslKey            QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslSocket         QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)

set(QtNetwork_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qabstractsocket_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qauthenticator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qhostaddress_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qhostinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qipv6address_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qlocalserver_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qlocalsocket_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkaccessmanager_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkaddressentry_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkcookie_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkcookiejar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkinterface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkproxy_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkproxyfactory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkproxyquery_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkreply_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qnetworkrequest_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qssl_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qtcpserver_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qtcpsocket_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qudpsocket_wrapper.cpp
${QtNetwork_OPTIONAL_SRC}
# module is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtNetwork/qtnetwork_module_wrapper.cpp
)

make_path(QtNetwork_typesystem_path ${QtCore_SOURCE_DIR}
                                    ${QtCore_BINARY_DIR}
                                    ${QtNetwork_SOURCE_DIR})

set(QtNetwork_include_dirs  ${QtNetwork_SOURCE_DIR}
                            ${QtNetwork_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_BINARY_DIR}/PySide/QtCore/)

set(QtNetwork_libraries pyside
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5Network_LIBRARIES})

set(QtNetwork_deps QtCore)

create_pyside_module(QtNetwork
                     QtNetwork_include_dirs
                     QtNetwork_libraries
                     QtNetwork_deps
                     QtNetwork_typesystem_path
                     QtNetwork_SRC
                     ""
                     ""
                     QtNetwork_DROPPED_ENTRIES)
