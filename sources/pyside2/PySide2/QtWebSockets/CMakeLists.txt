project(QtWebSockets)

set(QtWebSockets_SRC
${QtWebSockets_GEN_DIR}/qmaskgenerator_wrapper.cpp
${QtWebSockets_GEN_DIR}/qwebsocket_wrapper.cpp
${QtWebSockets_GEN_DIR}/qwebsocketcorsauthenticator_wrapper.cpp
${QtWebSockets_GEN_DIR}/qwebsocketprotocol_wrapper.cpp
${QtWebSockets_GEN_DIR}/qwebsocketserver_wrapper.cpp
# module is always needed
${QtWebSockets_GEN_DIR}/qtwebsockets_module_wrapper.cpp
)

set(QtWebSockets_include_dirs ${QtWebSockets_SOURCE_DIR}
                        ${QtWebSockets_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Network_INCLUDE_DIRS}
                        ${Qt5WebSockets_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_GEN_DIR}
                        ${QtWebSockets_GEN_DIR}
                        ${QtNetwork_GEN_DIR}
                        )
set(QtWebSockets_libraries    pyside2
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5WebSockets_LIBRARIES}
                        ${Qt5Network_LIBRARIES}
                        ${Qt5Core_LIBRARIES}
                        )

set(QtWebSockets_deps QtNetwork)

create_pyside_module(NAME QtWebSockets
                     INCLUDE_DIRS QtWebSockets_include_dirs
                     LIBRARIES QtWebSockets_libraries
                     DEPS QtWebSockets_deps
                     TYPESYSTEM_PATH QtWebSockets_SOURCE_DIR
                     SOURCES QtWebSockets_SRC)
