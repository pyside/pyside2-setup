project(QtNetwork)

set(QtNetwork_DROPPED_ENTRIES )

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
${QtNetwork_GEN_DIR}/qpassworddigestor_wrapper.cpp
${QtNetwork_GEN_DIR}/qssl_wrapper.cpp
${QtNetwork_GEN_DIR}/qtcpserver_wrapper.cpp
${QtNetwork_GEN_DIR}/qtcpsocket_wrapper.cpp
${QtNetwork_GEN_DIR}/qudpsocket_wrapper.cpp
# module is always needed
${QtNetwork_GEN_DIR}/qtnetwork_module_wrapper.cpp
)

get_property(QtNetwork_enabled_features  TARGET Qt5::Network PROPERTY INTERFACE_QT_ENABLED_FEATURES)
get_property(QtNetwork_disabled_features TARGET Qt5::Network PROPERTY INTERFACE_QT_DISABLED_FEATURES)

# ### fixme: For cmake >= 3.3, use if( needle IN_LIST list)
list(FIND QtNetwork_enabled_features  "ssl"  _sslEnabledIndex)
list(FIND QtNetwork_disabled_features "dtls" _dtlsDisabledIndex)
list(FIND QtNetwork_disabled_features "sctp" _sctpDisabledIndex)

if(_sslEnabledIndex EQUAL -1)
    list(APPEND QtNetwork_DROPPED_ENTRIES QSslCertificate QSslCertificateExtension
         QSslCipher QSslConfiguration QSslDiffieHellmanParameters QSslError
         QSslKey QSslPreSharedKeyAuthenticator QSslSocket)
    message(STATUS "Qt5Network: Dropping SSL classes")
else()
    # Problems with operator==(QSslEllipticCurve,QSslEllipticCurve)
    # check_qt_class(QtNetwork QSslEllipticCurve QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
    list(APPEND QtNetwork_SRC
         ${QtNetwork_GEN_DIR}/qsslcertificate_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslcertificateextension_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslcipher_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslconfiguration_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qssldiffiehellmanparameters_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslerror_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslkey_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslpresharedkeyauthenticator_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslsocket_wrapper.cpp)
    message(STATUS "Qt5Network: Adding SSL classes")
endif()

if(_dtlsDisabledIndex GREATER -1)
    list(APPEND QtNetwork_DROPPED_ENTRIES QDtls)
    message(STATUS "Qt5Network: Dropping DTLS classes")
else()
    list(APPEND QtNetwork_SRC
         ${QtNetwork_GEN_DIR}/qdtls_wrapper.cpp)
    message(STATUS "Qt5Network: Adding DTLS classes")
endif()

if(_sctpDisabledIndex GREATER -1)
    list(APPEND QtNetwork_DROPPED_ENTRIES QSctpServer QSctpSocket)
    message(STATUS "Qt5Network: Dropping SCTP classes")
else()
    list(APPEND QtNetwork_SRC
         ${QtNetwork_GEN_DIR}/qsctpserver_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsctpsocket_wrapper.cpp)
    message(STATUS "Qt5Network: Adding SCTP classes")
endif()

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
