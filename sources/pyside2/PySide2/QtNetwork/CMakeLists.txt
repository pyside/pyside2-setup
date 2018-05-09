project(QtNetwork)

set(QtNetwork_OPTIONAL_SRC )
set(QtNetwork_DROPPED_ENTRIES )

check_qt_class(QtNetwork QSslCertificate    QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslCertificateExtension QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslCipher         QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslConfiguration  QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslDiffieHellmanParameters QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
# Problems with operator==(QSslEllipticCurve,QSslEllipticCurve)
# check_qt_class(QtNetwork QSslEllipticCurve QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslError          QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslKey            QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslPreSharedKeyAuthenticator QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSslSocket         QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)

check_qt_class(QtNetwork QSctpServer        QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
check_qt_class(QtNetwork QSctpSocket        QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)

set(QtNetwork_SRC
${QtNetwork_GEN_DIR}/qabstractnetworkcache_wrapper.cpp
${QtNetwork_GEN_DIR}/qabstractsocket_wrapper.cpp
${QtNetwork_GEN_DIR}/qauthenticator_wrapper.cpp
${QtNetwork_GEN_DIR}/qdnsdomainnamerecord_wrapper.cpp
${QtNetwork_GEN_DIR}/qdnshostaddressrecord_wrapper.cpp
${QtNetwork_GEN_DIR}/qdnslookup_wrapper.cpp
${QtNetwork_GEN_DIR}/qdnsmailexchangerecord_wrapper.cpp
${QtNetwork_GEN_DIR}/qdnsservicerecord_wrapper.cpp
${QtNetwork_GEN_DIR}/qdnstextrecord_wrapper.cpp
${QtNetwork_GEN_DIR}/qhostaddress_wrapper.cpp
${QtNetwork_GEN_DIR}/qhostinfo_wrapper.cpp
${QtNetwork_GEN_DIR}/qhstspolicy_wrapper.cpp
${QtNetwork_GEN_DIR}/qhttpmultipart_wrapper.cpp
${QtNetwork_GEN_DIR}/qhttppart_wrapper.cpp
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
${QtNetwork_GEN_DIR}/qnetworkdatagram_wrapper.cpp
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
${QtNetwork_OPTIONAL_SRC}
# module is always needed
${QtNetwork_GEN_DIR}/qtnetwork_module_wrapper.cpp
)

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
                     QtNetwork_SOURCE_DIR
                     QtNetwork_SRC
                     ""
                     ""
                     QtNetwork_DROPPED_ENTRIES)
