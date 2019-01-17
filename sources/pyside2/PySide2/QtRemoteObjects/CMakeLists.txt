project(QtRemoteObjects)

set(QtRemoteObjects_SRC
${QtRemoteObjects_GEN_DIR}/qabstractitemmodelreplica_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectabstractpersistedstore_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectdynamicreplica_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjecthost_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjecthostbase_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectnode_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectpendingcall_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectpendingcallwatcher_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectregistry_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectregistryhost_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectreplica_wrapper.cpp
# ${QtRemoteObjects_GEN_DIR}/qtremoteobjects_wrapper.cpp
${QtRemoteObjects_GEN_DIR}/qremoteobjectsettingsstore_wrapper.cpp

# module is always needed
${QtRemoteObjects_GEN_DIR}/qtremoteobjects_module_wrapper.cpp
)

set(QtRemoteObjects_include_dirs ${QtRemoteObjects_SOURCE_DIR}
                                 ${QtRemoteObjects_BINARY_DIR}
                                 ${Qt5RemoteObjects_INCLUDE_DIRS}
                                 ${SHIBOKEN_INCLUDE_DIR}
                                 ${libpyside_SOURCE_DIR}
                                 ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                 ${QtCore_GEN_DIR})

set(QtRemoteObjects_libraries    pyside2
                                 ${SHIBOKEN_PYTHON_LIBRARIES}
                                 ${SHIBOKEN_LIBRARY}
                                 ${Qt5RemoteObjects_LIBRARIES})

set(QtRemoteObjects_deps QtCore QtNetwork)

create_pyside_module(NAME QtRemoteObjects
                     INCLUDE_DIRS QtRemoteObjects_include_dirs
                     LIBRARIES QtRemoteObjects_libraries
                     DEPS QtRemoteObjects_deps
                     TYPESYSTEM_PATH QtRemoteObjects_SOURCE_DIR
                     SOURCES QtRemoteObjects_SRC
                     TYPESYSTEM_NAME ${QtRemoteObjects_BINARY_DIR}/typesystem_remoteobjects.xml
                     )
