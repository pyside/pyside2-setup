project(Qt3DCore)

set(Qt3DCore_SRC
${Qt3DCore_GEN_DIR}/qt3dcore_qabstractaspect_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qabstractfrontendnodemanager_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qabstractfunctor_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qabstractskeleton_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qarmature_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qaspectengine_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qaspectjob_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qattribute_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qbackendnode_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qboundingvolume_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qbuffer_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qcomponent_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qentity_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qgeometry_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qgeometryview_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qjoint_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnode_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnodeid_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qnodeidtypepair_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qskeleton_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qskeletonloader_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_qtransform_wrapper.cpp
${Qt3DCore_GEN_DIR}/qt3dcore_wrapper.cpp
# module is always needed
${Qt3DCore_GEN_DIR}/qt3dcore_module_wrapper.cpp)

set(Qt3DCore_include_dirs
                     ${Qt3DCore_SOURCE_DIR}
                     ${Qt3DCore_BINARY_DIR}
                     ${pyside6_SOURCE_DIR}
                     ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                     ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                     ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                     ${Qt${QT_MAJOR_VERSION}3DCore_INCLUDE_DIRS}
                     ${libpyside_SOURCE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${QtNetwork_GEN_DIR})

set(Qt3DCore_libraries pyside6
                     ${Qt${QT_MAJOR_VERSION}3DCore_LIBRARIES})

set(Qt3DCore_deps QtGui QtNetwork)

create_pyside_module(NAME Qt3DCore
                     INCLUDE_DIRS Qt3DCore_include_dirs
                     LIBRARIES Qt3DCore_libraries
                     DEPS Qt3DCore_deps
                     TYPESYSTEM_PATH Qt3DCore_SOURCE_DIR
                     SOURCES Qt3DCore_SRC
                     TYPESYSTEM_NAME ${Qt3DCore_BINARY_DIR}/typesystem_3dcore.xml)
