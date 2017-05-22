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
${QtNetwork_GEN_DIR}/qabstractnetworkcache_wrapper.cpp
${QtNetwork_GEN_DIR}/qabstractsocket_wrapper.cpp
${QtNetwork_GEN_DIR}/qauthenticator_wrapper.cpp
${QtNetwork_GEN_DIR}/qhostaddress_wrapper.cpp
${QtNetwork_GEN_DIR}/qhostinfo_wrapper.cpp
${QtNetwork_GEN_DIR}/qipv6address_wrapper.cpp
${QtNetwork_GEN_DIR}/qlocalserver_wrapper.cpp
${QtNetwork_GEN_DIR}/qlocalsocket_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkaccessmanager_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkaddressentry_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkcachemetadata_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkconfiguration_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkconfigurationmanager_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkcookie_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkcookiejar_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkdiskcache_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkinterface_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkproxy_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkproxyfactory_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkproxyquery_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkreply_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworkrequest_wrapper.cpp
${QtNetwork_GEN_DIR}/qnetworksession_wrapper.cpp
${QtNetwork_GEN_DIR}/qssl_wrapper.cpp
${QtNetwork_GEN_DIR}/qtcpserver_wrapper.cpp
${QtNetwork_GEN_DIR}/qtcpsocket_wrapper.cpp
${QtNetwork_GEN_DIR}/qudpsocket_wrapper.cpp
# module is always needed
${QtNetwork_GEN_DIR}/qtnetwork_module_wrapper.cpp
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
                            ${QtCore_GEN_DIR})

set(QtNetwork_libraries pyside2
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
