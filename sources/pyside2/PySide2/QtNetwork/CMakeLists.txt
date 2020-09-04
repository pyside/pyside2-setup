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
${QtNetwork_GEN_DIR}/qpassworddigestor_wrapper.cpp
${QtNetwork_GEN_DIR}/qssl_wrapper.cpp
${QtNetwork_GEN_DIR}/qsslcertificate_wrapper.cpp
${QtNetwork_GEN_DIR}/qsslcertificateextension_wrapper.cpp
${QtNetwork_GEN_DIR}/qtcpserver_wrapper.cpp
${QtNetwork_GEN_DIR}/qtcpsocket_wrapper.cpp
${QtNetwork_GEN_DIR}/qudpsocket_wrapper.cpp
# module is always needed
${QtNetwork_GEN_DIR}/qtnetwork_module_wrapper.cpp
)

get_property(QtNetwork_disabled_features TARGET Qt${QT_MAJOR_VERSION}::Network
             PROPERTY QT_DISABLED_PUBLIC_FEATURES)

if("ssl" IN_LIST QtNetwork_disabled_features)
    list(APPEND QtNetwork_DROPPED_ENTRIES QOcspResponse QSslCipher
         QSslConfiguration QSslDiffieHellmanParameters QSslError
         QSslKey QSslPreSharedKeyAuthenticator QSslSocket)
    message(STATUS "Qt${QT_MAJOR_VERSION}Network: Dropping SSL classes")
else()
    # Problems with operator==(QSslEllipticCurve,QSslEllipticCurve)
    # check_qt_class(QtNetwork QSslEllipticCurve QtNetwork_OPTIONAL_SRC QtNetwork_DROPPED_ENTRIES)
    list(APPEND QtNetwork_SRC
         ${QtNetwork_GEN_DIR}/qsslcipher_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslconfiguration_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qssldiffiehellmanparameters_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslerror_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslkey_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslpresharedkeyauthenticator_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsslsocket_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qocspresponse_wrapper.cpp)
    message(STATUS "Qt${QT_MAJOR_VERSION}Network: Adding SSL classes")
endif()

if("dtls" IN_LIST QtNetwork_disabled_features)
    list(APPEND QtNetwork_DROPPED_ENTRIES QDtls)
    message(STATUS "Qt${QT_MAJOR_VERSION}Network: Dropping DTLS classes")
else()
    list(APPEND QtNetwork_SRC
         ${QtNetwork_GEN_DIR}/qdtls_wrapper.cpp)
    message(STATUS "Qt${QT_MAJOR_VERSION}Network: Adding DTLS classes")
endif()

if("sctp" IN_LIST QtNetwork_disabled_features)
    list(APPEND QtNetwork_DROPPED_ENTRIES QSctpServer QSctpSocket)
    message(STATUS "Qt${QT_MAJOR_VERSION}Network: Dropping SCTP classes")
else()
    list(APPEND QtNetwork_SRC
         ${QtNetwork_GEN_DIR}/qsctpserver_wrapper.cpp
         ${QtNetwork_GEN_DIR}/qsctpsocket_wrapper.cpp)
    message(STATUS "Qt${QT_MAJOR_VERSION}Network: Adding SCTP classes")
endif()

set(QtNetwork_include_dirs  ${QtNetwork_SOURCE_DIR}
                            ${QtNetwork_BINARY_DIR}
                            ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR})

set(QtNetwork_libraries pyside2
                        ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES})

set(QtNetwork_deps QtCore)

create_pyside_module(NAME QtNetwork
                     INCLUDE_DIRS QtNetwork_include_dirs
                     LIBRARIES QtNetwork_libraries
                     DEPS QtNetwork_deps
                     TYPESYSTEM_PATH QtNetwork_SOURCE_DIR
                     SOURCES QtNetwork_SRC
                     DROPPED_ENTRIES QtNetwork_DROPPED_ENTRIES)
