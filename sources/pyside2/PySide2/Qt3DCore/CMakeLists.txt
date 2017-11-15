project(Qt3DCore)

set(Qt3DCore_SRC
${Qt3DCore_GEN_DIR}/qt3dcore_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qabstractaspect_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qaspectengine_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qaspectjob_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qbackendnode_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qcomponent_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qcomponentaddedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qcomponentremovedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qdynamicpropertyupdatedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qentity_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnode_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnodecreatedchangebase_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnodedestroyedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnodeid_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnodeidtypepair_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertynodeaddedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertynoderemovedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertyupdatedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertyupdatedchangebase_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertyvalueaddedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertyvalueaddedchangebase_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertyvalueremovedchange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qpropertyvalueremovedchangebase_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qscenechange_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qstaticpropertyupdatedchangebase_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qstaticpropertyvalueaddedchangebase_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qstaticpropertyvalueremovedchangebase_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qtransform_wrapper.cpp
# module is always needed
${Qt3DCore_GEN_DIR}/qt3dcore_module_wrapper.cpp)

if (Qt53DCore_VERSION VERSION_EQUAL 5.10.0 OR Qt53DCore_VERSION VERSION_GREATER 5.10.0)
    list(APPEND Qt3DCore_SRC
         ${Qt3DCore_GEN_DIR}/qt3dcore_qarmature_wrapper.cpp
         ${Qt3DCore_GEN_DIR}/qt3dcore_qjoint_wrapper.cpp
         ${Qt3DCore_GEN_DIR}/qt3dcore_qabstractskeleton_wrapper.cpp
         ${Qt3DCore_GEN_DIR}/qt3dcore_qnodecommand_wrapper.cpp
         ${Qt3DCore_GEN_DIR}/qt3dcore_qskeleton_wrapper.cpp
         ${Qt3DCore_GEN_DIR}/qt3dcore_qskeletonloader_wrapper.cpp)
endif()

set(Qt3DCore_include_dirs
                     ${Qt3DCore_SOURCE_DIR}
                     ${Qt3DCore_BINARY_DIR}
                     ${pyside2_SOURCE_DIR}
                     ${Qt5Core_INCLUDE_DIRS}
                     ${Qt5Gui_INCLUDE_DIRS}
                     ${Qt5Network_INCLUDE_DIRS}
                     ${Qt53DCore_INCLUDE_DIRS}
                     ${SHIBOKEN_INCLUDE_DIR}
                     ${libpyside_SOURCE_DIR}
                     ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${QtNetwork_GEN_DIR})

set(Qt3DCore_libraries pyside2
                     ${SHIBOKEN_PYTHON_LIBRARIES}
                     ${SHIBOKEN_LIBRARY}
                     ${Qt53DCore_LIBRARIES})

set(Qt3DCore_deps QtGui QtNetwork)

create_pyside_module(Qt3DCore
                     Qt3DCore_include_dirs
                     Qt3DCore_libraries
                     Qt3DCore_deps
                     Qt3DCore_SOURCE_DIR
                     Qt3DCore_SRC
                     ""
                     ${Qt3DCore_BINARY_DIR}/typesystem_3dcore.xml)
